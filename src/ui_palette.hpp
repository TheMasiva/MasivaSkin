#pragma once
#include <initializer_list>
#include <cstdint>
#include <cmath>
namespace masiva::ui {
using Color=std::uint32_t;
inline constexpr Color rgb(unsigned r,unsigned g,unsigned b){return r|(g<<8)|(b<<16);}
inline Color mix(Color a,Color b,unsigned amount) {
    Color result=0;for(unsigned s:{0U,8U,16U})result|=((((a>>s)&255)*(100-amount)+((b>>s)&255)*amount)/100)<<s;
    return result;
}
inline double luminance(Color c) {
    auto linear=[](double v){v/=255;return v<=0.04045?v/12.92:std::pow((v+0.055)/1.055,2.4);};
    return 0.2126*linear(c&255)+0.7152*linear((c>>8)&255)+0.0722*linear((c>>16)&255);
}
inline Color ink(Color c){return luminance(c)>0.179?rgb(0,0,0):rgb(255,255,255);}
struct Palette {Color background,panel,text,muted,card,selected,button,disabled,accent;};
inline Palette palette(bool light,Color accent) {
    const Color bg=light?rgb(240,242,247):rgb(7,8,11),surface=light?rgb(255,255,255):rgb(14,16,21);
    return {bg,surface,light?rgb(24,28,38):rgb(235,234,245),light?rgb(85,90,105):rgb(160,158,179),
        light?rgb(233,236,243):rgb(15,15,15),mix(surface,accent,light?15:24),
        light?rgb(223,227,236):rgb(24,24,24),light?rgb(230,232,237):rgb(14,14,14),accent};
}
inline Palette custom_palette(Color base,Color accent){
 const bool light=luminance(base)>0.179;const Color foreground=ink(base);
 auto surface=[&](unsigned amount){auto c=mix(base,rgb(255,255,255),amount);if(!light&&luminance(c)>0.179)c=mix(base,rgb(0,0,0),amount);return c;};
 return {base,surface(5),foreground,mix(base,foreground,72),surface(9),mix(base,accent,20),surface(13),surface(3),accent};
}

}
