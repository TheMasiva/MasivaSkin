#include "bridge_startup.hpp"
#include <stdexcept>
#include <iostream>
using namespace masiva::startup;
void check(bool ok){if(!ok)throw std::runtime_error("startup lifecycle regression");}
int main(){
    // An unfinished loader must retain its path and cannot be called a load failure.
    check(observe(false,0,false,false)==State::loading);
    check(observe(false,0,false,true)==State::loading);
    // Loader completion and initializer completion are independent.
    check(observe(true,0,true,true)==State::initializing);
    check(observe(true,0,false,true)==State::initializing);
    check(observe(true,1,false,true)==State::ready);
    check(observe(true,1,true,true)==State::ready);
    check(observe(true,-1,true,true)==State::rejected);
    // A DWORD loader result is not a pointer; module presence decides this branch.
    check(observe(false,0,true,true)==State::missing_channel);
    check(observe(false,0,true,false)==State::load_failed);
    std::cout<<"9 startup lifecycle cases passed\n";
}
