using HudThink=void(*)(void*);
HudThink hud_original{};
std::uint64_t hud_update_at{};
struct DialogToken { std::uint32_t hash{},padding{};const char* text{}; };
struct HudPresentation {
    Address pawn{};
    std::uint32_t spawn{},active{};
    int team{},rows{},definition{},paint{},stattrak{};
    bool english{};
    ItemLabel name{};
    std::array<Address,16> images{};
    bool operator==(const HudPresentation& other)const {
        return pawn==other.pawn && spawn==other.spawn && active==other.active && team==other.team &&
            rows==other.rows && definition==other.definition && paint==other.paint &&
            stattrak==other.stattrak && english==other.english && name==other.name && images==other.images;
    }
};
struct HudRowMemo { Address panel{};std::uint32_t handle{};HudWriteCache<HudPresentation> write; };
std::vector<HudRowMemo> hud_rows,next_hud_rows;
void hud_tick(void* self) {
    hud_original(self);
    const auto now=GetTickCount64();
    if(now<hud_update_at)return;
    hud_update_at=now+100;
    if(!box || box->initialized!=1 || !frame_thread || frame_thread!=GetCurrentThreadId() ||
        InterlockedCompareExchange(&callback_lock,1,0)!=0)return;
    WorkTimer timer(box->hud_work_us,box->hud_work_calls,box->hud_max_us);
    try {
        update_hud_language();
        const auto element=reinterpret_cast<Address>(self);
        const auto view=element-0x98;
        if(memory.get<Address>(element)!=client+binding::hud_vtable ||
            memory.get<Address>(view)!=client+binding::hud_primary_vtable)
            throw std::runtime_error("HUD nesne kimligi farkli.");
        auto s=game->player_snapshot();
        if(entity(memory.get<std::uint32_t>(element+0x50))!=s.pawn)
            throw std::runtime_error("HUD yerel oyuncuyu gostermiyor.");
        const auto count=memory.get<int>(view+0x50);
        if(count<0 || count>64)throw std::runtime_error("HUD satir sayisi gecersiz.");
        const auto services=pointer(memory.get<Address>(s.pawn+offsets::weapon_services));
        const auto active=memory.get<std::uint32_t>(services+offsets::active_weapon);
        const auto spawn=memory.get<std::uint32_t>(s.pawn+0x14ec);
        auto& retained=next_hud_rows;
        retained.clear();retained.reserve(count);
        if(count) {
            const auto rows=pointer(memory.get<Address>(view+0x58));
            for(int i=0;i<count;++i) {
                const auto row=rows+static_cast<Address>(i)*0x48;
                const auto handle=memory.get<std::uint32_t>(row+0x38);
                auto dirty=std::find(dirty_hud.begin(),dirty_hud.end(),handle);
                const bool changed=dirty!=dirty_hud.end();
                if(changed && InterlockedCompareExchange(&box->full_hud_refresh,0,0)!=0) {
                    function<int(*)(Address,int)>(binding::clear_hud_icon)(view,i);
                    dirty_hud.erase(dirty);retained.clear();
                    InterlockedIncrement(&box->hud_row_rebuilds);
                    break;
                }
                if(changed)dirty_hud.erase(dirty);
                WeaponState held;
                try {held=game->held_weapon(handle);}catch(...) {continue;}
                if(!game->owned_weapon(held))continue;
                int definition{},paint{},stattrak{-1};ItemLabel name{};
                const auto selected=selections.get(s.team,held.definition);
                if(catalog::knife(held.definition) && selected) {
                    // The HUD row can exist before the first-person knife is drawn.
                    // Present its selected name/icon without modifying a holstered model.
                    definition=selected->definition;paint=selected->paint;stattrak=selected->stattrak;
                    std::copy(selected->custom_name.begin(),selected->custom_name.end(),name.begin());
                } else {
                    const auto index=find_saved(held);
                    if(index==saved.size() || !saved[index].applied)continue;
                    const auto& b=saved[index];definition=b.last_definition;paint=b.last.paint;
                    stattrak=b.last_stattrak;name=b.last_name;
                }
                if(!catalog::finish(definition,paint))continue;
                const auto panel=pointer(memory.get<Address>(row+8));
                (void)pointer(memory.get<Address>(panel+8));
                HudPresentation stamp{s.pawn,spawn,active,s.team,count,definition,paint,stattrak,hud_english,name,{}};
                const auto icon=knife_icon(definition);
                int icons=0;
                if(icon) {
                    icons=memory.get<int>(row+0x10);
                    if(icons<0 || icons>16)throw std::runtime_error("HUD icon count invalid");
                    if(icons) {
                        const auto list=pointer(memory.get<Address>(row+0x18));
                        memory.read_bytes(list,stamp.images.data(),icons*sizeof(Address));
                        for(int n=0;n<icons;++n)(void)pointer(stamp.images[n]);
                    }
                }
                HudRowMemo memo{panel,handle,{}};
                const auto previous=std::find_if(hud_rows.begin(),hud_rows.end(),[&](const auto& m){return m.panel==panel && m.handle==handle;});
                if(previous!=hud_rows.end())memo=*previous;
                if(changed || memo.write.needs(stamp,now)) {
                    const auto label=hud_item_label(definition,paint,hud_english,stattrak,name);
                    if(icons) {
                        const auto path=std::string("file://{images}/icons/equipment/")+icon+".svg";
                        for(int n=0;n<icons;++n)
                            function<void(*)(Address,const char*)>(binding::hud_image_source)(stamp.images[n],path.c_str());
                    }
                    const DialogToken token{0x01614551,0,"WeaponIcon--name"};
                    function<void(*)(Address,const DialogToken*,const char*)>(binding::set_dialog_text)(panel,&token,label.data());
                    memo.write.committed(stamp,now);
                    InterlockedIncrement(&box->hud_updates);
                    std::snprintf(box->hud_message,sizeof(box->hud_message),"Yerel HUD adi: %s",label.data());
                } else InterlockedIncrement(&box->hud_skipped);
                retained.push_back(std::move(memo));
            }
        }
        hud_rows.swap(retained);
    } catch(const std::exception& e) {
        hud_rows.clear();
        std::snprintf(box->hud_message,sizeof(box->hud_message),"HUD: %s",e.what());
    } catch(...) {hud_rows.clear();std::snprintf(box->hud_message,sizeof(box->hud_message),"HUD C++ hatasi.");}
    InterlockedExchange(&callback_lock,0);
}
