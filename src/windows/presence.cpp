#include <winsock2.h>
#include <dwmapi.h>
#include "presence.hpp"
#include "launch_ids.h"
#include "../presence_state.hpp"
#include "process.hpp"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <thread>
#include <atomic>
#include <chrono>
#include <charconv>
#include <cstring>
#include <array>

namespace masiva::rpc {
namespace {
using namespace std::chrono_literals;
using Clock=std::chrono::steady_clock;
using namespace presence;
std::wstring game_exe;
std::thread worker;
std::atomic<bool> quitting{false};
enum class Status {idle,waiting,connected,rejected,unavailable};
std::atomic<Status> status{Status::idle};
std::atomic<bool> no_cfg{false};
constexpr unsigned port=39279;
const char cfg[]=R"cfg("Masiva Skin Discord"
{
    "uri" "http://127.0.0.1:39279/masiva"
    "timeout" "2.0"
    "buffer" "1.0"
    "throttle" "5.0"
    "heartbeat" "15.0"
    "data" { "provider" "1" "map" "1" "player_id" "1" }
}
)cfg";
auto ini(){
    wchar_t path[32768]{};GetModuleFileNameW(nullptr,path,32768);
    return std::filesystem::path(path).parent_path()/L"Masiva-Skin.ini";
}
bool enabled(){
    return GetPrivateProfileIntW(L"Discord",L"Enabled",0,ini().c_str())!=0;
}
std::filesystem::path steam_game(){
    wchar_t steam[32768]{};DWORD bytes=sizeof(steam);
    if(RegGetValueW(HKEY_CURRENT_USER,L"Software\\Valve\\Steam",L"SteamPath",RRF_RT_REG_SZ,nullptr,steam,&bytes)!=ERROR_SUCCESS)return {};
    const auto locate=[](const std::filesystem::path& root){
        auto exe=root/L"steamapps/common/Counter-Strike Global Offensive/game/bin/win64/cs2.exe";
        std::error_code ec;return std::filesystem::is_regular_file(exe,ec)?exe:std::filesystem::path{};
    };
    if(auto exe=locate(steam);!exe.empty())return exe;
    std::ifstream libraries(std::filesystem::path(steam)/L"steamapps/libraryfolders.vdf");
    std::string line;
    while(std::getline(libraries,line)){
        std::istringstream in(line);std::string key,path;
        if((in>>std::quoted(key)>>std::quoted(path)) && key=="path"){
            if(auto exe=locate(std::filesystem::u8path(path));!exe.empty())return exe;
        }
    }
    return {};
}
void install_cfg(const std::filesystem::path& exe){
    auto dir=exe.parent_path().parent_path().parent_path()/L"csgo"/L"cfg";
    auto path=dir/L"gamestate_integration_masiva_discord.cfg";
    Handle file(CreateFileW(path.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr));
    if(!file.valid()){
        if(GetLastError()!=ERROR_FILE_EXISTS){no_cfg=true;return;}
        std::ifstream input(path,std::ios::binary);
        std::array<char,sizeof(cfg)> data{};
        input.read(data.data(),data.size());
        no_cfg=input.gcount()!=sizeof(cfg)-1 || std::memcmp(data.data(),cfg,sizeof(cfg)-1)!=0;
        return;
    }
    DWORD written{};
    no_cfg=!WriteFile(file.get(),cfg,sizeof(cfg)-1,&written,nullptr) || written!=sizeof(cfg)-1;
    if(no_cfg){file=Handle();DeleteFileW(path.c_str());}
}
struct Socket{
    SOCKET s=INVALID_SOCKET;
    ~Socket(){if(s!=INVALID_SOCKET)closesocket(s);}
};
bool receive(Socket& peer,std::string& label){
    auto deadline=Clock::now()+800ms;
    std::string packet;size_t end=0,length=0;
    char buf[4096];DWORD timeout=180;
    setsockopt(peer.s,SOL_SOCKET,SO_RCVTIMEO,reinterpret_cast<const char*>(&timeout),sizeof(timeout));
    while(!quitting && Clock::now()<deadline && packet.size()<65536){
        int n=recv(peer.s,buf,sizeof(buf),0);if(n<=0)return false;packet.append(buf,n);
        if(!end){
            auto at=packet.find("\r\n\r\n");if(at==packet.npos){if(packet.size()>8192)return false;continue;}
            if(packet.rfind("POST /masiva HTTP/1.",0)!=0)return false;
            auto header=packet.substr(0,at);
            std::transform(header.begin(),header.end(),header.begin(),[](unsigned char c){return static_cast<char>(std::tolower(c));});
            if(header.find("\r\norigin:")!=header.npos)return false;
            auto start=header.find("\r\ncontent-length:");if(start==header.npos)return false;
            start+=17;while(start<header.size() && header[start]==' ')++start;
            auto result=std::from_chars(header.data()+start,header.data()+header.size(),length);
            if(result.ec!=std::errc{} || length>60000 || !length)return false;
            end=at+4;
        }
        if(packet.size()>=end+length){
            try{label=details(Reader(std::string_view(packet).substr(end,length)).parse());}catch(...){return false;}
            constexpr char ok[]="HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Length: 0\r\n\r\n";
            send(peer.s,ok,sizeof(ok)-1,0);return true;
        }
    }
    return false;
}
// All pipe transfers are overlapped and time bounded. Discord may disappear during any frame.
bool transfer(HANDLE pipe,void* bytes,DWORD n,bool write){
    Handle event(CreateEventW(nullptr,TRUE,FALSE,nullptr));if(!event.valid())return false;
    OVERLAPPED op{};op.hEvent=event.get();DWORD count{};
    BOOL ok=write?WriteFile(pipe,bytes,n,&count,&op):ReadFile(pipe,bytes,n,&count,&op);
    if(!ok && GetLastError()==ERROR_IO_PENDING){
        if(WaitForSingleObject(event.get(),200)!=WAIT_OBJECT_0)CancelIoEx(pipe,&op);
        ok=GetOverlappedResult(pipe,&op,&count,TRUE);
    }
    return ok && count==n;
}
bool send_frame(HANDLE pipe,DWORD code,const std::string& json){
    DWORD head[]{code,static_cast<DWORD>(json.size())};
    std::string bytes(reinterpret_cast<char*>(head),sizeof(head));bytes+=json;
    return transfer(pipe,bytes.data(),static_cast<DWORD>(bytes.size()),true);
}
std::string command(const std::string& label,unsigned nonce){
    const auto activity=label.empty()?std::string("null"):
        "{\"details\":\""+label+"\",\"assets\":{\"large_image\":\"asuna\",\"large_text\":\"Masiva Skin 0.9.12 — Asuna\",\"small_image\":\"cs2\",\"small_text\":\"Counter-Strike 2\"}}";
    return "{\"cmd\":\"SET_ACTIVITY\",\"args\":{\"pid\":"+std::to_string(GetCurrentProcessId())+",\"activity\":"+activity+"},\"nonce\":\""+std::to_string(nonce)+"\"}";
}
void run(std::string id,const std::filesystem::path& exe){
    install_cfg(exe);
    WSADATA wd{};bool winsock=WSAStartup(MAKEWORD(2,2),&wd)==0;
    {
        Socket listener;
        if(winsock){
            listener.s=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
            sockaddr_in addr{};addr.sin_family=AF_INET;addr.sin_port=htons(port);addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
            BOOL exclusive=TRUE;
            setsockopt(listener.s,SOL_SOCKET,SO_EXCLUSIVEADDRUSE,reinterpret_cast<char*>(&exclusive),sizeof(exclusive));
            if(bind(listener.s,reinterpret_cast<sockaddr*>(&addr),sizeof(addr)) || listen(listener.s,4)){
                closesocket(listener.s);listener.s=INVALID_SOCKET;
            }
        }
        if(listener.s==INVALID_SOCKET)no_cfg=true;
        Handle pipe;bool ready=false;unsigned nonce=0;std::string current,last;
        Clock::time_point next{},publish{},seen{},handshake{};
        while(!quitting){
            auto now=Clock::now();
            if(!pipe.valid() && now>=next){
                status=Status::waiting;
                for(int i=0;i<10 && !pipe.valid();++i){
                    auto path=L"\\\\.\\pipe\\discord-ipc-"+std::to_wstring(i);
                    pipe=Handle(CreateFileW(path.c_str(),GENERIC_READ|GENERIC_WRITE,0,nullptr,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,nullptr));
                }
                if(pipe.valid() && !send_frame(pipe.get(),0,"{\"v\":1,\"client_id\":\""+id+"\"}"))pipe=Handle();
                next=now+10s;handshake=now+5s;ready=false;last.clear();publish={};
            }
            if(pipe.valid()){
                DWORD head[2]{},avail{},peeked{};
                bool ok=PeekNamedPipe(pipe.get(),head,sizeof(head),&peeked,&avail,nullptr)!=0;
                if(ok && peeked==sizeof(head)){
                    if(head[1]>65536)ok=false;
                    else if(avail>=sizeof(head)+head[1]){
                        std::string body(head[1],' ');
                        ok=transfer(pipe.get(),head,sizeof(head),false) && (!head[1] || transfer(pipe.get(),body.data(),head[1],false));
                        if(ok && head[0]==3)ok=send_frame(pipe.get(),4,body);
                        else if(ok && head[0]==2)ok=false;
                        else if(ok && head[0]==1){
                            std::string event;
                            try{event=Reader(body).parse().event;}catch(const std::invalid_argument&){ok=false;}
                            if(event=="READY"){ready=true;status=Status::connected;}
                            if(event=="ERROR"){status=Status::rejected;ok=false;}
                        }
                    }
                }
                std::string label=now-seen<45s && !current.empty()?current:"In Counter-Strike 2";
                if(ok && ready && label!=last && now>=publish){
                    ok=send_frame(pipe.get(),1,command(label,++nonce));last=label;publish=now+15s;
                }
                if(!ok || (!ready && now>handshake)){
                    pipe=Handle();next=now+10s;if(status!=Status::rejected)status=Status::waiting;
                }
            }
            if(listener.s!=INVALID_SOCKET){
                fd_set fds;FD_ZERO(&fds);FD_SET(listener.s,&fds);timeval wait{0,250000};
                if(select(0,&fds,nullptr,nullptr,&wait)>0){
                    Socket peer{accept(listener.s,nullptr,nullptr)};
                    if(peer.s!=INVALID_SOCKET && receive(peer,current))seen=Clock::now();
                }
            }else std::this_thread::sleep_for(250ms);
        }
        if(pipe.valid() && ready)send_frame(pipe.get(),1,command("",++nonce));
    }
    if(winsock)WSACleanup();
}
void reload(){
    stop();
    if(game_exe.empty() || !enabled())return;
    quitting=false;no_cfg=false;
    worker=std::thread([exe=game_exe]{try{run("1540031616603263046",exe);}catch(...){status=Status::unavailable;}});
}
struct ToggleState {bool on{},tr{};};
void caption(HWND w,const ToggleState& state){
    SetDlgItemTextW(w,IDC_RPC_ENABLE,state.on?(state.tr?L"Discord · Açık":L"Discord · On"):
                                               (state.tr?L"Discord · Kapalı":L"Discord · Off"));
}
INT_PTR CALLBACK dialog(HWND w,UINT m,WPARAM wp,LPARAM lp){
    auto* state=reinterpret_cast<ToggleState*>(GetWindowLongPtrW(w,DWLP_USER));
    if(m==WM_INITDIALOG){
        state=reinterpret_cast<ToggleState*>(lp);SetWindowLongPtrW(w,DWLP_USER,lp);
        wchar_t lang[8]{};
        GetPrivateProfileStringW(L"UI",L"OverlayLanguage",L"en",lang,8,ini().c_str());
        state->tr=wcscmp(lang,L"tr")==0;state->on=enabled();caption(w,*state);
        BOOL dark=TRUE;
        if(FAILED(DwmSetWindowAttribute(w,20,&dark,sizeof(dark))))DwmSetWindowAttribute(w,19,&dark,sizeof(dark));
        return TRUE;
    }
    if(m==WM_CTLCOLORDLG)return reinterpret_cast<INT_PTR>(GetStockObject(BLACK_BRUSH));
    if(m==WM_DRAWITEM && wp==IDC_RPC_ENABLE && state){
        const auto& item=*reinterpret_cast<DRAWITEMSTRUCT*>(lp);
        HDC dc=item.hDC;int saved=SaveDC(dc);RECT r=item.rcItem;
        FillRect(dc,&r,static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
        const int h=r.bottom-r.top, track=h/2, pad=h/5, x=r.right-pad-2*track, y=(h-track)/2;
        SelectObject(dc,GetStockObject(NULL_PEN));SelectObject(dc,GetStockObject(DC_BRUSH));
        SetDCBrushColor(dc,state->on?RGB(137,82,220):RGB(66,66,72));
        RoundRect(dc,x,y,x+2*track,y+track,track,track);
        const int knob=x+(state->on?track:0);
        SetDCBrushColor(dc,RGB(250,250,250));Ellipse(dc,knob+2,y+2,knob+track-2,y+track-2);
        SetTextColor(dc,RGB(242,242,246));SetBkMode(dc,TRANSPARENT);
        wchar_t label[64]{};GetWindowTextW(item.hwndItem,label,64);
        RECT text=r;text.left+=pad;text.right=x-pad;
        DrawTextW(dc,label,-1,&text,DT_LEFT|DT_SINGLELINE|DT_VCENTER);
        if((item.itemState&ODS_FOCUS) && !(item.itemState&ODS_NOFOCUSRECT)){InflateRect(&r,-1,-1);DrawFocusRect(dc,&r);}
        RestoreDC(dc,saved);return TRUE;
    }
    if(m==WM_COMMAND && (LOWORD(wp)==IDC_RPC_ENABLE || LOWORD(wp)==IDOK) && state){
        bool next=!state->on;
        if(!WritePrivateProfileStringW(L"Discord",L"Enabled",next?L"1":L"0",ini().c_str())){
            MessageBoxW(w,state->tr?L"Ayar kaydedilemedi.":L"Could not save this setting.",L"Masiva Skin",MB_ICONERROR);return TRUE;
        }
        state->on=next;caption(w,*state);InvalidateRect(GetDlgItem(w,IDC_RPC_ENABLE),nullptr,FALSE);
        if(next && game_exe.empty()){
            try{auto exe=steam_game();if(!exe.empty())install_cfg(exe);}catch(...){no_cfg=true;}
        }
        reload();return TRUE;
    }
    if(m==WM_CLOSE || (m==WM_COMMAND && LOWORD(wp)==IDCANCEL)){EndDialog(w,IDCANCEL);return TRUE;}
    return FALSE;
}
}
void stop(){quitting=true;if(worker.joinable())worker.join();status=Status::idle;}
bool is_enabled(){return enabled();}
void set_enabled(bool value){
 if(!WritePrivateProfileStringW(L"Discord",L"Enabled",value?L"1":L"0",ini().c_str()))throw std::runtime_error("Discord setting could not be saved.");
 if(value&&game_exe.empty()){try{auto exe=steam_game();if(!exe.empty())install_cfg(exe);}catch(...){no_cfg=true;}}
 reload();
}
void settings(HWND parent){ToggleState state;DialogBoxParamW(GetModuleHandleW(nullptr),MAKEINTRESOURCEW(IDD_RPC),parent,dialog,reinterpret_cast<LPARAM>(&state));}
void game_started(const std::wstring& exe){if(game_exe!=exe){game_exe=exe;reload();}}
}
