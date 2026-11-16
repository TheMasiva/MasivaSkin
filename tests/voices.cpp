#include "agent_voice.hpp"
#include "extras_catalog.hpp"
#include <stdexcept>
#include <iostream>
#include <set>
using namespace masiva;
void require(bool v,const char* m){if(!v)throw std::runtime_error(m);}
int main() {
    require(local_voice_source(123,0x1807b),"matching local entity index");
    require(!local_voice_source(124,0x1807b),"other speaker untouched");
    require(!local_voice_source(-1,0xffffffff) && !local_voice_source(0,0),"world/invalid sources untouched");
    require(!voice_group("ak47.fire","professional_fem"),"non-voice event untouched");
    require(!voice_group("announcer.round_start_01","swat_fem"),"announcer untouched");
    require(!voice_group("phoenix.t_flashbang01","phoenix"),"same bank untouched");
    require(!voice_group("phoenix.no_such_call99","swat_fem"),"unmapped semantic retains original");
    require(!voice_group(std::string(300,'x'),"swat_fem"),"bounded input");
    const auto flash=voice_group("PHOENIX.T_FLASHBANG01","professional_fem");
    require(flash && flash->count>0,"case insensitive legacy to modern flash");
    require(std::string_view(flash->key)=="professional_fem.throwing_flashbang","same meaning preserved");
    const auto radio=voice_group("fbihrt.radiobotfollowme03","swat_fem");
    require(radio && std::string_view(radio->key)=="swat_fem.request_follow_me","Z follow me mapped");
    require(voice_group("professional_fem.throwing_flashbang_04","phoenix"),"modern to legacy flash");
    require(voice_group("phoenix.onarollbrag15","professional_fem") &&
        std::string_view(voice_group("phoenix.onarollbrag15","professional_fem")->key)=="professional_fem.cheer","T Z cheer legacy concept");
    require(voice_group("sas.onarollbrag14","swat_fem") &&
        std::string_view(voice_group("sas.onarollbrag14","swat_fem")->key)=="swat_fem.cheer","CT Z cheer legacy concept");
    require(voice_group("phoenix.plantingbomb03","jungle_fem_epic") &&
        std::string_view(voice_group("phoenix.plantingbomb03","jungle_fem_epic")->key)=="jungle_fem_epic.i_plant_bomb","T planting uses chosen agent");
    require(voice_group("professional.radiobotplanting02","professional_fem"),"alternate legacy plant");
    require(voice_group("sas.defusingbomb03","gendarmerie_fem_epic") &&
        std::string_view(voice_group("sas.defusingbomb03","gendarmerie_fem_epic")->key)=="gendarmerie_fem_epic.defusing","CT defuse uses chosen agent");
    require(voice_group("swat.radiobotdefusing01","seal_fem"),"alternate CT defuse");
    require(!voice_group("phoenix.plantingbomb03","leet_epic"),"no invented generic plant or incorrect A/B call");
    require(voice_equal(body_voice_event("Player.DamageFall","professional_fem"),"Player.DamageFall.Fem"),"female falling pain");
    require(voice_equal(body_voice_event("Player.DamageFall.Fem","professional_epic"),"Player.DamageFall"),"switch back to male falling pain");
    require(voice_equal(body_voice_event("BaseGrenade.JumpThrowM","seal_fem"),"BaseGrenade.JumpThrowF"),"female jump-throw effort");
    require(voice_equal(body_voice_event("BaseGrenade.JumpThrowF","seal_epic"),"BaseGrenade.JumpThrowM"),"male jump-throw effort");
    require(!body_voice_event("BaseGrenade.JumpThrowF","swat_fem"),"matching shared voice retained");
    for(const auto event:{"Default.WalkJump","Player.DamageBody.Victim","C4.PlantSound","C4.DisarmStart","Player.DamageFall.Fem.extra"})
        require(!body_voice_event(event,"swat_fem"),"footsteps, impacts, bomb effects and prefix-like names untouched");
    require(!body_voice_event("Player.DamageFall","unknown_bank"),"unknown agent bank cannot select a voice");
    require(agent_voice_event("SAS.onarollbrag01") && agent_voice_event("Player.DamageFall"),"unmapped voice history recognizes relevant events");
    require(!agent_voice_event("Weapon_AK47.Single") && !agent_voice_event("Default.WalkJump"),"nonvoice sounds cannot flood history");
    std::set<std::string_view> keys;
    for(const auto& group:voice_data::groups) {
        require(keys.insert(group.key).second && group.count>0,"unique nonempty groups");
        require(group.first+group.count<=std::size(voice_data::events),"group range bounds");
    }
    unsigned agents=0;
    for(const auto& a:extras::agents) {
        ++agents;
        const auto source=std::string_view(a.voice)=="phoenix"?"fbihrt.ct_flashbang01":"phoenix.t_flashbang01";
        require(voice_group(source,a.voice),"every agent has a mapped flash event");
        const auto follow=std::string_view(a.voice)=="fbihrt"?"phoenix.radio_followme01":"fbihrt.radiobotfollowme01";
        require(voice_group(follow,a.voice),"every agent has follow me radio");
        const auto cheer=std::string_view(a.voice)=="phoenix"?"sas.onarollbrag01":"phoenix.onarollbrag01";
        require(voice_group(cheer,a.voice),"all 63 agents have Z cheer mapping");
    }
    require(agents==63,"all 63 catalog agents covered");
    std::cout<<"Local voice filtering, legacy/modern semantic mapping and 63 agent banks passed\n";
}
