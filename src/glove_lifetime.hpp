#pragma once
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace masiva {
// A glove is embedded in the current pawn. Backups from a previous spawn
// cannot be restored into a later spawn, even when its address is reused.
template<class Backups,class State>
std::size_t retain_glove_lifetime(Backups& backups,const State& current,float spawn) {
    if(!std::isfinite(spawn))throw std::runtime_error("Oyuncu dogma kimligi gecersiz.");
    const auto previous=backups.size();
    backups.erase(std::remove_if(backups.begin(),backups.end(),[&](const auto& entry){
        return !entry.state.same_object(current) || entry.spawn!=spawn;
    }),backups.end());
    return previous-backups.size();
}
}
