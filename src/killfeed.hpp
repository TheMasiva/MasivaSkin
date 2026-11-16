#pragma once
#include <string_view>
namespace masiva {
struct KnifeIcon { int definition; const char* icon; };
// Item names from the supplied items_game snapshot, without weapon_.
inline constexpr KnifeIcon knife_icons[] = {
{500,"bayonet"},{503,"knife_css"},{505,"knife_flip"},{506,"knife_gut"},
{507,"knife_karambit"},{508,"knife_m9_bayonet"},{509,"knife_tactical"},
{512,"knife_falchion"},{514,"knife_survival_bowie"},{515,"knife_butterfly"},
{516,"knife_push"},{517,"knife_cord"},{518,"knife_canis"},{519,"knife_ursus"},
{520,"knife_gypsy_jackknife"},{521,"knife_outdoor"},{522,"knife_stiletto"},
{523,"knife_widowmaker"},{525,"knife_skeleton"},{526,"knife_kukri"}};
inline const char* knife_icon(int definition) {
    for(const auto& k:knife_icons)if(k.definition==definition)return k.icon;
    return nullptr;
}
inline bool knife_event(std::string_view weapon) {
    if(weapon.substr(0,7)=="weapon_")weapon.remove_prefix(7);
    if(weapon=="knife" || weapon=="knife_t")return true;
    for(const auto& k:knife_icons)if(weapon==k.icon)return true;
    return false;
}
inline const char* killfeed_icon(std::string_view event,std::string_view weapon,
    int attacker,int local,int definition,bool applied) {
    if(event!="player_death" || local<0 || attacker!=local || !applied || !knife_event(weapon))return nullptr;
    return knife_icon(definition);
}
}
