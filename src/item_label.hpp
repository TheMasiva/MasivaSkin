#pragma once
#include "localized_catalog.hpp"
#include <array>
#include <string>
#include <stdexcept>
namespace masiva {
using ItemLabel=std::array<char,161>;
inline ItemLabel item_label(int definition,int paint,bool english=true) {
    auto w=catalog::weapon(definition);auto f=catalog::finish(definition,paint);
    if(!w || !f)throw std::runtime_error("Gecersiz gorunum adi.");
    std::string label=localized::weapon(definition,english);
    if(paint)label+=" | "+std::string(localized::finish(definition,paint,english));
    ItemLabel out{};std::size_t n=label.size();
    if(n>160) { n=160;while(n && (static_cast<unsigned char>(label[n])&0xc0)==0x80)--n; }
    label.copy(out.data(),n);return out;
}
// Menu labels retain phase details; HUD labels follow the finish family.
inline ItemLabel hud_item_label(int definition,int paint,bool english,int stattrak,const ItemLabel& custom={}) {
    if(custom[0])return custom;
    auto f=catalog::finish(definition,paint);
    if(!catalog::weapon(definition) || !f)throw std::runtime_error("Invalid HUD item");
    std::string finish=localized::finish(definition,paint,english);
    if(catalog::knife(definition)) {
        const std::string canonical=f->name;
        if(canonical.rfind("Gamma Doppler",0)==0)finish="Gamma Doppler";
        else if(canonical.rfind("Doppler",0)==0)finish="Doppler";
    }
    std::string label=catalog::knife(definition)?u8"★ ":"";
    if(stattrak>=0)label+=u8"StatTrak™ ";
    label+=localized::weapon(definition,english);
    if(paint)label+=" | "+finish;
    ItemLabel out{};auto n=std::min(label.size(),out.size()-1);
    if(n<label.size())while(n && (static_cast<unsigned char>(label[n])&0xc0)==0x80)--n;
    label.copy(out.data(),n);return out;
}
}
