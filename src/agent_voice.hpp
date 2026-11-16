#pragma once
#include "voice_catalog.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <string_view>
namespace masiva {
inline bool voice_equal(std::string_view a,std::string_view b) {
    if(a.size()!=b.size())return false;
    for(std::size_t i=0;i<a.size();++i) {
        const auto c=a[i]>='A' && a[i]<='Z'?a[i]+('a'-'A'):a[i];
        const auto d=b[i]>='A' && b[i]<='Z'?b[i]+('a'-'A'):b[i];
        if(c!=d)return false;
    }
    return true;
}
inline bool known_voice_bank(std::string_view bank) {
    for(const auto& b:voice_data::banks)if(bank==b.name)return true;
    return false;
}
// These two native body reactions have shared male/female banks, not an
// individual recording for each agent. Do not remap footsteps or impacts.
inline bool female_voice_bank(std::string_view bank) {
    return bank=="professional_fem" || bank=="swat_fem" || bank=="seal_fem" ||
        bank=="gendarmerie_fem" || bank=="gendarmerie_fem_epic" ||
        bank=="jungle_fem" || bank=="jungle_fem_epic";
}
inline const char* body_voice_event(std::string_view event,std::string_view bank) {
    if(!known_voice_bank(bank))return nullptr;
    const char* selected=nullptr;
    if(voice_equal(event,"Player.DamageFall") || voice_equal(event,"Player.DamageFall.Fem"))
        selected=female_voice_bank(bank)?"Player.DamageFall.Fem":"Player.DamageFall";
    else if(voice_equal(event,"BaseGrenade.JumpThrowM") || voice_equal(event,"BaseGrenade.JumpThrowF"))
        selected=female_voice_bank(bank)?"BaseGrenade.JumpThrowF":"BaseGrenade.JumpThrowM";
    return selected && !voice_equal(event,selected)?selected:nullptr;
}
inline bool agent_voice_event(std::string_view event) {
    const auto dot=event.find('.');
    if(dot==std::string_view::npos)return false;
    for(const auto& b:voice_data::banks)if(voice_equal(event.substr(0,dot),b.name))return true;
    return voice_equal(event,"Player.DamageFall") || voice_equal(event,"Player.DamageFall.Fem") ||
        voice_equal(event,"BaseGrenade.JumpThrowM") || voice_equal(event,"BaseGrenade.JumpThrowF");
}
// Fixed-size, allocation-free lookup; only known agent prefixes qualify.
inline const voice_data::Group* voice_group(std::string_view event,std::string_view target) {
    if(event.empty() || event.size()>190 || target.empty() || target.size()>48)return nullptr;
    std::array<char,192> text{};
    for(std::size_t i=0;i<event.size();++i)text[i]=(event[i]>='A' && event[i]<='Z')?event[i]+('a'-'A'):event[i];
    const std::string_view lower(text.data(),event.size());
    const auto dot=lower.find('.');if(dot==std::string_view::npos)return nullptr;
    const auto bank=lower.substr(0,dot);
    bool known=false;std::string_view actor_prefix;
    for(const auto& b:voice_data::banks)if(bank==b.name){known=true;actor_prefix=b.prefix;break;}
    if(!known || bank==target)return nullptr;
    auto stem=lower.substr(dot+1);
    if(!actor_prefix.empty() && stem.substr(0,actor_prefix.size())==actor_prefix)stem.remove_prefix(actor_prefix.size());
    // A few banks have a letter after the numbered take.
    if(stem.size()>1 && stem.back()>='a' && stem.back()<='z' && stem[stem.size()-2]>='0' && stem[stem.size()-2]<='9')stem.remove_suffix(1);
    const auto before=stem.size();
    while(!stem.empty() && stem.back()>='0' && stem.back()<='9')stem.remove_suffix(1);
    if(stem.size()!=before && !stem.empty() && stem.back()=='_')stem.remove_suffix(1);
    for(const auto& a:voice_data::aliases)if(stem==a.from){stem=a.to;break;}
    std::array<char,256> key{};
    if(target.size()+1+stem.size()>=key.size())return nullptr;
    auto end=std::copy(target.begin(),target.end(),key.begin());*end++='.';std::copy(stem.begin(),stem.end(),end);
    const std::string_view wanted(key.data(),target.size()+1+stem.size());
    const auto begin=std::begin(voice_data::groups),last=std::end(voice_data::groups);
    const auto it=std::lower_bound(begin,last,wanted,[](const auto& a,std::string_view b){return std::string_view(a.key)<b;});
    return it!=last && wanted==it->key?it:nullptr;
}
inline bool local_voice_source(std::int32_t source,std::uint32_t handle) {
    return handle!=0xffffffffU && (handle&0x7fffU)!=0 && source>0 && static_cast<std::uint32_t>(source)==(handle&0x7fffU);
}
}
