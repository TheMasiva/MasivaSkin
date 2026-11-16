#pragma once
#include <cstdint>
namespace masiva::preview_policy {
inline bool local_character(std::uint64_t local,std::uint64_t scene,int team,bool visible) {
    return visible && local!=0 && local==scene && (team==2 || team==3);
}
inline void advance_verification(std::uint64_t& deadline,std::uint64_t now) {
    if(now>=deadline)deadline=now+250;
}
inline bool needs_item_read(bool has_backup,bool same_choice,bool due) {
    return !has_backup || !same_choice || due;
}
inline bool should_update(bool has_backup,bool same_choice,bool same_item_id,bool due) {
    return !has_backup || !same_choice || (due && !same_item_id);
}
}
