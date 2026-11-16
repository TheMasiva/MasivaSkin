#pragma once
#include "loadout.hpp"
#include <optional>
#include <cstdint>
#include <cstring>
namespace masiva {
struct DecorationAttribute {std::uint16_t id;const char* name;};
inline constexpr DecorationAttribute decoration_attributes[] = {
{113,"sticker slot 0 id"},
{114,"sticker slot 0 wear"},
{115,"sticker slot 0 scale"},
{116,"sticker slot 0 rotation"},
{117,"sticker slot 1 id"},
{118,"sticker slot 1 wear"},
{119,"sticker slot 1 scale"},
{120,"sticker slot 1 rotation"},
{121,"sticker slot 2 id"},
{122,"sticker slot 2 wear"},
{123,"sticker slot 2 scale"},
{124,"sticker slot 2 rotation"},
{125,"sticker slot 3 id"},
{126,"sticker slot 3 wear"},
{127,"sticker slot 3 scale"},
{128,"sticker slot 3 rotation"},
{129,"sticker slot 4 id"},
{130,"sticker slot 4 wear"},
{131,"sticker slot 4 scale"},
{132,"sticker slot 4 rotation"},
{299,"keychain slot 0 id"},
{300,"keychain slot 0 offset x"},
{301,"keychain slot 0 offset y"},
{302,"keychain slot 0 offset z"},
{306,"keychain slot 0 seed"},
};
using DecorationState=std::array<std::optional<std::uint32_t>,25>;
inline std::uint32_t float_bits(float value) {std::uint32_t result;std::memcpy(&result,&value,4);return result;}
inline DecorationState decoration_values(const Decorations& d) {
    DecorationState state{};
    // Missing dynamic IDs fall back to the inventory item's attributes.
    // Explicit zero suppresses that fallback; nullopt remains meaningful when
    // restoring the captured original dynamic attribute list.
    for(unsigned i=0;i<5;++i) {
        state[i*4]=static_cast<std::uint32_t>(d.stickers[i]);
        if(!d.stickers[i])continue;
        state[i*4+1]=float_bits(0.f);state[i*4+2]=float_bits(1.f);state[i*4+3]=float_bits(0.f);
    }
    state[20]=static_cast<std::uint32_t>(d.charm);
    if(d.charm) {
        state[21]=float_bits(d.x);state[22]=float_bits(d.y);state[23]=float_bits(d.z);
        state[24]=static_cast<std::uint32_t>(d.charm_seed);
    }
    return state;
}
}
