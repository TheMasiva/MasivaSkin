#pragma once
#include <windows.h>
#include <shellapi.h>
#include <array>
#include <atomic>
#include "../menu_policy.hpp"
#include "../release.hpp"
namespace masiva {
inline constexpr UINT menu_toggle_message=WM_APP+70,menu_key_message=WM_APP+71,menu_tray_message=WM_APP+72;
struct MenuHost {
    HWND window{};std::atomic<HWND> game{};
    HHOOK keyboard{};HANDLE worker{},started{};DWORD worker_id{};
    std::atomic<unsigned> key{menu::default_key};
    std::atomic<bool> capturing{false};
    std::array<bool,256> held{},swallowed{};
    inline static MenuHost* active{};
    ~MenuHost(){if(worker){PostThreadMessageW(worker_id,WM_QUIT,0,0);WaitForSingleObject(worker,INFINITE);CloseHandle(worker);}if(started)CloseHandle(started);if(active==this)active=nullptr;
        NOTIFYICONDATAW n{};n.cbSize=sizeof(n);n.hWnd=window;n.uID=1;Shell_NotifyIconW(NIM_DELETE,&n);}
    static LRESULT CALLBACK keys(int code,WPARAM w,LPARAM l) {
        auto* self=active;
        if(code<0 || !self)return CallNextHookEx(nullptr,code,w,l);
        const auto& info=*reinterpret_cast<const KBDLLHOOKSTRUCT*>(l);const auto k=info.vkCode;
        if(k>=256)return CallNextHookEx(nullptr,code,w,l);
        const bool down=w==WM_KEYDOWN || w==WM_SYSKEYDOWN;
        if(!down) {
            self->held[k]=false;
            if(self->swallowed[k]){self->swallowed[k]=false;return 1;}
            return CallNextHookEx(nullptr,code,w,l);
        }
        const bool first=!self->held[k];self->held[k]=true;
        if(self->swallowed[k])return 1;
        if(!first || (!self->capturing && k!=self->key))
            return CallNextHookEx(nullptr,code,w,l);
        const auto foreground=GetForegroundWindow();
        const bool own=foreground==self->window;
        if(self->capturing && own && first && (menu::valid_key(k) || k==VK_ESCAPE)) {
            self->swallowed[k]=true;PostMessageW(self->window,menu_key_message,k,0);return 1;
        }
        wchar_t cls[32]{};if(own){GUITHREADINFO gui{};gui.cbSize=sizeof(gui);if(GetGUIThreadInfo(GetWindowThreadProcessId(self->window,nullptr),&gui))GetClassNameW(gui.hwndFocus,cls,32);}
        const bool editing=_wcsicmp(cls,L"Edit")==0 || _wcsicmp(cls,L"ComboBox")==0;
        const bool modifiers=(GetAsyncKeyState(VK_CONTROL)&0x8000) || (GetAsyncKeyState(VK_MENU)&0x8000) ||
            (GetAsyncKeyState(VK_SHIFT)&0x8000) || (GetAsyncKeyState(VK_LWIN)&0x8000) || (GetAsyncKeyState(VK_RWIN)&0x8000);
        const auto game_window=self->game.load();
        if(first && k==self->key && menu::toggle_allowed(game_window && foreground==game_window,own,
            !IsWindowEnabled(self->window),editing,modifiers,k)) {
            self->swallowed[k]=true;PostMessageW(self->window,menu_toggle_message,0,0);return 1;
        }
        return CallNextHookEx(nullptr,code,w,l);
    }
    static DWORD WINAPI pump(void* context) {
        auto* self=static_cast<MenuHost*>(context);MSG message{};
        PeekMessageW(&message,nullptr,WM_USER,WM_USER,PM_NOREMOVE);
        self->keyboard=SetWindowsHookExW(WH_KEYBOARD_LL,keys,GetModuleHandleW(nullptr),0);
        SetEvent(self->started);
        while(GetMessageW(&message,nullptr,0,0)>0){TranslateMessage(&message);DispatchMessageW(&message);}
        if(self->keyboard)UnhookWindowsHookEx(self->keyboard);
        return 0;
    }
    bool install(HWND target,unsigned selected) {
        window=target;key=menu::valid_key(selected)?selected:menu::default_key;active=this;
        started=CreateEventW(nullptr,TRUE,FALSE,nullptr);
        if(started)worker=CreateThread(nullptr,0,pump,this,0,&worker_id);
        if(worker)WaitForSingleObject(started,INFINITE);
        NOTIFYICONDATAW n{};n.cbSize=sizeof(n);n.hWnd=window;n.uID=1;n.uFlags=NIF_ICON|NIF_TIP|NIF_MESSAGE;
        n.hIcon=LoadIconW(GetModuleHandleW(nullptr),MAKEINTRESOURCEW(101));n.uCallbackMessage=menu_tray_message;
        wcscpy_s(n.szTip,release::window_title);Shell_NotifyIconW(NIM_ADD,&n);
        return keyboard!=nullptr;
    }
    void locate_game(DWORD connected_pid) {
        if(connected_pid) {
            const auto cached=game.load();
            DWORD owner{};
            if(cached && IsWindowVisible(cached) && !GetWindow(cached,GW_OWNER) &&
               GetWindowThreadProcessId(cached,&owner) && owner==connected_pid)return;
        }
        // No game memory or process injection: enumerate only visible windows.
        struct Search { DWORD pid; HWND found; } search{connected_pid,nullptr};
        EnumWindows([](HWND h,LPARAM data)->BOOL {
            if(!IsWindowVisible(h) || GetWindow(h,GW_OWNER))return TRUE;
            DWORD pid{};GetWindowThreadProcessId(h,&pid);
            auto& search=*reinterpret_cast<Search*>(data);
            if(search.pid) {
                if(pid!=search.pid)return TRUE;
                search.found=h;return FALSE;
            }
            const auto process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid);
            if(!process)return TRUE;
            wchar_t path[1024]{};DWORD length=1024;
            const bool read=QueryFullProcessImageNameW(process,0,path,&length)!=FALSE;CloseHandle(process);
            if(!read)return TRUE;
            const auto name=wcsrchr(path,L'\\');
            if(_wcsicmp(name?name+1:path,L"cs2.exe")!=0)return TRUE;
            search.found=h;return FALSE;
        },reinterpret_cast<LPARAM>(&search));
        game.store(search.found);
    }
    void show() {
        RECT target{};
        if(game && IsWindow(game) && !IsIconic(game))GetWindowRect(game,&target);
        else {MONITORINFO m{};m.cbSize=sizeof(m);GetMonitorInfoW(MonitorFromWindow(window,MONITOR_DEFAULTTONEAREST),&m);target=m.rcWork;}
        RECT r{};GetWindowRect(window,&r);
        const auto x=target.left+std::max<LONG>(0,(target.right-target.left-(r.right-r.left))/2);
        const auto y=target.top+std::max<LONG>(0,(target.bottom-target.top-(r.bottom-r.top))/2);
        SetWindowPos(window,game?HWND_TOPMOST:HWND_NOTOPMOST,x,y,0,0,SWP_NOSIZE|SWP_SHOWWINDOW);
        SetForegroundWindow(window);SetFocus(window);
    }
    void hide(bool return_to_game=true) {
        capturing=false;ShowWindow(window,SW_HIDE);
        SetWindowPos(window,HWND_NOTOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
        if(return_to_game && game && IsWindow(game))SetForegroundWindow(game);
    }
    void toggle(){if(IsWindowVisible(window))hide();else show();}
};
}
