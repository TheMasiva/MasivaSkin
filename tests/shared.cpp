#include "shared_selection.hpp"
#include "profile_packet.hpp"
#include "ui_palette.hpp"
#include <iostream>
using namespace masiva;
void check(bool value,const char* why){if(!value)throw std::runtime_error(why);}
template<class F> bool rejects(F f){try{f();}catch(const std::exception&){return true;}return false;}
int main(){
    for(int d:{1,2,9,24,31,36,40,63,64,500,5030})check(shared_item(d),"common item omitted");
    for(int d:{7,16,60,61,4})check(!shared_item(d),"team-exclusive item included");
    for(const auto& a:extras::agents)check(!shared_item(a.id),"agent included in Both");
    Cosmetic c{"T","weapon",1,0,123,0.01f,17,"Hibiki"};
    c.decorations.stickers[0]=extras::stickers[1].id;c.decorations.charm=extras::charms[1].id;
    const auto targets=selection_targets(c,true);
    check(targets.size()==2 && targets[0].team=="T" && targets[1].team=="CT","two real team records");
    Cosmetic ak{"T","weapon",7,474,42,0.01f};
    auto t=extras::agents[0];
    Cosmetic agent{t.team==2?"T":"CT","agent",t.id,0,0,0};
    std::vector<Cosmetic> profile{ak,agent};remember_targets(profile,c,true);
    c.seed=999;remember_targets(profile,c,true);
    check(profile.size()==4 && profile[0].definition==7 && profile[1].category=="agent","preserve unrelated profile slots");
    std::ostringstream out;write(out,profile);std::istringstream in(out.str());
    Selections choices;choices.set_all(decode_profile(encode_profile(read(in))));
    for(int team:{2,3}){
        auto found=choices.get(team,1);
        check(found && found->seed==999 && found->stattrak==17 && found->custom_name=="Hibiki" && found->decorations==c.decorations,"Both persisted every option");
    }
    check(rejects([&]{remember_targets(profile,ak,true);}) && profile.size()==4,"exclusive request must not partially mutate");
    check(rejects([&]{selection_targets(agent,true);}),"agent Both request rejected");
    choices.set_all(selection_targets(c,true));
    check(choices.get(2,7) && choices.get(t.team,t.id),"Both queue preserves other choices");
    choices.clear(2,1);choices.clear(3,1);
    check(!choices.get(2,1) && !choices.get(3,1) && choices.get(2,7),"Both restore isolates slot");
    check(ui::palette(false,0).background==ui::rgb(7,8,11) && ui::palette(false,0).panel==ui::rgb(14,16,21),"layered black theme surfaces");
    for(unsigned r:{0U,63U,127U,191U,255U})for(unsigned g:{0U,63U,127U,191U,255U})for(unsigned b:{0U,63U,127U,191U,255U}) {
        const auto base=ui::rgb(r,g,b);const auto p=ui::custom_palette(base,ui::rgb(137,66,230));
        check(p.background==base,"custom main colour must be exact");
        for(auto surface:{p.background,p.panel,p.card,p.button}){auto a=ui::luminance(surface),z=ui::luminance(p.text);check((std::max(a,z)+0.05)/(std::min(a,z)+0.05)>=4.49,"custom colour text contrast");}
    }
    for(bool light:{false,true})for(auto accent:{ui::rgb(0,0,0),ui::rgb(255,255,255),ui::rgb(255,255,0),ui::rgb(0,0,255),ui::rgb(132,92,236)}){
        auto p=ui::palette(light,accent);
        for(auto bg:{p.accent,p.button}){
            double a=ui::luminance(bg),b=ui::luminance(ui::ink(bg));
            check((std::max(a,b)+.05)/(std::min(a,b)+.05)>=4.5,"button caption contrast");
        }
        check(p.text!=p.background && p.accent==accent,"palette preserves accent and text");
    }
    std::cout<<"Both filters, atomic profile/IPC persistence, isolated restore and theme contrast passed\n";
}
