#pragma once
#include <algorithm>
#include <cstddef>
namespace masiva::menu {
inline constexpr unsigned default_key=0x2d; // Insert
inline constexpr std::size_t page_size=6;
struct CardRect {
    int x,y,w,h;
    constexpr bool contains(int a,int b)const{return a>=x && a<x+w && b>=y && b<y+h;}
};
inline constexpr CardRect card_rect(std::size_t card) {
    return {184+static_cast<int>(card%2)*328,188+static_cast<int>(card/2)*148,312,136};
}
inline constexpr CardRect page_button_rect(bool next) { return {next?786:184,644,38,30}; }
inline bool valid_key(unsigned k) {
    return (k>=0x30 && k<=0x5a && !(k>=0x3a && k<=0x40)) ||
        (k>=0x60 && k<=0x87) || (k>=0x21 && k<=0x28) ||
        k==0x2d || k==0x2e || k==0x13 || k==0x91 ||
        (k>=0xba && k<=0xc0) || (k>=0xdb && k<=0xde);
}
inline bool text_key(unsigned k){return (k>=0x30 && k<=0x6f) || k>=0xba;}
inline bool toggle_allowed(bool game,bool own,bool modal,bool editing,bool modifiers,unsigned k) {
    return !modal && !modifiers && (game || own) && !(own && editing && text_key(k));
}
inline std::size_t pages(std::size_t count){return std::max<std::size_t>(1,(count+page_size-1)/page_size);}
inline std::size_t clamp_page(std::size_t page,std::size_t count){return std::min(page,pages(count)-1);}
inline std::size_t card_index(std::size_t page,std::size_t card,std::size_t count) {
    const auto i=clamp_page(page,count)*page_size+card;
    return card<page_size && i<count?i:count;
}
}
