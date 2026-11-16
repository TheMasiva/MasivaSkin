#include "team_preview_policy.hpp"
#include "selections.hpp"
#include "collection_tools.hpp"
#include <stdexcept>
#include <iostream>
using namespace masiva;
void require(bool v){if(!v)throw std::runtime_error("Team preview policy regression");}
int main(){
    using namespace preview_policy;
    // Unknown owner, other player, closed scene and spectator must never mutate.
    require(!local_character(0,0,2,true));
    require(!local_character(11,12,2,true));
    require(!local_character(11,11,1,true));
    require(!local_character(11,11,2,false));
    require(local_character(11,11,2,true)&&local_character(11,11,3,true));
    require(should_update(false,false,false,false));
    require(!should_update(true,true,true,true));
    require(!should_update(true,true,false,false));
    require(should_update(true,true,false,true));
    require(should_update(true,false,true,false));
    // Fast callbacks must not keep postponing network overwrite detection.
    std::uint64_t deadline=0;unsigned checks=0;
    for(std::uint64_t now=0;now<1000;now+=10) {
        if(needs_item_read(true,true,now>=deadline))++checks;
        advance_verification(deadline,now);
    }
    require(checks==4);
    require(needs_item_read(false,true,false));
    require(needs_item_read(true,false,false)); // user edits remain immediate
    require(!needs_item_read(true,true,false));
    // A slow callback still performs one check, without a catch-up loop.
    require(needs_item_read(true,true,2000>=deadline));
    advance_verification(deadline,2000);require(deadline==2250);
    Selections s;Cosmetic t{"T","weapon",7,0,0,0.01f};
    Cosmetic ct{"CT","weapon",1,0,0,0.01f};
    s.set(t);s.set(ct);
    require(s.get(2,7)&&!s.get(3,7));
    require(s.get(3,1)&&!s.get(2,1));
    auto changed=t;changed.seed=1;require(!same_cosmetic(t,changed));
    changed=t;changed.custom_name="Asuna";require(!same_cosmetic(t,changed));
    changed=t;changed.stattrak=4;require(!same_cosmetic(t,changed));
    s.clear();require(!s.get(2,7)&&!s.get(3,1));
    // A newly queued choice after a reset is independent of the old selection.
    s.set(changed);require(s.get(2,7)->stattrak==4);
    std::cout<<"Local ownership, team scope, idle reuse, network reset and restore selection checks passed\n";
}
