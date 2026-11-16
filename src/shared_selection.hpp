#pragma once
#include <algorithm>
#include "loadout.hpp"
#include "catalog.hpp"
namespace masiva {
inline bool shared_item(int definition) {
    return !catalog::agent(definition) && extras::buyable(definition,2) && extras::buyable(definition,3);
}
inline std::vector<Cosmetic> selection_targets(Cosmetic c,bool both) {
    validate(c);
    if(!both)return {c};
    if(!shared_item(c.definition))throw std::runtime_error("Both only supports items shared by T and CT");
    c.team="T";auto ct=c;ct.team="CT";return {c,ct};
}
inline void remember_targets(std::vector<Cosmetic>& profile,const Cosmetic& c,bool both) {
    const auto targets=selection_targets(c,both);auto next=profile;
    for(const auto& target:targets) {
        auto found=std::find_if(next.begin(),next.end(),[&](const auto& old){
            return old.team==target.team && old.category==target.category && (target.category!="weapon" || old.definition==target.definition);
        });
        if(found==next.end())next.push_back(target);else *found=target;
    }
    profile.swap(next);
}
}
