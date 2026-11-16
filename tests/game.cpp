#include "game.hpp"
#include "module_contract.hpp"
#include "stat_attributes.hpp"
#include <map>
#include <functional>
#include <iostream>

using namespace masiva;
constexpr int fixture_build=14192;
constexpr Address client = 0x10000000, engine = 0x20000000, pawn = 0x30000000;
constexpr Address services = 0x31000000, weapon = 0x32000000, list = 0x40000000, chunk = 0x41000000;
constexpr Address controller=0x34000000;
constexpr std::uint64_t local_xuid=0x110000100123456ULL;
constexpr Address slot = chunk + offsets::entity_stride * 100;
constexpr Address item = weapon + offsets::attribute_manager + offsets::item;
void require(bool ok, const char* name) { if (!ok) throw std::runtime_error(name); }
class FakeMemory : public Memory {
public:
    std::map<Address, std::uint8_t> bytes;
    bool running = true;
    int writes = 0, fail_write = -1;
    std::function<void()> after_write;
    bool alive() const override { return running; }
    void read_bytes(Address at, void* out, std::size_t size) const override {
        if (!running) throw std::runtime_error("process exited");
        auto p = static_cast<std::uint8_t*>(out);
        for (std::size_t i = 0; i < size; ++i) {
            auto it = bytes.find(at + i);
            if (it == bytes.end()) throw std::runtime_error("unmapped memory");
            p[i] = it->second;
        }
    }
    void write_bytes(Address at, const void* data, std::size_t size) override {
        ++writes;
        if (writes == fail_write) throw std::runtime_error("injected write failure");
        raw(at, data, size);
        if (after_write) after_write();
    }
    void raw(Address at, const void* data, std::size_t size) {
        auto p = static_cast<const std::uint8_t*>(data);
        for (std::size_t i = 0; i < size; ++i) bytes[at + i] = p[i];
    }
    template<class T> void seed(Address at, T v) { raw(at, &v, sizeof(v)); }
    FakeMemory() {
        seed(engine + offsets::build_number, fixture_build);
        seed(client + offsets::local_pawn, pawn);
        seed(pawn + offsets::health, 100);
        seed(pawn+offsets::pawn_controller,std::uint32_t{2});
        seed(chunk+offsets::entity_stride*2,controller);
        seed(controller+offsets::controller_pawn,std::uint32_t{1});
        seed(controller+offsets::controller_steam_id,local_xuid);
        seed(weapon+offsets::original_owner,local_xuid);
        seed(pawn + offsets::team, std::uint8_t{2});
        seed(pawn + offsets::life_state, std::uint8_t{0});
        seed(pawn + offsets::weapon_services, services);
        seed(services + offsets::active_weapon, std::uint32_t{100});
        seed(client + offsets::entity_list, list);
        seed(list + 0x10, chunk);
        seed(slot, weapon);
        seed(chunk + offsets::entity_stride, pawn);
        seed(weapon + offsets::identity, slot);
        seed(weapon + offsets::owner, std::uint32_t{1});
        seed(item + offsets::definition, std::uint16_t{7});
        seed(weapon + offsets::paint, std::int32_t{12});
        seed(weapon + offsets::seed, std::int32_t{7});
        seed(weapon + offsets::wear, 0.3f);
        seed(item + offsets::item_id_high, std::uint32_t{123});
    }
};
const Cosmetic ak{"T", "weapon", 7, 474, 42, 0.01f};
template<class F> void rejects(F f, const char* name) {
    bool rejected = false;
    try { f(); } catch (const std::runtime_error&) { rejected = true; }
    require(rejected, name);
}
int main() {
    int checks = 0;
    auto test = [&](const char* name, const std::function<void()>& f) {
        f(); ++checks; std::cout << "PASS " << name << '\n';
    };
    test("exact module pair required before accepting any runtime patch version", [] {
        FakeMemory m;
        const auto seed_identity=[&](Address base,std::uint32_t stamp,std::uint32_t size){
            m.seed(base,std::uint16_t{0x5a4d});m.seed(base+0x3c,std::uint32_t{0x100});
            m.seed(base+0x100,std::uint32_t{0x4550});m.seed(base+0x104,std::uint16_t{0x8664});
            m.seed(base+0x108,stamp);m.seed(base+0x118,std::uint16_t{0x20b});m.seed(base+0x150,size);
        };
        seed_identity(client,binding::timestamp,binding::image_size);
        seed_identity(engine,engine_binding::timestamp,engine_binding::image_size);
        for(const auto& c:binding::required)m.raw(client+c.rva,c.bytes.data(),c.bytes.size());
        for(const auto& c:engine_binding::required)m.raw(engine+c.rva,c.bytes.data(),c.bytes.size());
        require(verified_patch_version(m,client,engine)==fixture_build,"matching module pair accepted");
        m.seed(engine+offsets::build_number,fixture_build+1);
        const auto accepted=verified_patch_version(m,client,engine);Game g(m,client,engine,accepted);
        require(g.player_snapshot().health==100,"validated numeric patch version pinned");
        m.seed(engine+offsets::build_number,fixture_build+2);
        rejects([&]{g.player_snapshot();},"mid-session build change rejected");
        m.seed(engine+0x108,engine_binding::timestamp+1);
        rejects([&]{verified_patch_version(m,client,engine);},"wrong engine identity");
        seed_identity(engine,engine_binding::timestamp,engine_binding::image_size);
        m.seed(client+binding::required[0].rva,std::uint8_t{0});
        rejects([&]{verified_patch_version(m,client,engine);},"wrong client instructions");
        require(m.writes==0,"validation has zero game writes");
    });
    test("StatTrak enable/change/disable/restore preserves unrelated attributes", [] {
        FakeMemory m;constexpr Address data=0x51000000;
        std::map<std::uint16_t,std::uint32_t> attrs{{6,12345},{80,42},{81,7}};
        const auto publish=[&] {
            m.seed(item+0x210,static_cast<int>(attrs.size()));m.seed(item+0x218,data);
            unsigned i=0;
            for(const auto& [id,value]:attrs) {
                const auto entry=data+i++*0x48;
                m.seed(entry+0x30,id);m.seed(entry+0x34,value);m.seed(entry+0x40,std::uint8_t{0});
            }
        };
        publish();const StatAttributes original{read_stat_attribute(m,item,80),read_stat_attribute(m,item,81)};
        int calls=0;
        const auto set=[&](Address object,const char* name,float encoded) {
            require(object==item,"setter item identity");++calls;
            std::uint32_t bits{};std::memcpy(&bits,&encoded,sizeof(bits));
            attrs[std::string(name)=="kill eater"?80:81]=bits;publish();
        };
        const auto remove=[&](Address object,std::uint16_t id) {
            require(object==item && (id==80 || id==81),"remove only counter attributes");
            ++calls;attrs.erase(id);publish();
        };
        write_stat_attributes(m,item,{123U,0U},set,remove);
        require(attrs.at(80)==123 && attrs.at(81)==0,"counter bits passed to native setter");
        const auto before=calls;write_stat_attributes(m,item,{123U,0U},set,remove);
        require(calls==before,"stable attributes do not trigger notifications");
        write_stat_attributes(m,item,{999999U,0U},set,remove);
        require(attrs.at(80)==999999,"counter change");
        write_stat_attributes(m,item,{},set,remove);
        require(attrs.size()==1 && attrs.at(6)==12345,"disable removes presence, not a minus-one value");
        write_stat_attributes(m,item,original,set,remove);
        require(attrs.at(80)==42 && attrs.at(81)==7 && attrs.at(6)==12345,"restore original values without clearing other attributes");
        const auto no_op=[](Address,const char*,float){};
        rejects([&]{write_stat_attributes(m,item,{1U,0U},no_op,remove);},"failed native write rejected");
        m.seed(item+0x210,257);const auto unchanged=calls;
        rejects([&]{write_stat_attributes(m,item,{},set,remove);},"invalid list rejected before mutation");
        require(calls==unchanged,"no setter on invalid list");
    });
    test("holstered knife HUD reads require lifetime and original ownership", [] {
        FakeMemory m;Game g(m,client,engine,fixture_build);
        m.seed(slot+0x10,std::uint32_t{100});m.seed(item+offsets::definition,std::uint16_t{42});
        m.seed(services+offsets::active_weapon,std::uint32_t{0xffffffffU});
        const auto held=g.held_weapon(100);
        require(held.definition==42 && g.owned_weapon(held),"knife HUD resolves before knife is drawn");
        require(m.writes==0,"HUD query never changes holstered model");
        m.seed(weapon+offsets::original_owner,local_xuid+1);
        require(!g.owned_weapon(g.held_weapon(100)),"foreign original owner excluded");
        m.seed(slot+0x10,std::uint32_t{100+0x8000});
        rejects([&]{g.held_weapon(100);},"reused entity slot with different serial rejected");
        m.seed(slot+0x10,std::uint32_t{100});m.seed(pawn+offsets::health,0);
        rejects([&]{g.held_weapon(100);},"dead local player cannot label a spectator weapon");
        require(m.writes==0,"all presentation queries read only");
    });
    test("expired backup pruning retains live dropped originals", [] {
        FakeMemory m;Game g(m,client,engine,fixture_build);g.apply(ak);
        m.seed(weapon+offsets::owner,std::uint32_t{0xffffffffU});
        int visits=0;
        g.prune_expired([&](const WeaponState& s){++visits;require(s.weapon==weapon,"backup identity");return false;});
        require(visits==1 && g.backup_count()==1,"live dropped backup is kept");
        const auto writes=m.writes;
        g.prune_expired([](const WeaponState&){return true;});
        require(g.backup_count()==0 && m.writes==writes,"expired backup removed without stale restoration writes");
    });
    test("knife fallback backup survives model definition changes", [] {
        FakeMemory m; Game g(m,client,engine,fixture_build);
        m.seed(item+offsets::definition,std::uint16_t{42});
        const auto original=g.values(g.snapshot());
        Cosmetic knife{"T","knife",500,415,0,0.01f};g.apply(knife);
        m.seed(item+offsets::definition,std::uint16_t{500});
        require(g.has_active_backup(),"backup follows same knife object");
        knife.definition=507;g.apply(knife);
        m.seed(item+offsets::definition,std::uint16_t{507});
        require(g.backup_count()==1,"one original knife backup");
        g.restore();require(g.values(g.snapshot())==original,"knife fallback restored");
    });
    test("every catalog firearm applies and restores its own fallback slot", [] {
        unsigned count=0;
        for(const auto& w:catalog::weapons) {
            if(!catalog::firearm(w.definition))continue;
            FakeMemory m;Game g(m,client,engine,fixture_build);
            m.seed(item+offsets::definition,static_cast<std::uint16_t>(w.definition));
            const auto original=g.values(g.snapshot());
            const catalog::Finish* finish=nullptr;
            for(const auto& f:catalog::finishes)if(f.definition==w.definition && f.paint) { finish=&f;break; }
            require(finish,"each firearm has finishes");
            g.apply({"T","weapon",w.definition,finish->paint,22,finish->minimum});
            require(g.values(g.snapshot()).paint==finish->paint,"weapon-specific paint applied");
            g.restore();require(g.values(g.snapshot())==original,"original fallback restored");++count;
        }
        require(count==35,"35 firearm/equipment definitions");
    });
    test("rifle choice cannot write a different held rifle", [] {
        FakeMemory m;Game g(m,client,engine,fixture_build);
        rejects([&]{g.apply({"T","weapon",16,309,0,0.1f});},"M4 must not write an AK");
        require(m.writes==0,"mismatched gun untouched");
    });
    test("glove snapshot is independent of active weapon", [] {
        FakeMemory m;Game g(m,client,engine,fixture_build);
        const auto pawn_identity=chunk+offsets::entity_stride;
        m.seed(pawn+offsets::identity,pawn_identity);
        m.seed(pawn_identity+0x10,std::uint32_t{1});
        m.seed(pawn+0x17a0+offsets::definition,std::uint16_t{5030});
        m.seed(services+offsets::active_weapon,std::uint32_t{0xffffffffU});
        auto gloves=g.glove_snapshot();
        require(gloves.item==pawn+0x17a0 && gloves.definition==5030,"pawn glove item resolved");
        rejects([&]{g.snapshot();},"no held weapon");
        m.seed(pawn+0x17a0+offsets::definition,std::uint16_t{4725});
        require(gloves.same_object(g.glove_snapshot()),"glove model changes retain pawn identity");
        m.seed(pawn+offsets::health,0);
        rejects([&]{g.glove_snapshot();},"dead pawn blocks gloves");
    });
    test("apply and restore exact original values", [] {
        FakeMemory m; Game g(m, client, engine,fixture_build);
        auto original = g.values(g.snapshot()); g.apply(ak);
        require(g.values(g.snapshot()).paint == 474 && g.has_backup(), "apply result");
        g.restore(); require(g.values(g.snapshot()) == original && !g.has_backup(), "restore result");
    });
    test("reapply preserves first backup", [] {
        FakeMemory m; Game g(m, client, engine,fixture_build); auto original = g.values(g.snapshot());
        g.apply(ak); auto newer = ak; newer.seed = 999; g.apply(newer); g.restore();
        require(g.values(g.snapshot()) == original, "original preserved");
    });
    test("picked-up foreign and unknown weapons receive zero writes", [] {
        for(auto owner:{local_xuid+1,std::uint64_t{0}}){
            FakeMemory m;Game g(m,client,engine,fixture_build);m.seed(weapon+offsets::original_owner,owner);
            require(!g.owned_weapon(g.snapshot()),"foreign/unknown original owner");
            rejects([&]{g.apply(ak);},"preserve picked-up appearance");require(m.writes==0,"no foreign writes");
        }
    });
    test("own dropped weapon is eligible after pickup", [] {
        FakeMemory m;Game g(m,client,engine,fixture_build);auto s=g.snapshot();
        m.seed(weapon+offsets::owner,std::uint32_t{0xffffffff});
        require(!g.owned_weapon(s),"dropped weapon cannot be changed");
        m.seed(weapon+offsets::owner,std::uint32_t{1});
        require(g.owned_weapon(g.snapshot()),"own original owner retained");g.apply(ak);
        require(m.writes>0,"own reacquired weapon applies");
    });
    test("controller association and unknown local identity fail closed", [] {
        FakeMemory m;Game g(m,client,engine,fixture_build);m.seed(controller+offsets::controller_pawn,std::uint32_t{100});
        rejects([&]{g.apply(ak);},"mismatched controller");require(m.writes==0,"no writes");
        m.seed(controller+offsets::controller_pawn,std::uint32_t{1});m.seed(controller+offsets::controller_steam_id,std::uint64_t{0});
        rejects([&]{g.apply(ak);},"unknown local identity");require(m.writes==0,"no writes");
    });
    test("build mismatch has zero writes", [] {
        FakeMemory m; Game g(m, client, engine,fixture_build); m.seed(engine + offsets::build_number, 1);
        rejects([&]{g.apply(ak);}, "build mismatch"); require(m.writes == 0, "no writes");
    });
    test("dead player has zero writes", [] {
        FakeMemory m; Game g(m, client, engine,fixture_build); m.seed(pawn + offsets::health, 0);
        rejects([&]{g.apply(ak);}, "dead player"); require(m.writes == 0, "no writes");
    });
    test("non AK weapon blocked", [] {
        FakeMemory m; Game g(m, client, engine,fixture_build); m.seed(item + offsets::definition, std::uint16_t{9});
        rejects([&]{g.apply(ak);}, "wrong weapon"); require(m.writes == 0, "no writes");
    });
    test("team mismatch blocked", [] {
        FakeMemory m; Game g(m, client, engine,fixture_build); auto c = ak; c.team = "CT";
        rejects([&]{g.apply(c);}, "wrong team"); require(m.writes == 0, "no writes");
    });
    test("wrong owner blocked", [] {
        FakeMemory m; Game g(m, client, engine,fixture_build); m.seed(chunk + offsets::entity_stride, weapon);
        rejects([&]{g.apply(ak);}, "wrong owner"); require(m.writes == 0, "no writes");
    });
    test("wrong identity layout blocked", [] {
        FakeMemory m; Game g(m, client, engine,fixture_build); m.seed(weapon + offsets::identity, slot + 8);
        rejects([&]{g.apply(ak);}, "wrong identity"); require(m.writes == 0, "no writes");
    });
    test("partial write failure rolls back", [] {
        FakeMemory m; Game g(m, client, engine,fixture_build); auto before = g.values(g.snapshot()); m.fail_write = 3;
        rejects([&]{g.apply(ak);}, "write failed");
        require(g.values(g.snapshot()) == before && !g.has_backup(), "rollback");
    });
    test("weapon switch blocks stale restoration", [] {
        FakeMemory m; Game g(m, client, engine,fixture_build); g.apply(ak); int before = m.writes;
        m.seed(services + offsets::active_weapon, std::uint32_t{101});
        rejects([&]{g.restore();}, "changed weapon"); require(m.writes == before, "no stale write");
    });
    test("process exit blocks restoration", [] {
        FakeMemory m; Game g(m, client, engine,fixture_build); g.apply(ak); int before = m.writes; m.running = false;
        rejects([&]{g.restore();}, "exited"); require(m.writes == before, "no stale write");
    });
    test("game value conflict is not overwritten", [] {
        FakeMemory m; Game g(m, client, engine,fixture_build); g.apply(ak);
        m.seed(weapon + offsets::paint, std::int32_t{777});
        g.restore();
        require(m.get<std::int32_t>(weapon + offsets::paint) == 777, "game paint preserved");
        require(m.get<std::int32_t>(weapon + offsets::seed) == 7, "our seed restored");
        require(!g.has_backup(), "backup released");
    });
    test("invalid numeric input blocked", [] {
        FakeMemory m; Game g(m, client, engine,fixture_build); auto c = ak; c.wear = NAN;
        rejects([&]{g.apply(c);}, "NaN"); require(m.writes == 0, "no writes");
    });
    test("knife input blocked", [] {
        FakeMemory m; Game g(m, client, engine,fixture_build); auto c = ak; c.category = "knife"; c.definition = 500;
        rejects([&]{g.apply(c);}, "knife"); require(m.writes == 0, "no writes");
    });
    test("missing restore backup rejected", [] {
        FakeMemory m; Game g(m, client, engine,fixture_build);
        rejects([&]{g.restore();}, "no backup"); require(m.writes == 0, "no writes");
    });
    test("identity change mid-write halts further writes", [] {
        FakeMemory m; Game g(m, client, engine,fixture_build);
        m.after_write = [&]{m.seed(services + offsets::active_weapon, std::uint32_t{101});};
        rejects([&]{g.apply(ak);}, "switch mid-write"); require(m.writes == 1 && g.has_backup(), "halted");
    });
    test("game correction no longer locks reapply", [] {
        FakeMemory m; Game g(m, client, engine,fixture_build); g.apply(ak);
        m.seed(item + offsets::item_id_high, std::uint32_t{555});
        auto newer = ak; newer.paint = 282; newer.wear = 0.15f;
        g.apply(newer); g.restore();
        require(m.get<std::uint32_t>(item + offsets::item_id_high) == 555, "new baseline retained");
        require(m.get<std::int32_t>(weapon + offsets::paint) == 12, "original paint retained");
    });
    test("separate weapons have separate backups", [] {
        FakeMemory m; Game g(m, client, engine,fixture_build); g.apply(ak);
        constexpr Address second = 0x33000000;
        const auto second_item = second + offsets::attribute_manager + offsets::item;
        const auto second_slot = chunk + offsets::entity_stride * 101;
        m.seed(second_slot, second);
        m.seed(second + offsets::identity, second_slot);
        m.seed(second + offsets::owner, std::uint32_t{1});
        m.seed(second + offsets::original_owner,local_xuid);
        m.seed(second_item + offsets::definition, std::uint16_t{7});
        m.seed(second + offsets::paint, std::int32_t{10});
        m.seed(second + offsets::seed, std::int32_t{20});
        m.seed(second + offsets::wear, 0.4f);
        m.seed(second_item + offsets::item_id_high, std::uint32_t{30});
        m.seed(services + offsets::active_weapon, std::uint32_t{101});
        require(!g.has_active_backup(), "new active has no backup");
        g.apply(ak); require(g.backup_count() == 2, "two independent backups");
        g.restore(); require(g.backup_count() == 1, "first backup retained");
        require(m.get<std::int32_t>(second + offsets::paint) == 10, "second restored");
        m.seed(services + offsets::active_weapon, std::uint32_t{100});
        g.restore(); require(!g.has_backup(), "all restored after selecting each");
        require(m.get<std::int32_t>(weapon + offsets::paint) == 12, "first restored");
    });
    std::cout << checks << " transaction checks passed\n";
}
