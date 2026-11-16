// SoundOpSystem001 (not SoundSystem001). This callback changes one local
// playback descriptor, never a network message, recipient list or voice chat.
using SoundStartFn=void*(*)(void*,void*,const void*);
SoundStartFn sound_start_original{};
Address sound_module{},sound_object{};
std::uint32_t jump_throw_m{0xffffffffU},jump_throw_f{0xffffffffU};
std::atomic<std::uint64_t> voice_choice{};
std::atomic_flag voice_guard=ATOMIC_FLAG_INIT;
struct alignas(8) SoundStart {
    Address source{},parameters{};
    std::uint32_t hash{};
    std::int32_t entity{};
    std::uint32_t guid{},flags{},parameter_bytes{};
    std::uint16_t channel{};
    std::uint8_t option_a{},option_b{},debug{};
};
static_assert(offsetof(SoundStart,hash)==0x10 && offsetof(SoundStart,entity)==0x14 && offsetof(SoundStart,debug)==0x28);
void publish_voice(const WeaponState& s,int agent) {
    const auto identity=pointer(memory.get<Address>(s.pawn+offsets::identity));
    const auto handle=memory.get<std::uint32_t>(identity+0x10);
    voice_choice.store((static_cast<std::uint64_t>(agent)<<32)|handle,std::memory_order_release);
}
void* start_voice(void* self,void* out,const void* descriptor) {
    const auto chosen=voice_choice.load(std::memory_order_acquire);
    if(!chosen || reinterpret_cast<Address>(self)!=sound_object || !descriptor || !box || box->initialized!=1)
        return sound_start_original(self,out,descriptor);
    SoundStart copy{};bool changed=false;
    if(!voice_guard.test_and_set(std::memory_order_acquire)) {
        try {
            memory.read_bytes(reinterpret_cast<Address>(descriptor),&copy,0x29);
            const auto handle=static_cast<std::uint32_t>(chosen);
            const auto agent=extras::agent(static_cast<int>(chosen>>32));
            const bool direct=local_voice_source(copy.entity,handle);
            const bool jump_event=copy.hash!=0xffffffffU && (copy.hash==jump_throw_m || copy.hash==jump_throw_f);
            if(agent && (direct || (jump_event && copy.entity>0 && copy.entity<0x8000))) {
                const auto pawn=memory.get<Address>(client+offsets::local_pawn);
                const auto actual=entity(handle);
                bool local_source=direct;
                if(!direct && actual==pawn) {
                    // A jump-throw effort may be emitted by the held grenade.
                    // Only these two exact hashes may use local entity ownership.
                    const auto emitter=entity(static_cast<std::uint32_t>(copy.entity));
                    const auto owner=memory.get<std::uint32_t>(emitter+offsets::owner);
                    local_source=owner==handle && entity(owner)==pawn;
                }
                if(local_source && actual==pawn && memory.get<std::uint8_t>(pawn+offsets::team)==agent->team &&
                    memory.get<std::uint32_t>(pointer(memory.get<Address>(pawn+offsets::identity))+0x10)==handle) {
                    InterlockedIncrement(&box->voice_local_events);
                    const auto dict=sound_object+8;
                    using Name=const char*(*)(Address,std::uint32_t);
                    using Hash=std::uint32_t(*)(Address,const char*,bool);
                    using Exists=bool(*)(Address,std::uint32_t);
                    const auto get_name=reinterpret_cast<Name>(sound_module+audio_binding::name_rva);
                    const auto get_hash=reinterpret_cast<Hash>(sound_module+audio_binding::hash_rva);
                    const auto exists=reinterpret_cast<Exists>(sound_module+audio_binding::exists_rva);
                    const auto name=get_name(dict,copy.hash);
                    std::array<char,192> text{};
                    bool terminated=false;
                    if(name) {
                        const auto start=reinterpret_cast<Address>(name);
                        for(std::size_t offset=0;offset<text.size();) {
                            const auto address=start+offset;
                            // Stay on the current Windows x64 page. A terminator
                            // at its end must not require the next page to be readable.
                            const auto length=std::min({std::size_t{32},text.size()-offset,
                                static_cast<std::size_t>(4096-(address&4095))});
                            memory.read_bytes(address,text.data()+offset,length);
                            if(std::memchr(text.data()+offset,0,length)){terminated=true;break;}
                            offset+=length;
                        }
                    }
                    if(terminated) {
                        const auto group=voice_group(text.data(),agent->voice);
                        const auto body=body_voice_event(text.data(),agent->voice);
                        const char* replacement=nullptr;
                        if(group || body) {
                            InterlockedIncrement(&box->voice_matches);
                            // Bounded attempts, no polling, new sounds or resource loading.
                            const auto attempts=body?1U:std::min(8U,group->count);
                            for(unsigned i=0;i<attempts;++i) {
                                const auto candidate=body?body:voice_data::events[group->first+(copy.hash+i)%group->count];
                                const auto hash=get_hash(dict,candidate,true);
                                if(hash!=0xffffffffU && exists(dict,hash)) {
                                    copy.hash=hash;changed=true;replacement=candidate;
                                    InterlockedIncrement(&box->voice_replacements);
                                    std::snprintf(box->voice_message,sizeof(box->voice_message),"%s -> %s",text.data(),candidate);
                                    break;
                                }
                            }
                            if(!changed)std::snprintf(box->voice_message,sizeof(box->voice_message),"Voice unavailable; original retained: %s -> %s",text.data(),agent->voice);
                        } else if(box->voice_matches==0) {
                            std::snprintf(box->voice_message,sizeof(box->voice_message),"Local audio observed: %.180s; target=%s",text.data(),agent->voice);
                        }
                        if(agent_voice_event(text.data())) {
                            // Fixed-size recent history includes unmapped voice calls;
                            // footsteps cannot evict the evidence we need for diagnosis.
                            const auto n=static_cast<unsigned long>(box->voice_history_count)%12U;
                            std::snprintf(box->voice_history[n],sizeof(box->voice_history[n]),
                                "%.100s -> %.100s [bank=%s]",text.data(),replacement?replacement:"original retained",agent->voice);
                            InterlockedIncrement(&box->voice_history_count);
                        }
                    }
                }
            }
        } catch(...) { InterlockedIncrement(&box->voice_errors); }
        voice_guard.clear(std::memory_order_release);
    }
    // Exactly one original call, outside exception handling and all locks.
    return sound_start_original(self,out,changed?&copy:descriptor);
}
void install_voices() {
    sound_module=reinterpret_cast<Address>(GetModuleHandleW(L"soundsystem.dll"));
    if(!sound_module)throw std::runtime_error("soundsystem.dll not loaded; original voices retained");
    const auto dos=memory.get<IMAGE_DOS_HEADER>(sound_module);
    if(dos.e_magic!=IMAGE_DOS_SIGNATURE || dos.e_lfanew<=0 || dos.e_lfanew>0x1000)throw std::runtime_error("Sound module PE invalid");
    const auto nt=memory.get<IMAGE_NT_HEADERS64>(sound_module+dos.e_lfanew);
    if(nt.Signature!=IMAGE_NT_SIGNATURE || nt.FileHeader.TimeDateStamp!=audio_binding::timestamp || nt.OptionalHeader.SizeOfImage!=audio_binding::image_size)
        throw std::runtime_error("Sound module build mismatch; original voices retained");
    for(const auto& c:audio_binding::required) {
        std::array<std::uint8_t,32> bytes{};memory.read_bytes(sound_module+c.rva,bytes.data(),bytes.size());
        if(bytes!=c.bytes)throw std::runtime_error("Sound function signature mismatch; original voices retained");
    }
    sound_object=pointer(memory.get<Address>(client+audio_binding::client_global));
    if(memory.get<Address>(sound_object)!=sound_module+audio_binding::vtable ||
       memory.get<Address>(sound_object+8)!=sound_module+audio_binding::dictionary_vtable)
        throw std::runtime_error("SoundOpSystem001 identity mismatch");
    for(const auto& entry:audio_binding::slots)
        if(memory.get<Address>(sound_module+entry.first)!=sound_module+entry.second)
            throw std::runtime_error("SoundOpSystem001 virtual method mismatch");
    sound_start_original=reinterpret_cast<SoundStartFn>(sound_module+audio_binding::start_rva);
    using Hash=std::uint32_t(*)(Address,const char*,bool);
    const auto hash=reinterpret_cast<Hash>(sound_module+audio_binding::hash_rva);
    jump_throw_m=hash(sound_object+8,"BaseGrenade.JumpThrowM",true);
    jump_throw_f=hash(sound_object+8,"BaseGrenade.JumpThrowF",true);
    auto slot=reinterpret_cast<void**>(sound_module+audio_binding::vtable+0x58);
    DWORD protection{},ignored{};
    if(!VirtualProtect(slot,sizeof(void*),PAGE_READWRITE,&protection))throw std::runtime_error("Sound callback page unavailable");
    const auto previous=InterlockedCompareExchangePointer(slot,reinterpret_cast<void*>(&start_voice),reinterpret_cast<void*>(sound_start_original));
    const bool restored=VirtualProtect(slot,sizeof(void*),protection,&ignored)!=FALSE;
    if(previous!=reinterpret_cast<void*>(sound_start_original) || !restored)throw std::runtime_error("Sound callback install failed");
    InterlockedExchange(&box->voice_ready,1);
    std::snprintf(box->voice_message,sizeof(box->voice_message),"Local agent voice playback ready; select/apply an agent");
}
