#pragma once
#include "process.hpp"
#include "bridge_protocol.hpp"
#include "../bridge_startup.hpp"
#include <filesystem>

namespace masiva {
class BridgeClient {
    DWORD pid_{};
    Handle mapping_, loader_process_, loader_thread_;
    bridge::Mailbox* box_{};
    void* remote_path_{};
    DWORD sequence_{};
    bool attempted_{}, loader_completed_{};
    DWORD loader_exit_{};
    std::filesystem::path dll_path_;
    bool module_loaded() const {
        Handle modules(CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,pid_));
        if(!modules.valid())throw win_error("Kopru modul listesi okunamadi");
        MODULEENTRY32W m{};m.dwSize=sizeof(m);
        if(!Module32FirstW(modules.get(),&m))throw win_error("Kopru modul listesi bos");
        do {if(_wcsicmp(m.szModule,L"MasivaSkinBridge.dll")==0)return true;}while(Module32NextW(modules.get(),&m));
        return false;
    }
    bool open_mapping() {
        if(box_) return true;
        Handle h(OpenFileMappingW(FILE_MAP_ALL_ACCESS,FALSE,bridge::name(pid_).c_str()));
        if(!h.valid()) return false;
        auto p=static_cast<bridge::Mailbox*>(MapViewOfFile(h.get(),FILE_MAP_ALL_ACCESS,0,0,sizeof(bridge::Mailbox)));
        if(!p) throw win_error("Kopru yaniti okunamadi");
        if(p->magic==0) { UnmapViewOfFile(p); return false; }
        if(p->magic!=bridge::magic || p->protocol!=bridge::version || p->pid!=pid_) {
            UnmapViewOfFile(p); throw std::runtime_error("Kopru protokolu uyusmuyor; CS2'yi yeniden baslat.");
        }
        mapping_=std::move(h); box_=p;
        return true;
    }
    void reclaim_path() {
        if(loader_thread_.valid() && WaitForSingleObject(loader_thread_.get(),0)==WAIT_OBJECT_0) {
            if(!GetExitCodeThread(loader_thread_.get(),&loader_exit_))throw win_error("DLL yukleme sonucu okunamadi");
            loader_completed_=true;
            if(remote_path_) VirtualFreeEx(loader_process_.get(),remote_path_,0,MEM_RELEASE);
            remote_path_=nullptr; loader_thread_=Handle{}; loader_process_=Handle{};
        }
    }
    Address remote_loader() {
        auto local=GetProcAddress(GetModuleHandleW(L"kernel32.dll"),"LoadLibraryW");
        HMODULE owner{};
        if(!local || !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCWSTR>(local),&owner)) throw win_error("Windows yukleyicisi bulunamadi");
        wchar_t path[32768]{};
        if(!GetModuleFileNameW(owner,path,32768)) throw win_error("Windows modul yolu okunamadi");
        auto basename=std::filesystem::path(path).filename().wstring();
        Handle snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,pid_));
        if(!snapshot.valid()) throw win_error("Windows modulleri okunamadi");
        MODULEENTRY32W m{}; m.dwSize=sizeof(m);
        if(!Module32FirstW(snapshot.get(),&m)) throw win_error("Windows modulu bulunamadi");
        do {
            if(_wcsicmp(m.szModule,basename.c_str())==0) {
                auto offset=reinterpret_cast<Address>(local)-reinterpret_cast<Address>(owner);
                if(offset>=m.modBaseSize) throw std::runtime_error("Windows yukleyici adresi gecersiz.");
                return reinterpret_cast<Address>(m.modBaseAddr)+offset;
            }
        } while(Module32NextW(snapshot.get(),&m));
        throw std::runtime_error("Windows yukleyici modulu CS2'de bulunamadi.");
    }
public:
    explicit BridgeClient(DWORD pid):pid_(pid) { open_mapping(); }
    ~BridgeClient() {
        try {reclaim_path();}catch(...){}
        // If LoadLibrary has not returned, its argument must stay allocated.
        // CS2 owns that outstanding allocation until completion/process exit.
        if(box_) UnmapViewOfFile(box_);
    }
    BridgeClient(const BridgeClient&)=delete;
    BridgeClient& operator=(const BridgeClient&)=delete;
    const bridge::Mailbox* status() const { return box_; }
    bool active_backup() const { return box_ && InterlockedCompareExchange(&box_->active_backup,0,0)!=0; }
    bool glove_backup() const { return box_ && InterlockedCompareExchange(&box_->glove_backup,0,0)!=0; }
    LONG backup_count() const { return box_ ? InterlockedCompareExchange(&box_->pending_backups,0,0) : 0; }
    void ensure() {
        reclaim_path();
        if(!open_mapping() && !attempted_) {
            Handle modules(CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,pid_));
            MODULEENTRY32W module{}; module.dwSize=sizeof(module);
            if(modules.valid() && Module32FirstW(modules.get(),&module)) {
                do {
                    if(_wcsicmp(module.szModule,L"MasivaSkinBridge.dll")==0)
                        throw std::runtime_error("Eski veya hazir olmayan Masiva koprusu yuklu. CS2'yi yeniden baslat.");
                } while(Module32NextW(modules.get(),&module));
            }
            wchar_t exe[32768]{};
            if(!GetModuleFileNameW(nullptr,exe,32768)) throw win_error("Uygulama yolu okunamadi");
            auto dll=std::filesystem::path(exe).parent_path()/L"MasivaSkinBridge.dll";
            if(!std::filesystem::is_regular_file(dll)) throw std::runtime_error("MasivaSkinBridge.dll EXE'nin yaninda bulunamadi. ZIP'i tamamen cikart.");
            dll_path_=dll;
            auto path=dll.wstring(); auto bytes=(path.size()+1)*sizeof(wchar_t);
            loader_process_=Handle(OpenProcess(PROCESS_CREATE_THREAD|PROCESS_QUERY_INFORMATION|PROCESS_VM_OPERATION|PROCESS_VM_WRITE|PROCESS_VM_READ,FALSE,pid_));
            if(!loader_process_.valid()) throw win_error("Oyun ici kopru yukleme erisimi reddedildi");
            auto entry=remote_loader();
            remote_path_=VirtualAllocEx(loader_process_.get(),nullptr,bytes,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
            if(!remote_path_) throw win_error("Kopru yolu icin bellek ayrilamadi");
            SIZE_T done{};
            if(!WriteProcessMemory(loader_process_.get(),remote_path_,path.c_str(),bytes,&done) || done!=bytes) {
                VirtualFreeEx(loader_process_.get(),remote_path_,0,MEM_RELEASE); remote_path_=nullptr;
                throw win_error("Kopru yolu yazilamadi");
            }
            loader_thread_=Handle(CreateRemoteThread(loader_process_.get(),nullptr,0,reinterpret_cast<LPTHREAD_START_ROUTINE>(entry),remote_path_,0,nullptr));
            if(!loader_thread_.valid()) {
                VirtualFreeEx(loader_process_.get(),remote_path_,0,MEM_RELEASE); remote_path_=nullptr;
                throw win_error("Windows DLL yukleme istegi baslatilamadi");
            }
            attempted_=true;
        }
        const auto until=GetTickCount64()+30000;
        do {
            reclaim_path();
            const bool mapped=open_mapping();
            const auto phase=startup::observe(mapped,mapped?InterlockedCompareExchange(&box_->initialized,0,0):0,
                loader_completed_,mapped || !loader_completed_ || module_loaded());
            if(phase==startup::State::rejected)throw std::runtime_error(box_->message);
            if(phase==startup::State::ready)return;
            if(phase==startup::State::load_failed)
                throw std::runtime_error("Windows MasivaSkinBridge.dll dosyasini yukleyemedi (loader exit="+std::to_string(loader_exit_)+"). CS2 Trusted Mode may block third-party software. Check Steam > CS2 > Properties > General > Launch Options: -allow_third_party_software, then fully restart CS2 and Masiva. This is a troubleshooting suggestion, not a confirmed cause for every load failure. See README; keep matching EXE/DLL files.");
            Sleep(10);
        } while(GetTickCount64()<until);
        throw std::runtime_error(box_ ? "Kopru DLL yuklendi fakat baslatma tamamlanmadi. Tani raporunda Bridge startup stage alanina bak; CS2'yi yeniden baslat." :
            loader_completed_ ? "DLL yuklendi fakat kopru paylasimi acilmadi. Tani raporunu kaydet ve CS2'yi yeniden baslat." :
            "Windows DLL yukleme islemi halen bekliyor. Yeni yukleme baslatilmadi veya zorla sonlandirilmadi. Tani raporunu kaydet.");
    }
    std::string loader_report() const {
        std::ostringstream out;
        out<<"Loader attempted: "<<attempted_<<"\nLoader completed: "<<loader_completed_
           <<"\nLoader thread pending: "<<loader_thread_.valid()<<"\nLoader exit (low DWORD, not GetLastError): "<<loader_exit_;
        const auto path=dll_path_.u8string();out<<"\nLoader DLL path: "<<path;
        try{out<<"\nLoader module present: "<<module_loaded();}catch(const std::exception& e){out<<"\nLoader module query: "<<e.what();}
        if(box_)out<<"\nBridge startup stage: "<<box_->startup_stage<<"\nBridge startup status: "<<box_->message;
        out<<'\n';return out.str();
    }
    bool hud_compatibility_=false;
    std::string request(DWORD operation,const WeaponState& expected,const Cosmetic* c=nullptr,const ProfilePacket* profile=nullptr) {
        if(c)validate(*c);
        if(profile)(void)decode_profile(*profile);
        ensure();
        InterlockedExchange(&box_->full_hud_refresh,hud_compatibility_?1:0);
        auto state=InterlockedCompareExchange(&box_->state,0,0);
        if(state==bridge::Queued || state==bridge::Running)
            throw std::runtime_error("Onceki motor istegi henuz tamamlanmadi; ikinci istek gonderilmedi.");
        // Observe pending completion before reusing the slot.
        InterlockedExchange(&box_->state,bridge::Idle);
        sequence_=box_->request_id+1;
        box_->request_id=sequence_; box_->operation=operation;
        box_->expected_pawn=expected.pawn; box_->expected_identity=expected.identity; box_->expected_handle=expected.handle;
        if(c) { box_->decorations=c->decorations; box_->team=c->team=="T"?2:3; box_->definition=c->definition; box_->paint=c->paint; box_->seed=c->seed; box_->wear=c->wear; box_->stattrak=c->stattrak;
            std::memset(box_->custom_name,0,sizeof(box_->custom_name));
            std::memcpy(box_->custom_name,c->custom_name.data(),c->custom_name.size()); }
        if(profile)box_->profile=*profile;
        box_->deadline=GetTickCount64()+10000;
        InterlockedExchange(&box_->state,bridge::Queued);
        auto until=GetTickCount64()+11000;
        do {
            if(InterlockedCompareExchange(&box_->state,0,0)==bridge::Done && box_->completed_id==sequence_) {
                std::string result(box_->message);
                if(!box_->success) throw std::runtime_error(result);
                return result;
            }
            Sleep(10);
        } while(GetTickCount64()<until);
        // Cancel only if the callback hasn't taken ownership. Never interrupt a running engine call.
        if(InterlockedCompareExchange(&box_->state,bridge::Idle,bridge::Queued)==bridge::Queued)
            throw std::runtime_error("Oyun guncellemesi gelmedi; istek iptal edildi. Oyun ekranina donup tekrar dene.");
        throw std::runtime_error("Motor isteginin sonucu henuz yok; yeni istek gonderme. Tani raporunu kaydet.");
    }
    void hud_mode(bool enabled) {if(box_)InterlockedExchange(&box_->full_hud_refresh,enabled?1:0);hud_compatibility_=enabled;}
    std::string request_profile(const std::vector<Cosmetic>& items) {
        auto packet=encode_profile(items);
        return request(bridge::QueueProfile,WeaponState{},nullptr,&packet);
    }

};
}
