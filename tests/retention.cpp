#include "retention.hpp"
#include "catalog.hpp"
#include "selections.hpp"
#include "profile_packet.hpp"
#include "item_label.hpp"
#include "cosmetic_refresh.hpp"
#include <iostream>
#include <set>
#include <stdexcept>
using namespace masiva;
void require(bool v,const char* message) { if(!v)throw std::runtime_error(message); }
int main() {
    for(std::uint32_t value:{0U,1U,123U,999999U}) {
        auto encoded=integer_attribute(value);std::uint32_t decoded{};
        std::memcpy(&decoded,&encoded,sizeof(decoded));
        require(decoded==value,"integer schema attribute must preserve bits, not numeric float conversion");
    }
    MaterialCommit commit;
    require(!commit.due(1000),"no refresh without selection");
    commit.arm(1000);
    require(!commit.due(1399) && commit.due(1400),"first delayed commit");
    require(!commit.due(1401) && commit.due(2200),"second delayed commit");
    require(!commit.due(2201) && !commit.due(9000),"never loops after two commits");
    commit.arm(10000);
    require(!commit.due(16000),"do not refresh stale unequipped selections");
    commit.arm(17000);require(commit.due(17400),"new request restarts bounded commit");
    ModelSettle window(1000);
    require(window.due(1000),"first-person update starts immediately");
    require(!window.due(1001) && !window.due(1099),"model load retries throttled");
    require(window.due(1100),"pending material can retry");
    require(window.due(5900),"slow resource loads keep a bounded opportunity");
    require(window.expired(6000) && !window.due(6000),"load retries end, never restart world model");
    ModelSettle producer(1000);
    require(producer.world_due(false,1000),"retry missing world producer");
    require(!producer.world_due(false,1100),"world producer throttled separately");
    require(!producer.world_due(true,1500),"never recreate an existing pending model");
    require(producer.world_due(false,1500),"missing producer can recover after initial failure");
    require(!producer.world_due(false,6000) && producer.world_attempts()==2,"world retries expire too");
    AppearanceMemo first,second;AppearanceKey one{};one.weapon=100;one.handle=7;one.hud=500;
    AppearanceKey two=one;two.weapon=200;two.handle=8;two.hud=600;
    require(!first.reusable(one,true,false),"first application must generate materials");
    first.remember(one);second.remember(two);
    require(first.reusable(one,true,false) && second.reusable(two,true,false),"A to B to A preserves per-weapon resources");
    require(!first.reusable(one,false,false),"network-reset attributes require regeneration");
    require(!first.reusable(one,true,true),"compatibility mode keeps previous refresh behavior");
    auto changed=one;changed.hud_model++;
    require(!first.reusable(changed,true,false),"new HUD resource needs regeneration");
    changed=one;changed.handle++;
    require(!first.reusable(changed,true,false),"drop/reuse with new serial cannot reuse stale appearance");
    changed=one;changed.pawn++;
    require(!first.reusable(changed,true,false),"new pawn cannot reuse prior life resources");
    Retention r; AppearanceKey a{}; a.weapon=100;a.handle=12;a.definition=7;
    require(r.needs(a,true,0),"initial observation");
    r.applied(a,0);
    require(!r.needs(a,true,300),"stable appearance must not regenerate");
    require(!r.needs(a,false,100),"settling delay");
    require(r.needs(a,false,300),"network reset must regenerate");
    r.invalidate(); require(r.needs(a,true,310),"return after other weapon/drop");
    r.applied(a,310); auto b=a;b.hud=200;
    require(r.needs(b,true,320),"new HUD must regenerate");
    r.applied(b,320);b.handle++;
    require(r.needs(b,true,330),"reused entity with new serial");
    r.applied(b,330);b.hud_model++;
    require(r.needs(b,true,340),"model resource replacement");
    for(unsigned i=0;i<8;++i) {
        const auto time=1000+1000*i;
        require(r.needs(b,false,time),"retry available");r.failed(time);
        require(!r.needs(b,false,time+500),"retry throttled");
    }
    require(!r.needs(b,false,100000),"persistent failure bounded");
    b.handle++;require(r.needs(b,false,100001),"new weapon recovers retry budget");
    r.applied(b,100001);require(r.failures()==0,"success clears failures");
    Retention spawn_wait;AppearanceKey spawn_key{};spawn_key.pawn=1000;spawn_key.world_model=1;
    for(std::uint64_t now=0;now<3000;now+=150){
        require(spawn_wait.needs(spawn_key,false,now),"respawn readiness polling remains available");
        spawn_wait.waiting_for_model(now);
        require(!spawn_wait.needs(spawn_key,false,now+149),"waiting does not busy-loop every frame");
        require(spawn_wait.failures()==0,"missing arms do not consume the eight transaction retries");
    }
    require(spawn_wait.needs(spawn_key,false,3000),"apply on first poll after arms become ready");
    spawn_wait.applied(spawn_key,3000);
    require(!spawn_wait.needs(spawn_key,true,3400),"ready stable glove does not rebuild");
    spawn_wait.failed(3400);
    require(!spawn_wait.needs(spawn_key,false,3550),"actual transaction errors retain slow retry policy");
    spawn_wait.invalidate();
    require(spawn_wait.needs(spawn_key,false,3550),"death clears the old lifetime cooldown");
    std::set<std::pair<int,int>> pairs; unsigned ak=0,knives=0,guns=0,gloves=0;
    for(const auto& f:catalog::finishes) {
        require(catalog::weapon(f.definition),"known model");
        require(pairs.emplace(f.definition,f.paint).second,"unique compatible pair");
        require(f.minimum>=0 && f.maximum<=1 && f.minimum<=f.maximum,"valid wear range");
        if(f.definition==7)++ak;
        if(catalog::knife(f.definition))++knives;else if(catalog::firearm(f.definition))++guns;else if(catalog::glove(f.definition))++gloves;
    }
    require(ak==62 && knives==576 && guns==1491 && gloves==94,"catalog completeness for source snapshot");
    for(const auto& w:catalog::weapons)if(!catalog::glove(w.definition))require(catalog::finish(w.definition,0),"vanilla per model");
    require(!catalog::finish(7,415),"knife paint must not appear for AK");
    Selections selections;
    selections.set({"T","weapon",7,474,0,0.1f});
    selections.set({"T","weapon",16,309,5,0.1f});
    selections.set({"CT","weapon",7,801,0,0.1f});
    selections.set({"T","knife",500,415,0,0.1f});
    require(selections.get(2,7)->paint==474 && selections.get(2,16)->paint==309,"independent firearm choices");
    require(selections.get(3,7)->paint==801 && !selections.get(3,16),"team isolation");
    selections.set({"T","knife",507,415,0,0.1f});
    require(selections.get(2,500)->definition==507 && selections.get(2,42)->definition==507,"one knife slot follows replacement");
    require(selections.get(2,7)->paint==474,"knife switch preserves gun selection");
    selections.clear(2,507);
    require(!selections.get(2,42) && selections.get(2,16),"restore disables only current slot");
    require(catalog::compatible(500,507) && !catalog::compatible(7,16) && !catalog::knife(16),"weapon category boundaries");
    selections.set({"T","gloves",5030,10018,0,0.1f});
    selections.set({"CT","gloves",5030,10018,2,0.2f});
    require(selections.get(2,5031)->definition==5030,"all glove families share team slot");
    require(selections.get(3,5030)->seed==2,"glove teams independent");
    selections.clear(2,5033);
    require(!selections.get(2,5030) && selections.get(2,7) && selections.get(3,5030),"glove restore preserves other selections");
    // Selection registration requires neither a pawn nor a held weapon.
    selections.set({"T","knife",500,415,0,0.1f});
    selections.set({"T","weapon",16,309,0,0.1f});
    require(selections.get(2,42)->definition==500,"queued knife resolves when later equipped");
    require(selections.get(2,16)->paint==309 && !catalog::compatible(7,16),"queued M4 cannot target held AK");
    selections.set({"T","knife",507,415,0,0.1f});
    require(selections.get(2,500)->definition==507,"new queued knife replaces old choice");
    // During a category rebuild, the previous row retains its own identity.
    const auto previous=catalog::finish(7,282);
    auto transitional=item_label(previous->definition,previous->paint);
    require(std::string(transitional.data())=="AK-47 | Redline","preview independent of transient new model");
    auto label=item_label(7,282);
    require(std::string(label.data())=="AK-47 | Redline","HUD name format");
    require(std::string(item_label(7,0).data())=="AK-47","vanilla HUD name");
    for(const auto& f:catalog::finishes) {
        auto name=item_label(f.definition,f.paint);require(name.back()==0,"bounded name buffer");
    }
    std::vector<Cosmetic> profile={
        {"T","weapon",7,474,10,0.1f,123,"Türkçe isim"},
        {"CT","weapon",7,801,11,0.2f},
        {"T","knife",500,415,12,0.1f},
        {"CT","gloves",5030,10018,13,0.2f}};
    for(auto& c:profile)c.wear=catalog::finish(c.definition,c.paint)->minimum;
    auto packet=encode_profile(profile);
    auto decoded=decode_profile(packet);
    require(decoded.size()==4 && decoded[0].custom_name==profile[0].custom_name && decoded[0].stattrak==123,"profile packet preserves names and StatTrak");
    selections.set_all(decoded);
    // Reproduce load profile -> edit only attachments -> queue the same skin.
    auto edited=*selections.get(2,7);
    edited.decorations.stickers[0]=extras::stickers[1].id;
    edited.decorations.charm=extras::charms[1].id;
    selections.set(edited);
    require(selections.get(2,7)->decorations==edited.decorations,"profile-loaded slot accepts attachment-only edit");
    require(selections.get(3,7)->decorations==Decorations{},"attachment edit does not alter opposite team");
    std::ostringstream edited_disk;write(edited_disk,{*selections.get(2,7)});
    std::istringstream edited_input(edited_disk.str());
    auto edited_roundtrip=decode_profile(encode_profile(read(edited_input)));
    require(edited_roundtrip[0].decorations==edited.decorations,"edited loaded attachments survive disk and IPC");
    require(selections.get(2,7)->seed==10 && selections.get(3,7)->seed==11 && selections.get(2,42)->seed==12 && selections.get(3,5031)->seed==13,"batch activates all teams and categories");
    require(selections.get(2,16)!=nullptr,"profile preserves unrelated session choices");
    auto rejects=[](auto action) {try {action();}catch(const std::exception&) {return true;}return false;};
    auto invalid=profile;invalid[0].seed=99;invalid.back().paint=999999;
    require(rejects([&]{selections.set_all(invalid);}) && selections.get(2,7)->seed==10,"invalid last row cannot partially apply batch");
    auto duplicate=profile;duplicate.push_back({"T","knife",507,415,0,0.1f});
    require(rejects([&]{encode_profile(duplicate);}),"duplicate knife slot rejected");
    auto malformed=packet;malformed.count=513;
    require(rejects([&]{decode_profile(malformed);}),"packet count bounded");
    malformed=packet;malformed.items[0].name.fill('x');
    require(rejects([&]{decode_profile(malformed);}),"packet name terminator required");
    malformed=packet;malformed.items[0].team=4;
    require(rejects([&]{decode_profile(malformed);}),"packet team validated");
    selections.clear();require(!selections.get(2,7) && !selections.get(3,7),"stop disables both teams");
    std::cout<<"Retention lifecycle, retry budget and 2161 catalog entries passed\n";
}
