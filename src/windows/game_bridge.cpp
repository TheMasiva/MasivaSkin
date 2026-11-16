#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include "bridge_protocol.hpp"
#include "binding.hpp"
#include "audio_binding.hpp"
#include "../agent_voice.hpp"
#include <atomic>
#include "../collection_tools.hpp"
#include "../team_preview_policy.hpp"
#include "preview_binding.hpp"
#include "../game.hpp"
#include "../module_contract.hpp"
#include "../glove_lifetime.hpp"
#include "../retention.hpp"
#include "../selections.hpp"
#include "../shared_selection.hpp"
#include "../item_label.hpp"
#include <shellapi.h>
#include "../cosmetic_refresh.hpp"
#include "../stat_attributes.hpp"
#include "../decorations.hpp"
#include "../model_loading.hpp"
#include "../runtime_policy.hpp"
#include "../killfeed.hpp"
#include <memory>
#include <algorithm>
#include <cstdio>

namespace {
using namespace masiva;
class LocalMemory final : public Memory {
public:
    bool alive() const override { return true; }
    void read_bytes(Address a, void* p, std::size_t n) const override {
        SIZE_T done{};
        if (!ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(a), p, n, &done) || done != n)
            throw std::runtime_error("Oyun ici bellek okunamadi.");
    }
    void write_bytes(Address a, const void* p, std::size_t n) override {
        SIZE_T done{};
        if (!WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(a), p, n, &done) || done != n)
            throw std::runtime_error("Oyun ici bellek yazilamadi.");
    }
} memory;
Address client{}, engine{};
bridge::Mailbox* box{};
std::unique_ptr<Game> game;
class WorkTimer {
    LARGE_INTEGER started{};
    std::uint64_t& total;
    volatile LONG& count;
    volatile LONG& maximum;
public:
    WorkTimer(std::uint64_t& sum,volatile LONG& calls,volatile LONG& peak):total(sum),count(calls),maximum(peak) {
        QueryPerformanceCounter(&started);
    }
    ~WorkTimer() {
        static const LONGLONG frequency=[](){LARGE_INTEGER f{};QueryPerformanceFrequency(&f);return f.QuadPart;}();
        LARGE_INTEGER end{};QueryPerformanceCounter(&end);
        if(frequency<=0)return;
        const auto us=static_cast<std::uint64_t>((end.QuadPart-started.QuadPart)*1000000/frequency);
        total+=us;InterlockedIncrement(&count);
        if(us>static_cast<std::uint64_t>(maximum))maximum=static_cast<LONG>(std::min<std::uint64_t>(us,0x7fffffff));
    }
};

using FrameFn = void(*)(void*, int);
FrameFn original{};
volatile LONG callback_lock{};
DWORD frame_thread{};
struct Values { int paint{}, seed{}; float wear{}; };
struct Mesh { Address node{}; std::uint64_t mask{}; };
struct Saved { WeaponState state; Values original, last; Mesh world, hud; bool applied{}; std::uint32_t subclass{}; int quality{}; int stattrak{-1}; int last_stattrak{-1}; std::array<ItemLabel,3> names{}; ItemLabel last_name{}; StatAttributes stat_attributes{}; std::uint64_t parts_until{}; DecorationState decorations{}; int last_definition{}; AppearanceMemo memo{}; Decorations last_decorations{}; bool refresh_requested{}, rollback_requested{}; MaterialCommit material_commit{}; std::uint64_t verify_at{},parts_at{}; };
std::vector<Saved> saved;
std::vector<std::uint32_t> dirty_hud;
Selections selections;
std::optional<std::uint64_t> transition_at;
Retention retention;
std::uint64_t next_watch{}, knife_ready_at{};

template<class F> F function(const binding::Code& c) { return reinterpret_cast<F>(client + c.rva); }
void same(const WeaponState& s) {
    if (!game->snapshot().same_entity(s)) throw std::runtime_error("Islem sirasinda silah degisti.");
}
Address entity(std::uint32_t h) {
    if (h == 0xffffffffU || !(h & 0x7fffU)) throw std::runtime_error("HUD entity handle gecersiz.");
    auto list = pointer(memory.get<Address>(client + offsets::entity_list));
    auto chunk = pointer(memory.get<Address>(list + 0x10 + 8 * ((h & 0x7fffU) >> 9)));
    auto slot = chunk + offsets::entity_stride * (h & 0x1ffU);
    auto p = pointer(memory.get<Address>(slot));
    if (memory.get<Address>(p + offsets::identity) != slot) throw std::runtime_error("HUD entity kimligi uyusmuyor.");
    return p;
}
Address hud_weapon(const WeaponState& s) {
    auto arms = entity(memory.get<std::uint32_t>(s.pawn + 0x1dd8));
    auto node = pointer(memory.get<Address>(arms + 0x330));
    std::array<Address,64> seen{};
    node = memory.get<Address>(node + 0x40);
    for (std::size_t n = 0; node && n < seen.size(); ++n) {
        pointer(node);
        if (std::find(seen.begin(), seen.begin()+n, node) != seen.begin()+n)
            throw std::runtime_error("HUD dugumlerinde dongu bulundu.");
        seen[n] = node;
        auto owner = memory.get<Address>(node + 0x30);
        if (owner && memory.get<std::uint32_t>(pointer(owner) + offsets::owner) == s.handle) return owner;
        node = memory.get<Address>(node + 0x48);
    }
    throw std::runtime_error("Aktif silahin HUD modeli bulunamadi; oyun ekranina donup tekrar uygula.");
}
Mesh mesh(Address ent) {
    auto node = pointer(memory.get<Address>(ent + 0x330));
    return {node, memory.get<std::uint64_t>(node + 0x140 + 0x208)};
}
void set_mesh(const Mesh& m, std::uint64_t mask) {
    const auto model = m.node + 0x140;
    // The companion dirty-model state is validated before either write.
    auto dirty = memory.get<Address>(model + 0xd8);
    if (dirty) (void)memory.get<std::uint64_t>(pointer(dirty) + 0x10);
    memory.put(model + 0x208, mask);
    if (dirty) memory.put(dirty + 0x10, mask);
}
Values effective(Address item) {
    // Confirm the exact item implementation before using the verified ABI.
    auto vt = pointer(memory.get<Address>(item));
    if (memory.get<Address>(vt + 0x18) != client + binding::item_paint.rva ||
        memory.get<Address>(vt + 0x20) != client + binding::item_seed.rva ||
        memory.get<Address>(vt + 0x28) != client + binding::item_wear.rva)
        throw std::runtime_error("Item sanal metotlari desteklenen surumle eslesmiyor.");
    Values v;
    v.paint = function<int(*)(Address)>(binding::item_paint)(item);
    v.seed = function<int(*)(Address)>(binding::item_seed)(item);
    v.wear = function<float(*)(Address,float)>(binding::item_wear)(item, 0.0f);
    if (v.paint < 0 || v.seed < 0 || v.seed > 1000 || !std::isfinite(v.wear) || v.wear < 0 || v.wear > 1)
        throw std::runtime_error("Etkin kaplama degerleri gecersiz.");
    return v;
}
bool equal(const Values& a, const Values& b) {
    return a.paint == b.paint && a.seed == b.seed && std::abs(a.wear-b.wear) < 0.00001f;
}
void attributes(Address item, const Values& v) {
    auto set = function<void(*)(Address,const char*,float)>(binding::set_attribute);
    set(item, "set item texture prefab", static_cast<float>(v.paint));
    set(item, "set item texture seed", static_cast<float>(v.seed));
    set(item, "set item texture wear", v.wear);
}
std::optional<std::uint32_t> stat_attribute(Address item,std::uint16_t id) {
    return read_stat_attribute(memory,item,id);
}
void stat_attributes(Address item,const StatAttributes& values) {
    write_stat_attributes(memory,item,values,
        [](Address object,const char* name,float value) {
            function<void(*)(Address,const char*,float)>(binding::set_attribute)(object,name,value);
        },[](Address object,std::uint16_t id) {
            function<void(*)(Address,std::uint16_t)>(binding::remove_attribute)(object,id);
        });
}
#include "decorations.inl"
// Native HUD addon maps share this verified layout, but their update methods
// differ from the world C_EconEntity builders.
Address addon_entry(Address model,const char* name,std::uint32_t hash) {
    const auto map=model+0x1328;
    const auto count=memory.get<int>(map+0x10),capacity=memory.get<int>(map+0x14);
    if(count<0 || capacity<0 || capacity>1024 || count>capacity)
        throw std::runtime_error("HUD addon map size invalid");
    if(!count)return 0;
    if(!capacity || (capacity&(capacity-1)) || !(memory.get<std::uint32_t>(map+4)&0x7fffffffU))
        throw std::runtime_error("HUD addon map storage invalid");
    const auto data=pointer(memory.get<Address>(map+8));
    const auto index=function<int(*)(Address,const char*,std::uint32_t,Address)>(binding::find_arm_attachment)(map,name,hash,0);
    if(index==-1)return 0;
    if(index<0 || index>=capacity)throw std::runtime_error("HUD addon map index invalid");
    return data+static_cast<Address>(index)*16;
}
std::uint32_t addon_handle(Address model,const char* name,std::uint32_t hash) {
    const auto entry=addon_entry(model,name,hash);
    return entry?memory.get<std::uint32_t>(entry+12):0xffffffffU;
}
void invalidate_hud_part(Address hud,const char* name,std::uint32_t hash) {
    const auto entry=addon_entry(hud,name,hash);if(!entry)return;
    const auto handle=memory.get<std::uint32_t>(entry+12);
    if(handle==0xffffffffU)return;
    const auto part=entity(handle);
    const auto identity=pointer(memory.get<Address>(part+offsets::identity));
    if(memory.get<std::uint32_t>(identity+0x10)!=handle)
        throw std::runtime_error("HUD addon handle serial changed");
    // Same native removal and handle reset as the glove cache invalidation.
    function<void(*)(Address)>(binding::remove_model_entity)(part);
    memory.put(entry+12,std::uint32_t{0xffffffffU});
}
void model_parts(const WeaponState& s,Address hud,bool changed=false) {
    same(s);
    if(hud_weapon(s)!=hud || memory.get<Address>(hud)!=client+binding::hud_weapon_vtable ||
        entity(memory.get<std::uint32_t>(hud+offsets::owner))!=s.weapon)
        throw std::runtime_error("Attachment HUD type/owner changed");
    if(changed) {
        invalidate_hud_part(hud,"stattrak",0xc8d7255e);
        invalidate_hud_part(hud,"nametag",0xa6ebe9b9);
        invalidate_hud_part(hud,"keychain",0x0aa5272a);
    }
    function<void(*)(Address,Address)>(binding::update_stattrak_model)(s.weapon,s.item);
    function<void(*)(Address,Address)>(binding::update_nametag_model)(s.weapon,s.item);
    // These take the HUD weapon only and obtain the world addon through its
    // weapon owner. Calling the world builders on hud was the wrong path.
    function<void(*)(Address)>(binding::update_hud_nametag)(hud);
    function<void(*)(Address)>(binding::update_hud_stattrak)(hud);
    function<void(*)(Address,Address,bool)>(binding::update_charm_model)(s.weapon,s.item,false);
    function<void(*)(Address)>(binding::update_hud_charm)(hud);
    InterlockedIncrement(&box->part_updates);
    const auto stat=addon_handle(hud,"stattrak",0xc8d7255e);
    const auto name=addon_handle(hud,"nametag",0xa6ebe9b9);
    std::snprintf(box->part_message,sizeof(box->part_message),
        "HUD addon handles: stattrak=%u nametag=%u charm=%u; visual check required",stat,name,addon_handle(hud,"keychain",0x0aa5272a));
}
bool try_model_parts(const WeaponState& s,Address hud,bool changed=false) {
    try {
        model_parts(s,hud,changed);
        const auto choice=selections.get(s.team,s.definition);
        if(!choice)return true;
        return (choice->stattrak<0 || addon_handle(hud,"stattrak",0xc8d7255e)!=0xffffffffU) &&
            (choice->custom_name.empty() || addon_handle(hud,"nametag",0xa6ebe9b9)!=0xffffffffU) &&
            (!choice->decorations.charm || addon_handle(hud,"keychain",0x0aa5272a)!=0xffffffffU);
    }
    catch(const std::exception& e) {
        std::snprintf(box->part_message,sizeof(box->part_message),"Attachment: %s",e.what());
    }
    return false;
}
void regenerate(const WeaponState& s, Address hud, const Mesh& wm, const Mesh& hm,
                std::uint64_t world_mask, std::uint64_t hud_mask) {
    same(s);
    if (hud_weapon(s) != hud || mesh(s.weapon).node != wm.node || mesh(hud).node != hm.node)
        throw std::runtime_error("HUD/model islem sirasinda degisti.");
    set_mesh(wm, world_mask); set_mesh(hm, hud_mask);
    auto clear = function<void(*)(Address,bool)>(binding::clear_materials);
    clear(s.weapon + 0x610, true);
    function<void(*)(Address,bool)>(binding::update_weapon)(s.weapon, false);
    clear(hud + 0x610, true);
    // Use the same item for first-person and world models. No global function patch.
    auto flag = memory.get<std::uint8_t>(s.weapon + 0x1b20) != 0;
    function<void(*)(Address,Address,bool,bool,Address)>(binding::update_model)(hud, s.item, false, flag, 0);
}
void sample(const WeaponState& s, Address hud) {
    auto v=effective(s.item);
    box->weapon=s.weapon; box->hud=hud;
    box->effective_paint=v.paint; box->effective_seed=v.seed; box->effective_wear=v.wear;
    box->world_materials=memory.get<int>(s.weapon+0xab0)+memory.get<int>(s.weapon+0xac8);
    box->hud_materials=memory.get<int>(hud+0xab0)+memory.get<int>(hud+0xac8);
}
std::size_t find_saved(const WeaponState& s) {
    for(std::size_t i=0;i<saved.size();++i) if(saved[i].state.same_object(s)) return i;
    return saved.size();
}
std::string model_name(Address item) {
    auto address=function<Address(*)(Address)>(binding::model_path)(item);
    if(address<0x10000 || address>0x00007ffffffeffffULL) throw std::runtime_error("Motor model yolu bos.");
    std::string result;
    for(unsigned i=0;i<512;++i) { auto c=memory.get<char>(address+i); if(!c)break; result+=c; }
    if(result.size()<6 || result.size()>=512 || result.find("knife")==std::string::npos || result.substr(result.size()-5)!=".vmdl")
        throw std::runtime_error("Motor bicak model yolunu dogrulayamadi.");
    return result;
}
WeaponState change_knife(const WeaponState& s,int definition,std::uint32_t subclass,int quality) {
    same(s);
    auto hud=hud_weapon(s);
    memory.put(s.item+offsets::definition,static_cast<std::uint16_t>(definition));
    memory.put(s.item+0x1bc,quality);
    memory.put(s.weapon+0x380,subclass);
    function<void(*)(Address)>(binding::update_subclass)(s.weapon);
    auto path=model_name(s.item);
    auto set=function<void(*)(Address,const char*)>(binding::set_model);
    set(s.weapon,path.c_str()); set(hud,path.c_str());
    auto after=game->snapshot();
    if(!s.same_object(after) || after.definition!=definition) throw std::runtime_error("Bicak kimligi model guncellemesinde degisti.");
    return after;
}
AppearanceKey appearance(const WeaponState& s) {
    auto hud=hud_weapon(s);auto wm=mesh(s.weapon),hm=mesh(hud);
    return {s.pawn,s.weapon,s.identity,hud,wm.node,hm.node,
        memory.get<Address>(wm.node+0x140+0xa0),memory.get<Address>(hm.node+0x140+0xa0),s.handle,s.team,s.definition};
}
void apply(const Cosmetic& c,bool report=true) {
    validate(c);
    auto finish=catalog::finish(c.definition,c.paint);
    if(!finish || c.wear<finish->minimum || c.wear>finish->maximum)
        throw std::runtime_error("Katalog/float eslesmesi gecersiz.");
    const bool knife=c.category=="knife";
    auto s=game->snapshot();
    if(!game->owned_weapon(s))throw std::runtime_error("Picked-up weapon: original appearance preserved.");
    if(s.team!=(c.team=="T"?2:3) || !catalog::compatible(s.definition,c.definition))
        throw std::runtime_error("Aktif esya/takim secimle uyusmuyor.");
    auto hud=hud_weapon(s);auto wm=mesh(s.weapon),hm=mesh(hud);auto before=effective(s.item);
    auto index=find_saved(s);
    if(index==saved.size()) {
        if(saved.size()>=256) throw std::runtime_error("Yedek siniri; CS2'yi yeniden baslat.");
        Saved entry{s,before,before,wm,hm,false,memory.get<std::uint32_t>(s.weapon+0x380),memory.get<int>(s.item+0x1bc),memory.get<int>(s.weapon+0x18b4)};
        for(unsigned n=0;n<3;++n)entry.names[n]=memory.get<ItemLabel>(s.item+0x399+n*161);
        entry.stat_attributes={stat_attribute(s.item,80),stat_attribute(s.item,81)};
        entry.decorations=read_decorations(s.item);
        saved.push_back(entry);
    } else if(saved[index].applied && !equal(before,saved[index].last)) {
        // Network/equip reset: rebase original appearance, then restore the user's selection.
        saved[index].original=before;
    }
    auto& backup=saved[index];
    // Re-equipping an unchanged appearance does not invalidate the HUD row.
    // Changed definitions/labels/finishes and explicit compatibility mode do.
    const bool refresh_hud= backup.last_decorations!=c.decorations || !backup.applied || s.definition!=c.definition || backup.last_definition!=c.definition ||
        !equal(backup.last,{c.paint,c.seed,c.wear}) || backup.last_stattrak!=c.stattrak ||
        c.custom_name!=backup.last_name.data() || InterlockedCompareExchange(&box->full_hud_refresh,0,0)!=0;
    const bool commit_selection=backup.refresh_requested || refresh_hud;
    game->apply(c); backup.applied=false;
    if(knife) s=change_knife(s,c.definition,subclass_hash(c.definition),3);
    function<void(*)(Address,const char*,int)>(binding::set_custom_name)(s.item,c.custom_name.c_str(),0);
    backup.last_name={};std::copy(c.custom_name.begin(),c.custom_name.end(),backup.last_name.begin());
    stat_attributes(s.item,c.stattrak<0?StatAttributes{}:StatAttributes{static_cast<std::uint32_t>(c.stattrak),0U});
    memory.put(s.weapon+0x18b4,c.stattrak);
    memory.put(s.item+0x1bc,c.stattrak>=0?9:(knife?3:backup.quality));
    backup.last_stattrak=c.stattrak;
    write_decorations(s.item,decoration_values(c.decorations));
    attributes(s.item,{c.paint,c.seed,c.wear});
    auto after=effective(s.item);
    if(!equal(after,{c.paint,c.seed,c.wear})) throw std::runtime_error("Motor nitelik geri okumasi eslesmedi; Geri al'i kullan.");
    backup.last=after;backup.last_definition=c.definition;
    hud=hud_weapon(s);wm=mesh(s.weapon);hm=mesh(hud);
    const std::uint64_t mask=finish->legacy ? 2 : 1;
    regenerate(s,hud,wm,hm,mask,mask);
    backup.applied=true;
    if(commit_selection)backup.material_commit.arm(GetTickCount64());
    backup.parts_until=GetTickCount64()+5000;
    if(refresh_hud && std::find(dirty_hud.begin(),dirty_hud.end(),s.handle)==dirty_hud.end())dirty_hud.push_back(s.handle);
    if(report) sample(s,hud);
    if(try_model_parts(s,hud,true))backup.parts_until=0;
    backup.parts_at=GetTickCount64()+250;backup.verify_at=0;
    backup.memo.remember(appearance(s));
    backup.last_decorations=c.decorations;backup.refresh_requested=false;
}
void restore(bool clear_choice=true) {
    auto s=game->snapshot();auto i=find_saved(s);
    if(clear_choice){selections.clear(s.team,s.definition);}
    retention.invalidate();
    if(i==saved.size()) throw std::runtime_error("Otomatik secim kapatildi; aktif esya icin yedek yok.");
    // Resolve render dependencies before mutating or discarding fallback backups.
    // During equip/intro there may be no HUD child yet; leave the restore intact.
    (void)mesh(hud_weapon(s));(void)mesh(s.weapon);
    auto b=saved[i];
    write_decorations(s.item,b.decorations);
    stat_attributes(s.item,b.stat_attributes);
    memory.put(s.weapon+0x18b4,b.stattrak);memory.put(s.item+0x1bc,b.quality);
    for(unsigned n=0;n<3;++n)memory.put(s.item+0x399+n*161,b.names[n]);
    attributes(s.item,b.original);
    if(game->has_active_backup())game->restore();
    if(catalog::knife(s.definition)) s=change_knife(s,b.state.definition,b.subclass,b.quality);
    auto hud=hud_weapon(s);auto wm=mesh(s.weapon),hm=mesh(hud);
    regenerate(s,hud,wm,hm,b.world.mask,b.hud.mask);sample(s,hud);
    if(std::find(dirty_hud.begin(),dirty_hud.end(),s.handle)==dirty_hud.end())dirty_hud.push_back(s.handle);
    try_model_parts(s,hud,true);
    saved.erase(saved.begin()+i);
}
#include "gloves.inl"
#include "voices.inl"
#include "agents.inl"
#include "team_preview.inl"

bool rollback_gloves{},rollback_agent{};
std::uint64_t next_rollback{};
void settle_restore_all() {
    if(!rollback_gloves&&!rollback_agent&&std::none_of(saved.begin(),saved.end(),[](const auto& b){return b.rollback_requested;}))return;
    const auto now=GetTickCount64();if(now<next_rollback)return;next_rollback=now+500;
    try {auto s=game->snapshot();auto i=find_saved(s);if(i<saved.size()&&saved[i].rollback_requested)restore(false);}
    catch(const std::exception& e){std::snprintf(box->auto_message,sizeof(box->auto_message),"Restore pending: %s",e.what());}
    if(rollback_gloves)try {auto s=game->glove_snapshot();if(find_glove(s)<glove_saved.size())restore_gloves(false);rollback_gloves=false;}
    catch(const std::exception& e){std::snprintf(box->glove_message,sizeof(box->glove_message),"Restore pending: %s",e.what());}
    if(rollback_agent)try {auto s=game->glove_snapshot();if(agent_saved&&current_agent(*agent_saved,s))restore_agent(false);else {agent_saved.reset();InterlockedExchange(&box->agent_backup,0);}rollback_agent=false;}
    catch(const std::exception& e){std::snprintf(box->agent_message,sizeof(box->agent_message),"Restore pending: %s",e.what());}
}

void settle_parts(Saved& entry,const WeaponState& s,Address hud,std::uint64_t now) {
    if(now>=entry.parts_until || now<entry.parts_at)return;
    entry.parts_at=now+250;
    if(try_model_parts(s,hud))entry.parts_until=0;
}
void watch() {
    const auto now=GetTickCount64();if(now<next_watch || now<knife_ready_at)return;next_watch=now+100;
    WeaponState s;
    try { s=game->snapshot(); }
    catch(...) { retention.invalidate();return; }
    if(!game->owned_weapon(s)) {
        retention.invalidate();
        std::snprintf(box->auto_message,sizeof(box->auto_message),"Picked-up or unverified weapon: original appearance preserved.");return;
    }
    if(!catalog::firearm(s.definition) && !catalog::knife(s.definition)) { retention.invalidate();return; }
    const auto pending=find_saved(s);if(pending<saved.size()&&saved[pending].rollback_requested)return;
    auto choice=selections.get(s.team,s.definition);
    if(!choice) { retention.invalidate();return; }
    AppearanceKey key;
    try { key=appearance(s); } catch(...) { return; }
    try {
        auto cached=find_saved(s);
        auto f=catalog::finish(choice->definition,choice->paint);
        if(cached<saved.size() && saved[cached].applied && !saved[cached].refresh_requested &&
           saved[cached].memo.reusable(key,true,false) && now<saved[cached].verify_at) {
            if(saved[cached].material_commit.due(now)) {
                const auto mask=f->legacy?2U:1U;
                regenerate(s,key.hud,mesh(s.weapon),mesh(key.hud),mask,mask);sample(s,key.hud);
            }
            settle_parts(saved[cached],s,key.hud,now);
            InterlockedIncrement(&box->watch_checks_skipped);return;
        }
        auto expected=Values{choice->paint,choice->seed,choice->wear};
        const auto mask=f->legacy ? 2U : 1U;
        StatAttributes stat{};
        const auto decorations=read_decorations(s.item,&stat);
        bool match=equal(effective(s.item),expected) && s.definition==choice->definition &&
            mesh(s.weapon).mask==mask && mesh(key.hud).mask==mask &&
            memory.get<int>(s.weapon+0x18b4)==choice->stattrak &&
            stat.count==(choice->stattrak<0?std::optional<std::uint32_t>{}:std::optional<std::uint32_t>{static_cast<std::uint32_t>(choice->stattrak)}) &&
            stat.type==(choice->stattrak<0?std::optional<std::uint32_t>{}:std::optional<std::uint32_t>{0});
        auto actual_name=memory.get<ItemLabel>(s.item+0x399);actual_name.back()=0;
        match=match && choice->custom_name==actual_name.data() && decorations==decoration_values(choice->decorations);
        const auto fallback=game->values(s);
        const bool fallback_match=fallback.paint==choice->paint && fallback.seed==choice->seed &&
            std::abs(fallback.wear-choice->wear)<0.00001f && fallback.item_id_high==0xffffffffU;
        // Attribute readback alone does not prove that the renderer consumed a new choice.
        match=match && fallback_match && cached<saved.size() && !saved[cached].refresh_requested &&
            saved[cached].last_decorations==choice->decorations;
        if(cached<saved.size() && saved[cached].applied && saved[cached].memo.reusable(key,match && fallback_match,false)) {
            saved[cached].verify_at=now+750;
            if(saved[cached].material_commit.due(now)) {
                // Current owned item only; no HUD row rebuild or selection edit.
                regenerate(s,key.hud,mesh(s.weapon),mesh(key.hud),mask,mask);
                sample(s,key.hud);
            }
            retention.applied(key,now);
            settle_parts(saved[cached],s,key.hud,now);
            InterlockedIncrement(&box->material_reuses);return;
        }
        if(!retention.needs(key,match,now)) {
            auto i=find_saved(s);
            if(i<saved.size() && saved[i].applied)settle_parts(saved[i],s,key.hud,now);
            return;
        }
        auto original_index=find_saved(s);
        if(catalog::knife(choice->definition) && s.definition!=choice->definition && original_index<saved.size() &&
            saved[original_index].state.definition!=s.definition) {
            auto pending=*choice;restore();selections.set(pending);
            knife_ready_at=now+150;return;
        }
        apply(*choice,false);
        retention.applied(appearance(game->snapshot()),now);
        InterlockedIncrement(&box->auto_updates);
        std::snprintf(box->auto_message,sizeof(box->auto_message),"Secim yeniden uygulandi; aktif handle=%u",s.handle);
    } catch(const std::exception& e) {
        retention.failed(now);
        std::snprintf(box->auto_message,sizeof(box->auto_message),"Otomatik yenileme: %s",e.what());
    }
    InterlockedExchange(&box->pending_backups,static_cast<LONG>(saved.size()+glove_saved.size()));
}
void execute() {
    box->success=0; box->message[0]=0;
    box->weapon=0; box->hud=0; box->effective_paint=-1; box->effective_seed=-1;
    box->effective_wear=-1; box->world_materials=-1; box->hud_materials=-1;
    try {
        if(GetTickCount64()>box->deadline) throw std::runtime_error("Istek suresi doldu; oyuna uygulanmadi.");
        if(box->operation==bridge::RestoreAll) {
            selections.clear();voice_choice.store(0,std::memory_order_release);
            retention.invalidate();glove_retention.invalidate();glove_refresh.reset();
            for(auto& b:saved)b.rollback_requested=true;
            rollback_gloves=!glove_saved.empty();rollback_agent=agent_saved.has_value();next_rollback=0;
            settle_restore_all();
            box->success=1;
            std::snprintf(box->message,sizeof(box->message),"All selections cleared. Items restore when ready; holstered weapons restore on equip. Saved profile files unchanged.");
            box->completed_id=box->request_id;InterlockedExchange(&box->state,bridge::Done);return;
        }
        if(box->operation==bridge::Stop) {
            selections.clear();voice_choice.store(0,std::memory_order_release);
            glove_refresh.reset();glove_retention.invalidate();
            retention.invalidate();
            box->success=1;std::snprintf(box->message,sizeof(box->message),"Otomatik uygulama durduruldu.");
            box->completed_id=box->request_id;InterlockedExchange(&box->state,bridge::Done);return;
        }
        if(box->operation==bridge::ClearBoth) {
            if(!shared_item(box->definition))throw std::runtime_error("Both requires a shared item");
            selections.clear(2,box->definition);selections.clear(3,box->definition);
            retention.invalidate();glove_retention.invalidate();
            box->success=1;std::snprintf(box->message,sizeof(box->message),"Both: automatic selection cleared for T and CT.");
            box->completed_id=box->request_id;InterlockedExchange(&box->state,bridge::Done);return;
        }
        if(box->operation==bridge::QueueProfile) {
            const auto loaded=decode_profile(box->profile);
            agent_failures=0;next_agent_watch=0;
            selections.set_all(loaded);retention.invalidate();glove_retention.invalidate();
            for(auto& entry:saved)entry.refresh_requested=true;
            box->success=1;
            std::snprintf(box->message,sizeof(box->message),"Profil etkin: %u secim; takim ve esya uygun oldugunda uygulanir.",static_cast<unsigned>(loaded.size()));
            box->completed_id=box->request_id;InterlockedExchange(&box->state,bridge::Done);return;
        }
        if(box->operation==bridge::Queue || box->operation==bridge::QueueBoth) {
            if(box->team!=2 && box->team!=3)throw std::runtime_error("Takim secimi gecersiz.");
            Cosmetic c{box->team==2?"T":"CT",catalog::category(box->definition),
                static_cast<int>(box->definition),box->paint,box->seed,box->wear,box->stattrak,{}};
            box->custom_name[160]=0;c.custom_name=box->custom_name;c.decorations=box->decorations;
            auto finish=catalog::finish(c.definition,c.paint);validate(c);
            if(!finish || c.wear<finish->minimum || c.wear>finish->maximum)
                throw std::runtime_error("Esya/kaplama/float secimi gecersiz.");
            agent_failures=0;next_agent_watch=0;
            const bool both=box->operation==bridge::QueueBoth;
            selections.set_all(selection_targets(c,both));retention.invalidate();
            if(catalog::glove(c.definition))glove_retention.invalidate();
            for(auto& entry:saved)if((both || entry.state.team==(c.team=="T"?2:3)) &&
                catalog::compatible(entry.state.definition,c.definition))entry.refresh_requested=true;
            box->success=1;std::snprintf(box->message,sizeof(box->message),"Secim kaydedildi; uygun esya elde oldugunda otomatik uygulanacak.");
            box->completed_id=box->request_id;InterlockedExchange(&box->state,bridge::Done);return;
        }
        if(box->operation==bridge::RestoreAgent) {
            restore_agent();box->success=1;
            std::snprintf(box->message,sizeof(box->message),"Ajan gorunumu geri alindi.");
            box->completed_id=box->request_id;InterlockedExchange(&box->state,bridge::Done);return;
        }
        const bool glove_request=box->operation==bridge::RestoreGloves ||
            (box->operation==bridge::Apply && catalog::glove(box->definition));
        auto expected=glove_request?game->glove_snapshot():game->snapshot();
        if(expected.pawn!=box->expected_pawn || expected.identity!=box->expected_identity || expected.handle!=box->expected_handle)
            throw std::runtime_error("Istek beklerken aktif silah degisti; uygulanmadi.");
        if(box->operation==bridge::Apply) {
            Cosmetic c{box->team==2 ? "T" : "CT",catalog::category(box->definition),
                static_cast<int>(box->definition),box->paint,box->seed,box->wear,box->stattrak,{}};
            box->custom_name[160]=0;c.custom_name=box->custom_name;c.decorations=box->decorations;
            const auto previous=find_saved(expected);
            if(c.category=="knife" && expected.definition!=c.definition && previous<saved.size() &&
                saved[previous].state.definition!=expected.definition) {
                // Match the user-verified restore -> equip sequence, across updates.
                // restore() disables the old automatic choice before switching.
                restore();
                transition_at=GetTickCount64()+150;
                std::snprintf(box->message,sizeof(box->message),"Bicak modeli sifirlandi; yeni model icin guncelleme bekleniyor.");
                return; // Keep request Running until a subsequent frame completes it.
            }
            if(glove_request)apply_gloves(c);else apply(c);
            selections.set(c);
            if(glove_request)glove_retention.invalidate();
            else { retention.invalidate();retention.applied(appearance(game->snapshot()),GetTickCount64()); }
        }
        else if(box->operation==bridge::Restore) restore();
        else if(box->operation==bridge::RestoreGloves) restore_gloves();
        else throw std::runtime_error("Bilinmeyen kopru istegi.");
        box->success=1;
        std::snprintf(box->message,sizeof(box->message),"%s",
            box->operation==bridge::Apply ? (glove_request ? "Eldiven nitelikleri yazildi ve motorun eldiven yenilemesi istendi. Gorsel sonuc kontrol edilmeli." : "Nitelikler ve yerel gorunum adi yazildi, dunya/HUD yenilemesi cagrildi. Gorsel sonuc kontrol edilmeli.") :
            box->operation==bridge::RestoreGloves ? "Eldiven yedegi geri uygulandi ve otomatik eldiven secimi kapatildi." :
            "Onceki gorunum degerleri ve mesh maskeleri geri uygulandi. Eklenen nitelik kayitlari varsayilan degerleriyle kalabilir.");
    } catch(const std::exception& e) {
        std::snprintf(box->message,sizeof(box->message),"%s",e.what());
    } catch(...) { std::snprintf(box->message,sizeof(box->message),"Beklenmeyen C++ hatasi."); }
    box->completed_id=box->request_id;
    InterlockedExchange(&box->pending_backups,static_cast<LONG>(saved.size()+glove_saved.size()));
    InterlockedExchange(&box->state,bridge::Done);
}
bool expired_weapon(const WeaponState& state) {
    try {
        const auto resolved=function<Address(*)(Address)>(binding::resolve_handle)(reinterpret_cast<Address>(&state.handle));
        return !resolved || resolved!=state.weapon || memory.get<Address>(resolved+offsets::identity)!=state.identity ||
            memory.get<std::uint32_t>(state.identity+0x10)!=state.handle;
    } catch(...) {return false;}
}
void prune_weapons() {
    const auto previous=saved.size();
    saved.erase(std::remove_if(saved.begin(),saved.end(),[](const Saved& s){return expired_weapon(s.state);}),saved.end());
    game->prune_expired(expired_weapon);
    InterlockedExchangeAdd(&box->weapon_backups_pruned,static_cast<LONG>(previous-saved.size()));
    InterlockedExchange(&box->pending_backups,static_cast<LONG>(saved.size()+glove_saved.size()));
}
void frame(void* self,int stage) {
    original(self,stage);
    if(!box) return;
    InterlockedIncrement(&box->heartbeat);
    // Verified supplied Hibiki client: stage 5 starts FrameNetUpdate; 6/7 bracket
    // FramePostDataUpdate; 8 closes the enclosing network update.
    if(stage!=8 || InterlockedCompareExchange(&callback_lock,1,0)!=0) return;
    WorkTimer timer(box->frame_work_us,box->frame_work_calls,box->frame_max_us);
    if(!frame_thread) frame_thread=GetCurrentThreadId();
    box->stage=static_cast<DWORD>(stage); box->frame_thread=GetCurrentThreadId();
    if(frame_thread==GetCurrentThreadId()) {
        if(InterlockedCompareExchange(&box->state,bridge::Running,bridge::Queued)==bridge::Queued) execute();
        if(transition_at && GetTickCount64()>=*transition_at) {
            transition_at.reset();execute();
        }
        if(!transition_at) { settle_restore_all();if(!rollback_agent)watch_agent();watch();if(!rollback_gloves)watch_gloves();settle_gloves(); }
        static std::uint64_t next_prune{};
        if(GetTickCount64()>=next_prune){next_prune=GetTickCount64()+2000;prune_weapons();}
        static std::uint64_t next_status{};
        const auto now=GetTickCount64();
        if(now>=next_status) {
            next_status=now+250;
            bool active=false;
            try { if(!saved.empty()) active=find_saved(game->snapshot())!=saved.size(); } catch(...) {}
            InterlockedExchange(&box->active_backup,active ? 1 : 0);
            bool gloves=false;
            try { if(!glove_saved.empty())gloves=find_glove(game->glove_snapshot())<glove_saved.size(); } catch(...) {}
            InterlockedExchange(&box->glove_backup,gloves?1:0);
        }
    }
    InterlockedExchange(&callback_lock,0);
}
#include "game_language.inl"
#include "hud.inl"
#include "killfeed.inl"
DWORD WINAPI start(void*) {
    auto mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,nullptr,PAGE_READWRITE,0,sizeof(bridge::Mailbox),bridge::name(GetCurrentProcessId()).c_str());
    if(!mapping) return 1;
    if(GetLastError()==ERROR_ALREADY_EXISTS) { CloseHandle(mapping); return 2; }
    box=static_cast<bridge::Mailbox*>(MapViewOfFile(mapping,FILE_MAP_ALL_ACCESS,0,0,sizeof(bridge::Mailbox)));
    if(!box) { CloseHandle(mapping); return 3; }
    // Mapping and module intentionally live until process exit: no dangling callback on unload.
    box->protocol=bridge::version; box->pid=GetCurrentProcessId();
    box->startup_stage=1;MemoryBarrier();box->magic=bridge::magic;
    try {
        client=reinterpret_cast<Address>(GetModuleHandleW(L"client.dll"));
        engine=reinterpret_cast<Address>(GetModuleHandleW(L"engine2.dll"));
        InterlockedExchange(&box->startup_stage,2);
        const auto validated_build=verified_patch_version(memory,client,engine);
        game=std::make_unique<Game>(memory,client,engine,validated_build);
        HMODULE pinned{};
        if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
            reinterpret_cast<LPCWSTR>(&start),&pinned)) throw std::runtime_error("Kopru modulu sabitlenemedi.");
        InterlockedExchange(&box->startup_stage,3);
        // Verify the registry resolves the exact primary CSource2Client vtable.
        using Factory=void*(*)(const char*,int*);
        auto symbol=GetProcAddress(reinterpret_cast<HMODULE>(client),"CreateInterface");
        Factory factory{}; static_assert(sizeof(factory)==sizeof(symbol));
        std::memcpy(&factory,&symbol,sizeof(factory));
        if(!factory) throw std::runtime_error("CreateInterface bulunamadi.");
        auto object=factory("Source2Client002",nullptr);
        if(!object || memory.get<Address>(reinterpret_cast<Address>(object))!=client+binding::client_vtable)
            throw std::runtime_error("Source2Client002 vtable eslesmedi.");
        if(memory.get<Address>(client+binding::hud_vtable+6*8)!=client+binding::hud_think.rva)
            throw std::runtime_error("HUD guncelleme tablosu eslesmedi.");
        InterlockedExchange(&box->startup_stage,4);
        auto slot=reinterpret_cast<void**>(client+binding::client_vtable+36*8);
        original=reinterpret_cast<FrameFn>(client+binding::frame_notify.rva);
        DWORD protection{};
        if(!VirtualProtect(slot,sizeof(void*),PAGE_READWRITE,&protection)) throw std::runtime_error("Guncelleme geri cagrisi baglanamadi.");
        auto previous=InterlockedCompareExchangePointer(slot,reinterpret_cast<void*>(&frame),reinterpret_cast<void*>(original));
        DWORD ignored{}; const bool protected_again=VirtualProtect(slot,sizeof(void*),protection,&ignored)!=FALSE;
        if(previous!=reinterpret_cast<void*>(original)) throw std::runtime_error("Geri cagri baska bir modul tarafindan degistirilmis.");
        if(!protected_again) {
            // Keep the now-live callback/module alive, but reject all requests.
            std::snprintf(box->message,sizeof(box->message),"Vtable sayfa korumasi geri yuklenemedi; CS2'yi yeniden baslat.");
            InterlockedExchange(&box->initialized,-1); return 4;
        }
        auto hud_slot=reinterpret_cast<void**>(client+binding::hud_vtable+6*8);
        hud_original=reinterpret_cast<HudThink>(client+binding::hud_think.rva);
        if(!VirtualProtect(hud_slot,sizeof(void*),PAGE_READWRITE,&protection))throw std::runtime_error("HUD geri cagrisi acilamadi.");
        previous=InterlockedCompareExchangePointer(hud_slot,reinterpret_cast<void*>(&hud_tick),reinterpret_cast<void*>(hud_original));
        const bool hud_protected=VirtualProtect(hud_slot,sizeof(void*),protection,&ignored)!=FALSE;
        if(previous!=reinterpret_cast<void*>(hud_original) || !hud_protected)throw std::runtime_error("HUD geri cagrisi baglanamadi.");
        InterlockedExchange(&box->startup_stage,5);
        try {install_voices();} catch(const std::exception& e) {std::snprintf(box->voice_message,sizeof(box->voice_message),"%s",e.what());}
        InterlockedExchange(&box->startup_stage,6);
        try {install_death_notice();} catch(const std::exception& e) {std::snprintf(box->death_message,sizeof(box->death_message),"%s",e.what());}
        std::snprintf(box->message,sizeof(box->message),"Oyun ici kopru hazir; frame-stage 8 bekleniyor.");
        try {install_team_preview();}catch(const std::exception& e){std::snprintf(box->preview_message,sizeof(box->preview_message),"%s",e.what());}
        InterlockedExchange(&box->startup_stage,7);
        InterlockedExchange(&box->initialized,1);
    } catch(const std::exception& e) {
        std::snprintf(box->message,sizeof(box->message),"%s",e.what());
        InterlockedExchange(&box->initialized,-1);
    }
    return 0;
}
}
BOOL WINAPI DllMain(HINSTANCE module,DWORD reason,LPVOID) {
    if(reason==DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(module);
        auto worker=CreateThread(nullptr,0,start,nullptr,0,nullptr);
        if(!worker) return FALSE;
        CloseHandle(worker);
    }
    return TRUE;
}
