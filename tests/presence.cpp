#include "presence_state.hpp"
#include <stdexcept>
#include <iostream>
using namespace masiva::presence;
void check(bool x){if(!x)throw std::runtime_error("presence regression");}
int main(){
    auto label=[](const char* s){return details(Reader(s).parse());};
    check(label(R"({"map":{"name":"de_mirage","mode":"competitive"},"player":{"activity":"playing"}})")=="Competitive | de_mirage");
    check(label(R"({"previously":{"map":{"name":"de_dust2"}},"player":{"activity":"menu"}})")=="In the lobby");
    check(label(R"({"map":{"mode":"scrimcomp2v2","name":"de_nuke"},"player":{"activity":"menu"}})")=="Wingman | de_nuke");
    check(label(R"({"map":{"mode":"casual","name":"de_anubis"},"ignored":[1,false,{"player":{"activity":"menu"}}]})")=="Casual | de_anubis");
    check(label(R"({"map":{"name":"bad\"name","mode":"competitive"}})")=="In Counter-Strike 2");
    check(Reader(R"({"cmd":"DISPATCH","evt":"READY","data":{"evt":"ERROR"}})").parse().event=="READY");
    check(Reader(R"({"cmd":"SET_ACTIVITY","data":{},"evt":null})").parse().event.empty());
    bool failed=false;try{Reader("{").parse();}catch(const std::invalid_argument&){failed=true;}check(failed);
    std::cout<<"Presence states passed\n";
}
