#include "killfeed.hpp"
#include "catalog.hpp"
#include <stdexcept>
#include <set>
#include <iostream>
using namespace masiva;
void require(bool v,const char* message){if(!v)throw std::runtime_error(message);}
int main(){
    std::set<std::string_view> names;
    unsigned models=0;
    for(const auto& w:catalog::weapons)if(catalog::knife(w.definition)){
        ++models;const auto icon=knife_icon(w.definition);
        require(icon && names.insert(icon).second,"every knife has a distinct icon mapping");
        require(killfeed_icon("player_death","knife",4,4,w.definition,true)==icon,"local T knife kill");
        require(killfeed_icon("player_death","knife_t",4,4,w.definition,true)==icon,"local CT knife kill");
        require(killfeed_icon("player_death",icon,4,4,w.definition,true)==icon,"server named knife kill");
        require(!killfeed_icon("player_death","knife",5,4,w.definition,true),"other player untouched");
        require(!killfeed_icon("player_death","ak47",4,4,w.definition,true),"firearm kill untouched while knife selected");
        require(!killfeed_icon("player_death","knife",4,4,w.definition,false),"queued but unapplied choice untouched");
    }
    require(models==20,"all twenty knife models covered");
    require(!killfeed_icon("round_start","knife",4,4,500,true),"other event untouched");
    require(!killfeed_icon("player_death","knife",-1,-1,500,true),"invalid local slot never matches world");
    require(!killfeed_icon("player_death","knife",4,4,999,true),"unknown model fails closed");
    require(!knife_event("knife_fake") && knife_event("weapon_knife_butterfly"),"exact supported knife names only");
    std::cout<<"Kill feed local identity, event and model filtering passed\n";
}
