#pragma once
#include "selections.hpp"
#include <array>
#include <cstdint>
#include <algorithm>
namespace masiva {
struct ProfileEntry {
    std::uint32_t team{},definition{};
    int paint{},seed{},stattrak{-1};
    float wear{};
    std::array<char,161> name{};
    Decorations decorations{};
};
struct ProfilePacket {
    std::uint32_t count{};
    std::array<ProfileEntry,512> items{};
};
inline std::vector<Cosmetic> decode_profile(const ProfilePacket& packet) {
    if(packet.count>packet.items.size())throw std::runtime_error("Profile packet too large");
    std::vector<Cosmetic> items;items.reserve(packet.count);
    for(std::uint32_t i=0;i<packet.count;++i) {
        const auto& e=packet.items[i];
        if(e.team!=2 && e.team!=3)throw std::runtime_error("Invalid profile team");
        const auto end=std::find(e.name.begin(),e.name.end(),'\0');
        if(end==e.name.end())throw std::runtime_error("Unterminated profile name");
        items.push_back({e.team==2?"T":"CT",catalog::category(static_cast<int>(e.definition)),
            static_cast<int>(e.definition),e.paint,e.seed,e.wear,e.stattrak,std::string(e.name.begin(),end),e.decorations});
    }
    Selections checked;checked.set_all(items);return items;
}
inline ProfilePacket encode_profile(const std::vector<Cosmetic>& items) {
    Selections checked;checked.set_all(items);
    ProfilePacket packet;
    if(items.size()>packet.items.size())throw std::runtime_error("Profile too large");
    packet.count=static_cast<std::uint32_t>(items.size());
    for(std::size_t i=0;i<items.size();++i) {
        const auto& c=items[i];auto& e=packet.items[i];
        e.team=c.team=="T"?2:3;e.definition=c.definition;e.paint=c.paint;e.seed=c.seed;e.wear=c.wear;e.stattrak=c.stattrak;e.decorations=c.decorations;
        std::copy(c.custom_name.begin(),c.custom_name.end(),e.name.begin());
    }
    return packet;
}
}
