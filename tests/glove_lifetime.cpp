#include "game.hpp"
#include "glove_lifetime.hpp"
#include <iostream>
using namespace masiva;
struct Backup {WeaponState state;float spawn;int original;};
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(){
    WeaponState s{};s.pawn=0x10000;s.weapon=s.pawn;s.identity=0x20000;s.item=s.pawn+0x17a0;s.handle=123;s.team=2;
    std::vector<Backup> saved;
    for(int life=1;life<=1000;++life){
        // Exercise reused pawn addresses, serial changes and side swaps.
        if(life%3==0)++s.handle;
        s.team=life%2?2:3;
        retain_glove_lifetime(saved,s,static_cast<float>(life));
        require(saved.empty(),"expired lifetime must never consume capacity");
        saved.push_back({s,static_cast<float>(life),life});
        for(int edit=0;edit<8;++edit){
            s.definition=static_cast<std::uint16_t>(5030+edit%2);
            require(retain_glove_lifetime(saved,s,static_cast<float>(life))==0,"same-life skin edits retain first backup");
            require(saved.size()==1 && saved[0].original==life,"original restore state preserved");
        }
    }
    auto old=s;++s.identity;
    require(retain_glove_lifetime(saved,s,1000)==1,"different entity identity cannot reuse backup");
    saved.push_back({old,1000,71});saved.push_back({s,1000,72});
    require(retain_glove_lifetime(saved,s,1000)==1 && saved[0].original==72,"prune historical records while retaining active restore");
    bool rejected=false;try{retain_glove_lifetime(saved,s,NAN);}catch(const std::exception&){rejected=true;}
    require(rejected && saved.size()==1 && saved[0].original==72,"invalid snapshot does not discard current backup");
    std::cout<<"1000 glove lifetimes, same-life edits, reused addresses, side swaps and restore retention passed\n";
}
