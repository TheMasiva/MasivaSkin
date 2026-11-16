using DeathNotice=void(*)(void*,Address);
DeathNotice death_original{};
struct EventToken { std::uint32_t hash{},index{0xffffffffU};const char* name{}; };
using EventString=const char*(*)(Address,const EventToken*,const char*);
struct DeathContext { Address event{};EventString original{};const char* icon{};DWORD thread{}; } death_context;
std::string event_text(Address address) {
    std::string out;
    if(!address)return out;
    for(unsigned i=0;i<128;++i){auto c=memory.get<char>(address+i);if(!c)return out;out+=c;}
    throw std::runtime_error("Kill feed text exceeds limit");
}
EventToken event_token(const char* name) {
    auto n=std::strlen(name);
    return {function<std::uint32_t(*)(const char*,std::size_t,std::uint32_t)>(binding::event_token_hash)
        (name,n,0x31415926U^static_cast<std::uint32_t>(n)),0xffffffffU,name};
}
const char* death_string(Address event,const EventToken* token,const char* fallback) {
    const auto& c=death_context;
    const auto result=c.original(event,token,fallback);
    if(event==c.event && GetCurrentThreadId()==c.thread && token && c.icon) {
        try {
            if(event_text(reinterpret_cast<Address>(token->name))=="weapon" &&
                knife_event(event_text(reinterpret_cast<Address>(result))))return c.icon;
        } catch(...) {}
    }
    return result;
}
// Only this synchronous HUD listener sees a different GetString result.
// No event payload, event manager, network message, or other listener is edited.
struct ScopedDeathView {
    Address event{},original{};
    std::array<Address,33> table{}; // retain RTTI at [-1], preserve other virtual entries
    ScopedDeathView(Address e,Address vt,const char* icon):event(e),original(vt) {
        MEMORY_BASIC_INFORMATION info{};
        if(!VirtualQuery(reinterpret_cast<void*>(e),&info,sizeof(info)) ||
            info.State!=MEM_COMMIT || (info.Protect&0xff)!=PAGE_READWRITE || (info.Protect&PAGE_GUARD))
            throw std::runtime_error("Kill feed event is not writable");
        memory.read_bytes(vt-sizeof(Address),table.data(),sizeof(table));
        death_context={e,reinterpret_cast<EventString>(table[11]),icon,GetCurrentThreadId()};
        table[11]=reinterpret_cast<Address>(&death_string);
        const auto previous=InterlockedCompareExchangePointer(reinterpret_cast<void**>(e),table.data()+1,reinterpret_cast<void*>(vt));
        if(previous!=reinterpret_cast<void*>(vt))throw std::runtime_error("Kill feed event changed");
    }
    ~ScopedDeathView() { InterlockedCompareExchangePointer(reinterpret_cast<void**>(event),reinterpret_cast<void*>(original),table.data()+1); }
    ScopedDeathView(const ScopedDeathView&)=delete;
};
void death_tick(void* self,Address event) {
    if(!box || box->initialized!=1 || frame_thread!=GetCurrentThreadId() ||
        InterlockedCompareExchange(&callback_lock,1,0)!=0){death_original(self,event);return;}
    bool invoked=false;
    try {
        if(memory.get<Address>(reinterpret_cast<Address>(self))!=client+binding::death_vtable)
            throw std::runtime_error("Kill feed listener type mismatch");
        const auto vt=pointer(memory.get<Address>(event));
        const auto name_fn=reinterpret_cast<const char*(*)(Address)>(memory.get<Address>(vt+8));
        const auto name=event_text(reinterpret_cast<Address>(name_fn(event)));
        if(name=="player_death") {
            InterlockedIncrement(&box->death_events);
            auto weapon_token=event_token("weapon"),attacker_token=event_token("attacker");
            auto get_string=reinterpret_cast<EventString>(memory.get<Address>(vt+0x50));
            const auto weapon=event_text(reinterpret_cast<Address>(get_string(event,&weapon_token,"")));
            int attacker=-1,local=-1;
            reinterpret_cast<int*(*)(Address,int*,const EventToken*)>(memory.get<Address>(vt+0x78))(event,&attacker,&attacker_token);
            function<int*(*)(int*,int)>(binding::local_player_slot)(&local,0);
            if(local>=0 && attacker==local && knife_event(weapon)) {
                const auto player=game->player_snapshot();
                const auto choice=selections.get(player.team,42);
                bool applied=false;
                if(choice)for(const auto& b:saved)if(b.applied && b.state.pawn==player.pawn &&
                    b.state.team==player.team && b.last_definition==choice->definition && catalog::knife(b.last_definition))applied=true;
                const auto icon=killfeed_icon(name,weapon,attacker,local,choice?choice->definition:0,applied);
                if(icon) {
                    ScopedDeathView view(event,vt,icon);
                    invoked=true;death_original(self,event);
                    InterlockedIncrement(&box->death_icons);
                    std::snprintf(box->death_message,sizeof(box->death_message),"Local knife icon requested: %s (visual unverified)",icon);
                }
            }
        }
    } catch(const std::exception& e) {
        std::snprintf(box->death_message,sizeof(box->death_message),"Kill feed: %s",e.what());
    } catch(...) {std::snprintf(box->death_message,sizeof(box->death_message),"Kill feed update failed");}
    if(!invoked)death_original(self,event);
    InterlockedExchange(&callback_lock,0);
}
void install_death_notice() {
    auto slot=reinterpret_cast<void**>(client+binding::death_vtable+8);
    death_original=reinterpret_cast<DeathNotice>(client+binding::death_notice.rva);
    if(memory.get<Address>(reinterpret_cast<Address>(slot))!=client+binding::death_notice.rva)
        throw std::runtime_error("Kill feed callback mismatch");
    DWORD protection{},ignored{};
    if(!VirtualProtect(slot,sizeof(void*),PAGE_READWRITE,&protection))throw std::runtime_error("Kill feed callback unavailable");
    const auto previous=InterlockedCompareExchangePointer(slot,reinterpret_cast<void*>(&death_tick),reinterpret_cast<void*>(death_original));
    const bool restored=VirtualProtect(slot,sizeof(void*),protection,&ignored)!=FALSE;
    if(previous!=reinterpret_cast<void*>(death_original) || !restored)throw std::runtime_error("Kill feed callback install failed");
    std::snprintf(box->death_message,sizeof(box->death_message),"Local knife kill feed ready; waiting for a kill");
}
