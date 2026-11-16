// World model and first-person arms are distinct engine objects.
struct AgentSaved {
    WeaponState state;float spawn{};std::string model;
    int requested_definition{};Address requested_binding{},verified_binding{},verified_data{};
    ModelLoadWindow load{};
};
std::optional<AgentSaved> agent_saved;
std::uint64_t next_agent_watch{};
unsigned agent_failures{};
struct AgentResource {Address binding{},data{};std::string path;};
AgentResource agent_resource(Address ent,bool read_path=true) {
    const auto node=pointer(memory.get<Address>(ent+0x330));
    AgentResource r;r.binding=memory.get<Address>(node+0x140+0xa0);
    if(!r.binding)return r;
    r.data=memory.get<Address>(pointer(r.binding));
    if(!r.data || !read_path)return r;
    // ac4586/ac458e obtains the name from the loaded CModel, not the optional
    // network ModelName symbol. 1778510 returns CModel+8 or an empty string.
    const auto address=function<Address(*)(Address)>(binding::resource_model_name)(pointer(r.data));
    if(!address)return r;
    std::string path;
    for(unsigned i=0;i<512;++i){auto ch=memory.get<char>(pointer(address)+i);if(!ch)break;path+=ch;}
    r.path=normalize_model_path(path);return r;
}
bool current_agent(const AgentSaved& b,const WeaponState& s) {
    return b.state.same_object(s) && b.spawn==glove_spawn(s);
}
bool sync_agent_arms(const WeaponState& s,Address world_binding) {
    auto resolve=function<Address(*)(Address)>(binding::resolve_handle);
    auto arms=resolve(s.pawn+0x1dd8);
    const auto ready=[&](Address value) {
        if(!value || memory.get<Address>(value)!=client+binding::arms_vtable)return false;
        if(resolve(value+offsets::owner)!=s.pawn)return false;
        const auto r=agent_resource(value,false);
        return r.binding==world_binding && r.data && memory.get<Address>(value+0x13b0)==world_binding;
    };
    if(ready(arms))return true;
    // Native controller compares arms+13b0 with the pawn's model resource,
    // recreates stale arms, and initializes them from that resource.
    function<void(*)(Address)>(binding::sync_player_arms)(s.pawn);
    arms=resolve(s.pawn+0x1dd8);
    return ready(arms);
}
void restore_agent(bool clear_choice=true) {
    voice_choice.store(0,std::memory_order_release);
    auto s=game->glove_snapshot();
    if(clear_choice){if(auto selected=selections.get(s.team,60000))selections.clear(s.team,selected->definition);}
    if(!agent_saved || !current_agent(*agent_saved,s))throw std::runtime_error("Bu oyuncu icin ajan yedegi yok.");
    const auto original_path=agent_saved->model;
    function<void(*)(Address,const char*)>(binding::set_model)(s.pawn,original_path.c_str());
    function<void(*)(Address)>(binding::sync_player_arms)(s.pawn);
    agent_saved.reset();agent_failures=0;next_agent_watch=0;InterlockedExchange(&box->agent_backup,0);
    std::snprintf(box->agent_message,sizeof(box->agent_message),"Original agent and arms update requested");
}
void watch_agent() {
    const auto now=GetTickCount64();if(now<next_agent_watch)return;
    next_agent_watch=now+250;
    voice_choice.store(0,std::memory_order_release);
    WeaponState s;
    try{s=game->glove_snapshot();}catch(...){agent_failures=0;return;}
    try {
        if(agent_saved && !current_agent(*agent_saved,s)) {
            agent_saved.reset();agent_failures=0;InterlockedExchange(&box->agent_backup,0);
        }
        const auto c=selections.get(s.team,60000);
        if(!c){agent_failures=0;return;}
        if(agent_failures>=8)return;
        const auto a=extras::agent(c->definition);
        if(!a || a->team!=s.team)throw std::runtime_error("Agent team mismatch");
        const auto desired=normalize_model_path(a->model);
        if(desired.empty())throw std::runtime_error("Agent catalog model path invalid");
        auto resource=agent_resource(s.pawn,false);
        bool world_ready=agent_saved && agent_saved->requested_definition==a->id && resource.data &&
            resource.binding==agent_saved->verified_binding && resource.data==agent_saved->verified_data;
        if(!world_ready)resource=agent_resource(s.pawn);
        world_ready=world_ready || resource.path==desired;
        if(!agent_saved) {
            if(resource.path.empty())throw std::runtime_error("Original agent resource/name not ready; no model changed");
            const auto spawn=glove_spawn(s);
            if(!std::isfinite(spawn))throw std::runtime_error("Agent spawn identity invalid");
            agent_saved=AgentSaved{s,spawn,resource.path};
        }
        auto& b=*agent_saved;
        if(b.requested_definition!=a->id) {
            b.load.clear();b.requested_definition=a->id;b.verified_binding=b.verified_data=0;agent_failures=0;
        }
        if(!world_ready) {
            if(b.load.expired(now)){b.load.clear();throw std::runtime_error("Requested agent resource did not finish loading in 5 seconds");}
            if(!b.load.pending(now)) {
                if(!current_agent(b,game->glove_snapshot()))throw std::runtime_error("Agent pawn changed before set-model");
                InterlockedExchange(&box->agent_backup,1);
                function<void(*)(Address,const char*)>(binding::set_model)(s.pawn,desired.c_str());
                b.requested_binding=agent_resource(s.pawn,false).binding;b.load.begin(now);
                InterlockedIncrement(&box->agent_requests);
                resource=agent_resource(s.pawn);world_ready=resource.path==desired;
            }
            if(!world_ready) {
            std::snprintf(box->agent_message,sizeof(box->agent_message),"Agent %d: waiting for loaded model resource",a->id);
            std::snprintf(box->agent_details,sizeof(box->agent_details),"target=%s actual=%s binding=%llx requested=%llx failures=%u",desired.c_str(),resource.path.c_str(),static_cast<unsigned long long>(resource.binding),static_cast<unsigned long long>(b.requested_binding),agent_failures);
            next_agent_watch=now+50;return;
            }
        }
        b.load.clear();b.verified_binding=resource.binding;b.verified_data=resource.data;
        if(box->voice_ready)publish_voice(s,a->id);
        const bool arms_ready=sync_agent_arms(s,resource.binding);
        if(arms_ready)agent_failures=0;
        std::snprintf(box->agent_message,sizeof(box->agent_message),"Agent %d: world verified; first-person arms=%s (visual check required)",a->id,arms_ready?"ready":"pending");
        std::snprintf(box->agent_details,sizeof(box->agent_details),"target=%s world_binding=%llx world_data=%llx arms_handle=%u ready=%d",desired.c_str(),static_cast<unsigned long long>(resource.binding),static_cast<unsigned long long>(resource.data),memory.get<std::uint32_t>(s.pawn+0x1dd8),arms_ready?1:0);
        next_agent_watch=now+(arms_ready?250:100);
    }catch(const std::exception& e){
        ++agent_failures;next_agent_watch=now+1000;
        std::snprintf(box->agent_message,sizeof(box->agent_message),"Agent: %s [attempt %u/8]",e.what(),agent_failures);
    }
}
