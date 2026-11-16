// Included inside game_bridge's namespace. Uses the existing game-thread callback.
struct GloveFields {
    std::uint16_t definition{}; int quality{};
    std::uint64_t item_id{}, split_id{};
    std::uint8_t initialized{}, disallow_soc{}, restore_material{};
};
struct GloveSaved { WeaponState state; float spawn{}; GloveFields fields; Values original,last; bool default_visible{}; };
std::vector<GloveSaved> glove_saved;
Retention glove_retention;
std::uint64_t next_glove_watch{}, next_visual_check{};
struct GloveRefresh { WeaponState state; float spawn{}; ModelSettle window; bool expect_custom{true}; };
std::optional<GloveRefresh> glove_refresh;
float glove_spawn(const WeaponState& s) { return memory.get<float>(s.pawn+0x14ec); }
void same_glove(const WeaponState& s) {
    if(!game->glove_snapshot().same_object(s))throw std::runtime_error("Eldiven oyuncusu degisti.");
}
GloveFields glove_fields(Address item) {
    return {memory.get<std::uint16_t>(item+0x1ba),memory.get<int>(item+0x1bc),
        memory.get<std::uint64_t>(item+0x1c8),memory.get<std::uint64_t>(item+0x1d0),
        memory.get<std::uint8_t>(item+0x1e8),memory.get<std::uint8_t>(item+0x1e9),
        memory.get<std::uint8_t>(item+0x1b8)};
}
void write_glove_fields(const WeaponState& s,const GloveFields& f) {
    same_glove(s);
    memory.put(s.item+0x1e8,std::uint8_t{0});
    memory.put(s.item+0x1ba,f.definition);memory.put(s.item+0x1bc,f.quality);
    memory.put(s.item+0x1c8,f.item_id);memory.put(s.item+0x1d0,f.split_id);
    memory.put(s.item+0x1e9,f.disallow_soc);memory.put(s.item+0x1b8,f.restore_material);
    memory.put(s.item+0x1e8,f.initialized);
}
Address glove_arms(const WeaponState& s) {
    const auto arms=entity(memory.get<std::uint32_t>(s.pawn+0x1dd8));
    if(memory.get<Address>(arms)!=client+binding::arms_vtable ||
        entity(memory.get<std::uint32_t>(arms+offsets::owner))!=s.pawn)
        throw std::runtime_error("Glove HUD arms identity/owner mismatch");
    return arms;
}
Address glove_attachment(Address arms) { return addon_entry(arms,"econ_gloves",0x3b3411b3); }
void glove_apply_ready(const WeaponState& s) {
    (void)glove_arms(s);
    if(memory.get<Address>(s.pawn+0x15f8+0xb0)!=s.pawn)
        throw std::runtime_error("Waiting for glove controller owner");
}
bool default_gloves_visible(const WeaponState& s) {
    return function<bool(*)(Address)>(binding::default_gloves_visible)(s.pawn);
}
void default_gloves(const WeaponState& s,bool visible) {
    same_glove(s);
    function<void(*)(Address,bool)>(binding::set_default_gloves)(s.pawn,visible);
}
Address glove_world(const WeaponState& s) {
    // Use the exact resolver used by the controller, including client-only
    // entities and the full handle serial. A nonzero handle is not a live model.
    return function<Address(*)(Address)>(binding::resolve_handle)(s.pawn+0x15f8);
}
void check_glove_resources(const WeaponState& s) {
    const auto path=function<Address(*)(Address)>(binding::glove_model_path)(s.item);
    if(!path || !memory.get<char>(pointer(path)))throw std::runtime_error("Glove definition has no model path");
    const auto paint=function<Address(*)(Address)>(binding::glove_paint_definition)(s.item);
    if(!paint)throw std::runtime_error("Glove paint kit is absent from the running game schema");
}
void refresh_glove(const WeaponState& s,bool restore_default=false) {
    same_glove(s);
    const auto arms=glove_arms(s);
    const auto controller=s.pawn+0x15f8;
    if(memory.get<Address>(controller+0xb0)!=s.pawn)
        throw std::runtime_error("Glove model controller owner mismatch");
    // c662eb/c662f7 deletes the custom world model on the next update if
    // firstperson_default/thirdperson_default is still selected. Transition
    // the native glove bodygroup rather than repeatedly respawning the addon.
    default_gloves(s,restore_default);
    function<void(*)(Address)>(binding::request_glove_refresh)(s.pawn);
    // Start the world resource load ONCE. Repeating it while materials are
    // pending prevents the dependent first-person glove from being created.
    function<void(*)(Address)>(binding::update_glove_model)(controller);
    // The first-person cache otherwise reuses the old finish when the world
    // entity handle stays the same. Native update owns destruction/recreation.
    if(const auto entry=glove_attachment(arms))memory.put(entry+8,std::uint32_t{0xffffffffU});
    glove_refresh=GloveRefresh{s,glove_spawn(s),ModelSettle{GetTickCount64()},!restore_default};
}
std::size_t find_glove(const WeaponState& s) {
    const auto spawn=glove_spawn(s);
    const auto pruned=retain_glove_lifetime(glove_saved,s,spawn);
    InterlockedExchangeAdd(&box->glove_pruned,static_cast<LONG>(pruned));
    InterlockedExchange(&box->glove_backups,static_cast<LONG>(glove_saved.size()));
    for(std::size_t i=0;i<glove_saved.size();++i)
        if(glove_saved[i].state.same_object(s) && glove_saved[i].spawn==spawn)return i;
    return glove_saved.size();
}
void apply_gloves(const Cosmetic& c,bool report=true) {
    validate(c);auto finish=catalog::finish(c.definition,c.paint);
    if(!catalog::glove(c.definition) || !finish || c.wear<finish->minimum || c.wear>finish->maximum)
        throw std::runtime_error("Eldiven/kaplama/float uyusmuyor.");
    auto s=game->glove_snapshot();
    if(s.team!=(c.team=="T"?2:3))throw std::runtime_error("Eldiven takimi uyusmuyor.");
    // Do not write attributes or create a rollback while respawn arms are absent.
    glove_apply_ready(s);
    auto before=effective(s.item);auto fields=glove_fields(s.item);auto i=find_glove(s);
    const bool fresh=i==glove_saved.size();
    const bool was_default=default_gloves_visible(s);
    if(fresh) {
        glove_saved.push_back({s,glove_spawn(s),fields,before,before,was_default});
    }
    auto desired=fields;desired.definition=static_cast<std::uint16_t>(c.definition);
    desired.quality=3;desired.item_id=~std::uint64_t{0};desired.split_id=~std::uint64_t{0};
    desired.initialized=1;desired.disallow_soc=1;desired.restore_material=1;
    try {
        std::snprintf(box->glove_details,sizeof(box->glove_details),"apply def=%d paint=%d; writing attributes",c.definition,c.paint);
        write_glove_fields(s,desired);
        attributes(s.item,{c.paint,c.seed,c.wear});
        auto result=effective(s.item);
        if(!equal(result,{c.paint,c.seed,c.wear}))throw std::runtime_error("Eldiven nitelik geri okumasi eslesmedi.");
        std::snprintf(box->glove_details,sizeof(box->glove_details),"apply def=%d paint=%d; resource preflight",c.definition,c.paint);
        check_glove_resources(s);
        refresh_glove(s);glove_saved[i].last=result;
        if(report) {
            box->weapon=s.pawn;box->effective_paint=result.paint;
            box->effective_seed=result.seed;box->effective_wear=result.wear;
        }
    } catch(...) {
        try {
            write_glove_fields(s,fields);attributes(s.item,before);refresh_glove(s,was_default);
            if(fresh)glove_saved.erase(glove_saved.begin()+i);
        } catch(...) { throw std::runtime_error("Eldiven islemi ve geri alma basarisiz; CS2'yi yeniden baslat."); }
        throw;
    }
}
void restore_gloves(bool clear_choice=true) {
    glove_refresh.reset();
    auto s=game->glove_snapshot();if(clear_choice){selections.clear(s.team,5030);}glove_retention.invalidate();
    auto i=find_glove(s);
    if(i==glove_saved.size())throw std::runtime_error("Eldiven secimi kapatildi; bu oyuncu icin yedek yok.");
    auto b=glove_saved[i];write_glove_fields(s,b.fields);attributes(s.item,b.original);refresh_glove(s,b.default_visible);
    glove_saved.erase(glove_saved.begin()+i);
}
void settle_gloves() {
    if(!glove_refresh)return;
    const auto now=GetTickCount64();
    if(glove_refresh->window.expired(now)) {
        std::snprintf(box->glove_message,sizeof(box->glove_message),"Glove load timed out; see last stage in glove_details");
        glove_refresh.reset();return;
    }
    if(!glove_refresh->window.due(now))return;
    try {
        const auto s=game->glove_snapshot();
        if(!s.same_object(glove_refresh->state) || glove_spawn(s)!=glove_refresh->spawn) {
            glove_refresh.reset();return;
        }
        const auto arms=glove_arms(s);
        if(glove_refresh->expect_custom && default_gloves_visible(s))default_gloves(s,false);
        // A missing world resource is not a pending first-person resource.
        // Retry the producer only when no live world entity exists; never set
        // the force-rebuild flag again for an existing asynchronous model.
        if(glove_refresh->expect_custom && glove_refresh->window.world_due(glove_world(s)!=0,now)) {
            const auto controller=s.pawn+0x15f8;
            if(memory.get<Address>(controller+0xb0)!=s.pawn)throw std::runtime_error("Glove controller owner changed");
            // Cached controller state can retain a dead world handle after a
            // bodygroup/respawn reset. Re-arm only after resolving it as absent.
            if(restart_missing_world(false,glove_refresh->window.world_attempts())) {
                function<void(*)(Address)>(binding::request_glove_refresh)(s.pawn);
                InterlockedIncrement(&box->glove_recoveries);
            }
            function<void(*)(Address)>(binding::update_glove_model)(controller);
        }
        function<void(*)(Address)>(binding::update_hud_glove)(arms);
        const auto world_handle=memory.get<std::uint32_t>(s.pawn+0x15f8);
        const auto entry=glove_attachment(arms);
        const auto hud_handle=entry?memory.get<std::uint32_t>(entry+12):0xffffffffU;
        int pending=-1;
        if(const auto world=glove_world(s))pending=memory.get<int>(world+0xac8);
        bool ready=entry && memory.get<std::uint32_t>(entry+8)==world_handle &&
            hud_handle!=0xffffffffU && pending==0;
        if(ready) (void)entity(hud_handle);
        if(!glove_refresh->expect_custom && !glove_world(s) && hud_handle==0xffffffffU) {
            std::snprintf(box->glove_message,sizeof(box->glove_message),"Original default glove bodygroup restored");
            glove_refresh.reset();return;
        }
        std::snprintf(box->glove_message,sizeof(box->glove_message),
            "Glove world=%u HUD=%u pending=%d ready=%d (visual check required)",world_handle,hud_handle,pending,ready?1:0);
        std::snprintf(box->glove_details,sizeof(box->glove_details),
            "def=%u paint=%d init=%u disallowSOC=%u world=%u hud=%u pending=%d producer_retries=%u default_bodygroup=%d",
            s.definition,effective(s.item).paint,memory.get<unsigned char>(s.item+0x1e8),
            memory.get<unsigned char>(s.item+0x1e9),world_handle,hud_handle,pending,glove_refresh->window.world_attempts(),default_gloves_visible(s)?1:0);
        if(ready)glove_refresh.reset();
    } catch(const std::exception& e) {
        // Arms may be briefly absent during equip/spawn. Retry inside the same
        // bounded window without restarting the world model load.
        std::snprintf(box->glove_message,sizeof(box->glove_message),"Glove settle: %s",e.what());
    }
}
void watch_gloves() {
    const auto now=GetTickCount64();
    if(now<next_glove_watch)return;
    next_glove_watch=now+150;
    WeaponState s;
    try { s=game->glove_snapshot(); } catch(...) {
        glove_retention.invalidate();glove_refresh.reset();next_visual_check=0;return;
    }
    auto c=selections.get(s.team,5030);if(!c) { glove_retention.invalidate();return; }
    try {
        // Keep a network/bodygroup update from deleting the custom glove.
        // Only enforce after a backup exists, so restore retains the original.
        const bool backed_up=find_glove(s)<glove_saved.size();
        if(backed_up && default_gloves_visible(s))default_gloves(s,false);
        AppearanceKey key{};key.pawn=s.pawn;key.identity=s.identity;key.handle=s.handle;key.team=s.team;
        auto spawn=glove_spawn(s);std::uint32_t bits{};std::memcpy(&bits,&spawn,sizeof(bits));key.world_model=bits;
        key.hud=memory.get<std::uint32_t>(s.pawn+0x1dd8);

        bool match=s.definition==c->definition && equal(effective(s.item),{c->paint,c->seed,c->wear}) &&
            memory.get<std::uint8_t>(s.item+0x1e8)==1 &&
            memory.get<std::uint8_t>(s.item+0x1e9)==1 &&
            memory.get<std::uint64_t>(s.item+0x1c8)==~std::uint64_t{0} &&
            memory.get<std::uint64_t>(s.item+0x1d0)==~std::uint64_t{0};
        if(!glove_retention.needs(key,match,now)) {
            // The profile's item fields can survive respawn while the first-person
            // attachment is removed or rebuilt later by the agent's arms.
            if(match && backed_up && !glove_refresh && now>=next_visual_check) {
                next_visual_check=now+1000;
                Address arms{};
                try {arms=glove_arms(s);} catch(const std::exception&) {return;}
                const auto entry=glove_attachment(arms);
                const auto world=glove_world(s);
                const auto handle=memory.get<std::uint32_t>(s.pawn+0x15f8);
                bool present=false;
                if(world && entry && memory.get<std::uint32_t>(entry+8)==handle) {
                    const auto hud=memory.get<std::uint32_t>(entry+12);
                    if(hud!=0xffffffffU) {
                        try {present=entity(hud)!=0;}catch(const std::exception&) {}
                    }
                }
                if(!present) {
                    if(world && entry && memory.get<int>(world+0xac8)==0)
                        memory.put(entry+8,std::uint32_t{0xffffffffU});
                    glove_refresh=GloveRefresh{s,spawn,ModelSettle{now},true};
                    next_visual_check=now+10000;
                }
            }
            return;
        }
        try { glove_apply_ready(s); }
        catch(const std::exception& e) {
            glove_retention.waiting_for_model(now);
            std::snprintf(box->glove_message,sizeof(box->glove_message),"Waiting for respawn arms (150 ms): %s",e.what());
            return;
        }
        apply_gloves(*c,false);glove_retention.applied(key,now);next_visual_check=now+1000;
        InterlockedIncrement(&box->glove_updates);
        std::snprintf(box->glove_message,sizeof(box->glove_message),"Eldiven nitelikleri uygulandi; motor yenilemesi istendi.");
    } catch(const std::exception& e) {
        glove_retention.failed(now);
        std::snprintf(box->glove_message,sizeof(box->glove_message),"%s",e.what());
    }
    InterlockedExchange(&box->pending_backups,static_cast<LONG>(saved.size()+glove_saved.size()));
}
