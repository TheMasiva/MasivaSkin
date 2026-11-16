#pragma once
#include "catalog.hpp"
#include "extras_catalog.hpp"
#include "shared_selection.hpp"
#include <vector>
namespace masiva {
inline std::vector<const catalog::Finish*> gallery_entries(int group,int team,bool both,int model) {
    std::vector<const catalog::Finish*> result;
    for(const auto& f:catalog::finishes) {
        if(group==3) {
            if(!both && catalog::agent(f.definition) && extras::buyable(f.definition,team))result.push_back(&f);
        } else if(f.definition==model && (both?shared_item(f.definition):extras::buyable(f.definition,team)))result.push_back(&f);
    }
    return result;
}
}
