#include "runtime_policy.hpp"
#include "catalog.hpp"
#include "cosmetic_refresh.hpp"
#include <iostream>
#include <stdexcept>
using namespace masiva;
void check(bool v,const char* message){if(!v)throw std::runtime_error(message);}
int main(){
    for(const auto& f:catalog::finishes) {
        check(catalog::finish(f.definition,f.paint)==&f,"lookup retains the exact catalog entry");
        check(!catalog::finish(f.definition,-1),"missing finish cannot match an occupied hash slot");
    }
    check(!catalog::finish(-1,0),"unknown model excluded");
    HudWriteMemo stable;unsigned writes=0;
    for(std::uint64_t now=0;now<600000;now+=100)if(stable.needs("same validated row",now)){
        stable.committed("same validated row",now);++writes;
    }
    check(writes==300,"ten minute stable HUD has 300 rather than 6000 writes");
    HudWriteMemo memo;memo.committed("spawn1:knife500:panel10:EN:name",0);
    check(!memo.needs("spawn1:knife500:panel10:EN:name",1999),"stable row does not repeatedly notify Panorama");
    check(memo.needs("spawn1:knife500:panel10:EN:name",2000),"periodic correction remains available after a native overwrite");
    for(auto changed:{"spawn2:knife500:panel10:EN:name","spawn1:knife507:panel10:EN:name",
                      "spawn1:knife500:panel11:EN:name","spawn1:knife500:panel10:TR:name",
                      "spawn1:knife500:panel10:EN:custom"})
        check(memo.needs(changed,100),"spawn/model/panel/language/name changes bypass throttle");
    // Simulate the report: a previously ready model disappears, controller still
    // caches the old handle; producer cannot recreate until a request is armed.
    ModelSettle recover(0);bool world=false,force=false;unsigned requests=0;
    for(std::uint64_t now=0;now<5000;now+=100)if(recover.due(now)){
        if(recover.world_due(world,now)){
            if(restart_missing_world(world,recover.world_attempts())){force=true;++requests;}
            if(force){world=true;force=false;}
        }
    }
    check(world && requests==1,"lost world is rearmed once then allowed to load");
    ModelSettle failed(0);unsigned forced=0;
    for(std::uint64_t now=0;now<10000;now+=10)if(failed.due(now) && failed.world_due(false,now))
        if(restart_missing_world(false,failed.world_attempts()))++forced;
    check(forced==3,"failed world creation has bounded force requests");
    ModelSettle pending(0);
    for(std::uint64_t now=0;now<5000;now+=100)
        check(!pending.world_due(true,now),"existing pending resource is never restarted");
    std::cout<<"Stable HUD simulation: 6000 -> "<<writes<<" writes (95% fewer API submissions, not measured FPS).\n";
    std::cout<<"Changed rows and bounded missing-world recovery passed.\n";
}
