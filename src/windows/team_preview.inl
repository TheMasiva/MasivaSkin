using PreviewTick=void(*)(Address);
PreviewTick preview_original{};
struct PreviewItemBackup {
    GloveFields fields{};
    std::array<std::optional<std::uint32_t>,3> paint_attributes{};
    DecorationState decorations{};StatAttributes stat{};
    std::array<ItemLabel,3> names{};
    Cosmetic choice{};std::uint64_t applied_id{};
};
struct PreviewEntry {
    Address object{},identity{};std::uint32_t handle{};int team{},weapon_definition{};
    std::array<std::optional<PreviewItemBackup>,3> slots;
    std::uint64_t verify_at{},retry_at{};
};
std::vector<PreviewEntry> preview_entries;
std::uint64_t preview_prune_at{};
std::uint32_t preview_serial=0x40000000;
constexpr std::array<Address,3> preview_items{0x620,0xbd0,0x1180};
bool preview_live(const PreviewEntry& e) {
    try{return entity(e.handle)==e.object && memory.get<Address>(e.object+offsets::identity)==e.identity && memory.get<std::uint32_t>(e.identity+0x10)==e.handle;}
    catch(...){return false;}
}
void preview_fields(Address item,const GloveFields& f) {
    memory.put(item+0x1e8,std::uint8_t{0});
    memory.put(item+0x1ba,f.definition);memory.put(item+0x1bc,f.quality);
    memory.put(item+0x1c8,f.item_id);memory.put(item+0x1d0,f.split_id);
    memory.put(item+0x1e9,f.disallow_soc);memory.put(item+0x1b8,f.restore_material);
    memory.put(item+0x1e8,f.initialized);
}
PreviewItemBackup preview_backup(Address item) {
    (void)effective(item); // exact item getter ABI check before mutation
    PreviewItemBackup b;b.fields=glove_fields(item);
    for(unsigned i=0;i<3;++i){b.paint_attributes[i]=stat_attribute(item,static_cast<std::uint16_t>(6+i));b.names[i]=memory.get<ItemLabel>(item+0x399+i*161);}
    b.decorations=read_decorations(item,&b.stat);return b;
}
void preview_restore(Address item,const PreviewItemBackup& b) {
    static constexpr const char* names[]={"set item texture prefab","set item texture seed","set item texture wear"};
    for(unsigned i=0;i<3;++i) {
        if(b.paint_attributes[i])function<void(*)(Address,const char*,float)>(binding::set_attribute)(item,names[i],integer_attribute(*b.paint_attributes[i]));
        else function<void(*)(Address,std::uint16_t)>(binding::remove_attribute)(item,static_cast<std::uint16_t>(6+i));
        memory.put(item+0x399+i*161,b.names[i]);
    }
    write_decorations(item,b.decorations);stat_attributes(item,b.stat);preview_fields(item,b.fields);
}
std::uint64_t preview_local_id() {
    const auto pawn=pointer(memory.get<Address>(client+offsets::local_pawn));
    const auto controller=entity(memory.get<std::uint32_t>(pawn+offsets::pawn_controller));
    if(entity(memory.get<std::uint32_t>(controller+offsets::controller_pawn))!=pawn)return 0;
    return memory.get<std::uint64_t>(controller+offsets::controller_steam_id);
}
int preview_weapon(Address object,int team) {
    const auto definition=memory.get<std::uint16_t>(object+0x1180+offsets::definition);
    if(catalog::firearm(definition)||catalog::knife(definition))return definition;
    const auto address=memory.get<Address>(object+0x610);if(!address)return 0;
    std::string name;
    for(unsigned i=0;i<64;++i){auto c=memory.get<char>(address+i);if(!c)break;name+=c;}
    static constexpr std::pair<const char*,int> names[]={{"weapon_ak47",7},{"weapon_m4a1",16},{"weapon_m4a1_silencer",60},{"weapon_awp",9},{"weapon_deagle",1},{"weapon_glock",4},{"weapon_hkp2000",32},{"weapon_usp_silencer",61},{"weapon_famas",10},{"weapon_galilar",13},{"weapon_aug",8},{"weapon_sg556",39},{"weapon_ssg08",40},{"weapon_scar20",38},{"weapon_g3sg1",11},{"weapon_mp9",34},{"weapon_mac10",17},{"weapon_mp7",33},{"weapon_mp5sd",23},{"weapon_ump45",24},{"weapon_p90",19},{"weapon_bizon",26},{"weapon_nova",35},{"weapon_xm1014",25},{"weapon_mag7",27},{"weapon_sawedoff",29},{"weapon_m249",14},{"weapon_negev",28},{"weapon_p250",36},{"weapon_fiveseven",3},{"weapon_tec9",30},{"weapon_elite",2},{"weapon_cz75a",63},{"weapon_revolver",64},{"weapon_taser",31}};
    if(name=="weapon_knife"||name=="weapon_knife_t")return team==2?59:42;
    for(const auto& n:names)if(name==n.first)return n.second;
    return 0;
}
void preview_apply(Address item,PreviewItemBackup& b,const Cosmetic& choice) {
    auto f=b.fields;f.definition=static_cast<std::uint16_t>(choice.definition);
    f.quality=choice.stattrak>=0?9:(catalog::knife(choice.definition)?3:b.fields.quality);
    f.item_id=f.split_id=(std::uint64_t{0xffffffff}<<32)|++preview_serial;
    f.initialized=1;f.disallow_soc=1;f.restore_material=1;
    preview_fields(item,f);
    if(choice.category!="agent") {
        attributes(item,{choice.paint,choice.seed,choice.wear});
        stat_attributes(item,choice.stattrak<0?StatAttributes{}:StatAttributes{static_cast<std::uint32_t>(choice.stattrak),0U});
        write_decorations(item,decoration_values(choice.decorations));
        function<void(*)(Address,const char*,int)>(binding::set_custom_name)(item,choice.custom_name.c_str(),0);
        if(!equal(effective(item),{choice.paint,choice.seed,choice.wear}))throw std::runtime_error("Team preview attribute verification failed");
    }
    b.choice=choice;b.applied_id=f.split_id;InterlockedIncrement(&box->preview_updates);
}
void preview_update(Address object) {
    const auto now=GetTickCount64();
    auto it=std::find_if(preview_entries.begin(),preview_entries.end(),[&](const auto& e){return e.object==object;});
    const bool visible=memory.get<std::uint8_t>(object+0x1ce9)!=0;
    // Dormant unmodified scenes need no local-controller lookup or lifetime sweep.
    if(!visible && it==preview_entries.end()){InterlockedIncrement(&box->preview_idle_skips);return;}
    if(now>=preview_prune_at) {
        preview_prune_at=now+1000;
        preview_entries.erase(std::remove_if(preview_entries.begin(),preview_entries.end(),[](const auto& e){return !preview_live(e);}),preview_entries.end());
        it=std::find_if(preview_entries.begin(),preview_entries.end(),[&](const auto& e){return e.object==object;});
    }
    // Keep per-object identity validation even between global sweep ticks.
    if(it!=preview_entries.end()&&!preview_live(*it)) {preview_entries.erase(it);it=preview_entries.end();}
    if(it!=preview_entries.end()&&now<it->retry_at)return;
    const auto team=memory.get<int>(object+0x1ce0);
    const bool local=visible && preview_policy::local_character(preview_local_id(),memory.get<std::uint64_t>(object+0x618),team,true);
    if(!local) {
        InterlockedIncrement(&box->preview_nonlocal);
        if(it!=preview_entries.end()) {for(unsigned i=0;i<3;++i)if(it->slots[i])preview_restore(object+preview_items[i],*it->slots[i]);preview_entries.erase(it);}
        return;
    }
    InterlockedIncrement(&box->preview_local_calls);
    if(it==preview_entries.end()) {
        if(preview_entries.size()>=8)throw std::runtime_error("Team preview lifetime limit reached");
        PreviewEntry e;e.object=object;e.identity=pointer(memory.get<Address>(object+offsets::identity));e.handle=memory.get<std::uint32_t>(e.identity+0x10);e.team=team;
        if(!preview_live(e))throw std::runtime_error("Team preview identity mismatch");
        preview_entries.push_back(e);it=std::prev(preview_entries.end());
    }
    auto& e=*it;
    try {
        if(e.slots[2] && now>=e.verify_at && memory.get<std::uint64_t>(object+0x1180+0x1d0)!=e.slots[2]->applied_id)e.slots[2].reset();
        if(!e.slots[2])e.weapon_definition=preview_weapon(object,team);
        const auto original_weapon=e.weapon_definition;
        const Cosmetic* choices[]={selections.get(team,60000),selections.get(team,5028),selections.get(team,original_weapon)};
        for(unsigned i=0;i<3;++i) {
            auto& backup=e.slots[i];const auto item=object+preview_items[i];const auto c=choices[i];
            if(!c){if(backup){preview_restore(item,*backup);backup.reset();}continue;}
            if(!preview_policy::needs_item_read(backup.has_value(),backup&&same_cosmetic(backup->choice,*c),now>=e.verify_at)) {
                InterlockedIncrement(&box->preview_item_skips);continue;
            }
            const auto id=memory.get<std::uint64_t>(item+0x1d0);
            if(!preview_policy::should_update(backup.has_value(),backup&&same_cosmetic(backup->choice,*c),backup&&id==backup->applied_id,now>=e.verify_at))continue;
            if(backup && backup->applied_id && id!=backup->applied_id)backup.reset(); // network supplied a fresh item
            if(!backup)backup=preview_backup(item);
            preview_apply(item,*backup,*c);
        }
        preview_policy::advance_verification(e.verify_at,now);e.retry_at=0;
        std::snprintf(box->preview_message,sizeof(box->preview_message),"Local team preview updated; visual result unverified");
    } catch(...) {e.retry_at=now+1000;throw;}
}
void preview_tick(Address self) {
    if(!box||box->initialized!=1||box->preview_ready!=1){preview_original(self);return;}
    InterlockedIncrement(&box->preview_ticks);
    if(frame_thread!=GetCurrentThreadId()){InterlockedIncrement(&box->preview_thread_skips);preview_original(self);return;}
    if(InterlockedCompareExchange(&callback_lock,1,0)!=0){InterlockedIncrement(&box->preview_busy_skips);preview_original(self);return;}
    {
    WorkTimer timer(box->preview_work_us,box->preview_work_calls,box->preview_max_us);
    try {preview_update(self);}catch(const std::exception& e){InterlockedIncrement(&box->preview_errors);std::snprintf(box->preview_message,sizeof(box->preview_message),"%s",e.what());}
    }
    InterlockedExchange(&callback_lock,0);
    preview_original(self); // native code owns creation, animation and material rebuilds
}
void install_team_preview() {
    for(const auto& code:preview_binding::required){std::array<std::uint8_t,32> bytes{};memory.read_bytes(client+code.rva,bytes.data(),bytes.size());if(bytes!=code.bytes)throw std::runtime_error("Team preview binary contract mismatch");}
    for(auto vt:preview_binding::vtables)if(memory.get<Address>(client+vt+146*8)!=client+preview_binding::tick.rva)throw std::runtime_error("Team preview callback contract mismatch");
    preview_original=reinterpret_cast<PreviewTick>(client+preview_binding::tick.rva);
    for(auto vt:preview_binding::vtables) {
        auto slot=reinterpret_cast<void**>(client+vt+146*8);DWORD old{},unused{};
        if(!VirtualProtect(slot,sizeof(void*),PAGE_READWRITE,&old))throw std::runtime_error("Team preview callback page unavailable");
        const auto previous=InterlockedCompareExchangePointer(slot,reinterpret_cast<void*>(&preview_tick),reinterpret_cast<void*>(preview_original));
        const bool protected_again=VirtualProtect(slot,sizeof(void*),old,&unused)!=FALSE;
        if(previous!=reinterpret_cast<void*>(preview_original)||!protected_again)throw std::runtime_error("Team preview callback installation incomplete");
    }
    InterlockedExchange(&box->preview_ready,1);
    std::snprintf(box->preview_message,sizeof(box->preview_message),"Waiting for local team intro");
}
