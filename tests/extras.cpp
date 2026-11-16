#include "profile_packet.hpp"
#include "decorations.hpp"
#include "model_loading.hpp"
#include <iostream>
using namespace masiva;
void require(bool value,const char* why){if(!value)throw std::runtime_error(why);}
template<class F> bool rejects(F f){try{f();}catch(const std::exception&){return true;}return false;}
int main(){
    require(normalize_model_path("AGENTS\\models\\tm_test.vmdl_c")=="agents/models/tm_test.vmdl","compiled resource paths normalize for comparison");
    require(normalize_model_path("../bad.vmdl").empty() && normalize_model_path("bad.png").empty(),"invalid restore paths rejected");
    ModelLoadWindow load;load.begin(1000);
    require(load.pending(1001) && load.pending(5999) && !load.expired(5999),"async load keeps one request pending");
    require(load.expired(6000) && !load.pending(6000),"pending resources have bounded wait");
    load.clear();require(!load.expired(7000) && !load.pending(7000),"verified or changed selection clears previous request");
    require(extras::buyable(7,2) && !extras::buyable(7,3),"AK team filter");
    require(!extras::buyable(16,2) && extras::buyable(16,3) && !extras::buyable(60,2),"M4 team filter");
    for(int d:{9,40,1,2,36,63,64,31,24})require(extras::buyable(d,2) && extras::buyable(d,3),"shared buy-menu weapons");
    for(const auto& w:catalog::weapons)if(catalog::firearm(w.definition))require(extras::buyable(w.definition,2) || extras::buyable(w.definition,3),"each firearm has a team");
    require(std::size(extras::stickers)==11135 && std::size(extras::charms)==79 && std::size(extras::agents)==63,"catalog counts include None entries");
    Cosmetic c{"T","weapon",7,474,3,0.01f,123,"Masiva Türkçe"};
    for(unsigned i=0;i<5;++i)c.decorations.stickers[i]=extras::stickers[i+1].id;
    c.decorations.charm=extras::charms[1].id;c.decorations.charm_seed=100000;
    c.decorations.x=1.25f;c.decorations.y=-3.f;c.decorations.z=0.5f;
    const auto agent_t=*std::find_if(std::begin(extras::agents),std::end(extras::agents),[](const auto& a){return a.team==2;});
    const auto agent_ct=*std::find_if(std::begin(extras::agents),std::end(extras::agents),[](const auto& a){return a.team==3;});
    Cosmetic t{"T","agent",agent_t.id,0,0,0},ct{"CT","agent",agent_ct.id,0,0,0};
    std::vector<Cosmetic> profile{c,t,ct};
    std::ostringstream out;write(out,profile);std::istringstream in(out.str());auto decoded=read(in);
    require(decoded[0].decorations==c.decorations && decoded[0].custom_name==c.custom_name,"disk profile retains all attachments and labels");
    auto packet=encode_profile(decoded);auto transferred=decode_profile(packet);
    require(transferred[0].decorations==c.decorations,"IPC preserves all attachment fields");
    Selections selected;selected.set_all(transferred);
    require(selected.get(2,60000)->definition==agent_t.id && selected.get(3,60000)->definition==agent_ct.id,"independent agent slots");
    selected.clear(2,agent_t.id);require(!selected.get(2,60000) && selected.get(3,60000) && selected.get(2,7),"agent restore isolates team and category");
    const auto state=decoration_values(c.decorations);
    require(*state[0]==static_cast<unsigned>(c.decorations.stickers[0]) && *state[20]==static_cast<unsigned>(c.decorations.charm),"IDs use integer schema bits");
    require(*state[2]==float_bits(1.f) && *state[21]==float_bits(1.25f) && *state[24]==100000,"scale/offset are floats; charm seed is integer");
    const auto blank=decoration_values(Decorations{});
    for(unsigned slot=0;slot<5;++slot)require(blank[slot*4].has_value() && *blank[slot*4]==0,"None explicitly suppresses inventory sticker");
    require(blank[20].has_value() && *blank[20]==0,"None explicitly suppresses inventory charm");
    // Reproduce the inventory fallback: removing a dynamic ID exposes the SOC
    // sticker again. An explicit zero survives even with a decorated inventory.
    const auto inventory_id=static_cast<std::uint32_t>(extras::stickers[1].id);
    auto effective=[&](const DecorationState& dynamic,unsigned i){return dynamic[i].value_or(inventory_id);};
    const DecorationState original{};
    require(effective(original,0)==inventory_id,"fixture must have inherited inventory sticker");
    for(unsigned slot=0;slot<5;++slot)require(effective(blank,slot*4)==0,"blank choice hides all inventory stickers");
    require(effective(blank,20)==0,"blank choice hides inherited charm");
    auto single=Decorations{};single.stickers[2]=extras::stickers[2].id;
    const auto one=decoration_values(single);
    for(unsigned slot=0;slot<5;++slot)require(effective(one,slot*4)==static_cast<unsigned>(single.stickers[slot]),"selected sticker only; other slots remain empty");
    require(effective(original,0)==inventory_id && !original[0],"restore retains absence and re-exposes original inventory decoration");
    auto bad=c;bad.decorations.stickers[4]=99999999;require(rejects([&]{validate(bad);}),"unknown last sticker rejected");
    bad=c;bad.decorations.x=NAN;require(rejects([&]{validate(bad);}),"NaN attachment position rejected");
    bad=t;bad.team="CT";require(rejects([&]{validate(bad);}),"wrong-team agent rejected");
    bad=t;bad.decorations=c.decorations;require(rejects([&]{validate(bad);}),"agent cannot carry weapon attachments");
    auto old=c;old.decorations={};selected.set(old);auto broken=profile;broken.back().team="T";
    require(rejects([&]{selected.set_all(broken);}) && selected.get(2,7)->decorations==Decorations{},"invalid agent cannot partially commit decorated gun");
    std::istringstream legacy("MASIVA_SKIN 2 1\nT weapon 7 474 0 0.01 10 \"Legacy\"\n");
    auto older=read(legacy);require(older[0].decorations==Decorations{} && older[0].stattrak==10,"v2 remains readable");
    std::cout<<"Team filters, agent isolation, decoration ABI, v3 profile and IPC passed\n";
}
