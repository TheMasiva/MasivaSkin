#pragma once
#include <cmath>
#include <array>
#include "extras_catalog.hpp"
#include <iomanip>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <sstream>
#include <locale>
#include <string>
#include <vector>

namespace masiva {
struct Decorations {
    std::array<int,5> stickers{};
    int charm{},charm_seed{};
    float x{},y{},z{};
    bool operator==(const Decorations& b) const {return stickers==b.stickers && charm==b.charm && charm_seed==b.charm_seed && x==b.x && y==b.y && z==b.z;}
    bool operator!=(const Decorations& b) const {return !(*this==b);}
};
struct Cosmetic {
    std::string team, category;
    int definition{}, paint{}, seed{};
    float wear{};
    int stattrak{-1}; // -1 disabled; otherwise a local display counter
    std::string custom_name{};
    Decorations decorations{};
};
inline void validate(const Cosmetic& c) {
    for(int id:c.decorations.stickers)if(!extras::contains(extras::stickers,id))throw std::runtime_error("Unknown sticker");
    if(!extras::contains(extras::charms,c.decorations.charm) || c.decorations.charm_seed<0 || c.decorations.charm_seed>100000)
        throw std::runtime_error("Invalid charm or charm seed");
    for(float value:{c.decorations.x,c.decorations.y,c.decorations.z})
        if(!std::isfinite(value) || std::abs(value)>100)throw std::runtime_error("Charm position must be -100..100");
    if(c.category!="weapon" && c.decorations!=Decorations{})throw std::runtime_error("Stickers and charms require a firearm");
    if(c.category=="agent") {
        const auto a=extras::agent(c.definition);
        if(!a || a->team!=(c.team=="T"?2:3) || c.paint!=0 || c.stattrak!=-1 || !c.custom_name.empty())
            throw std::runtime_error("Invalid agent/team selection");
    }
    if(c.stattrak < -1 || c.stattrak > 999999)throw std::runtime_error("StatTrak must be 0..999999 or disabled");
    if(c.custom_name.size()>160 || c.custom_name.find_first_of("\r\n")!=std::string::npos || c.custom_name.find('\0')!=std::string::npos)
        throw std::runtime_error("Name tag must be a single line of at most 160 UTF-8 bytes");
    if(c.category=="gloves" && (c.stattrak!=-1 || !c.custom_name.empty()))
        throw std::runtime_error("Gloves do not support StatTrak or name tags");
    if (c.team != "T" && c.team != "CT") throw std::runtime_error("Team must be T or CT");
    if (c.category != "weapon" && c.category != "knife" && c.category != "gloves" && c.category != "agent")
        throw std::runtime_error("Unknown cosmetic category");
    if (c.definition <= 0 || c.definition > 65535 || c.paint < 0)
        throw std::runtime_error("Invalid definition or paint identifier");
    if (c.seed < 0 || c.seed > 1000 || !std::isfinite(c.wear) || c.wear < 0 || c.wear > 1)
        throw std::runtime_error("Seed or wear out of range");
}
inline std::vector<Cosmetic> read(std::istream& in) {
    std::string magic;
    int version{};
    std::size_t count{};
    if (!(in >> magic >> version >> count) || magic != "MASIVA_SKIN" || (version != 1 && version != 2 && version != 3) || count > 512)
        throw std::runtime_error("Invalid or unsupported profile header");
    std::vector<Cosmetic> result;
    for (std::size_t i = 0; i < count; ++i) {
        Cosmetic c;
        if (!(in >> c.team >> c.category >> c.definition >> c.paint >> c.seed >> c.wear))
            throw std::runtime_error("Truncated or malformed profile");
        if(version>=2 && !(in >> c.stattrak >> std::quoted(c.custom_name)))
            throw std::runtime_error("Malformed StatTrak/name tag profile fields");
        if(version==3) {
            for(auto& id:c.decorations.stickers)if(!(in>>id))throw std::runtime_error("Malformed sticker profile");
            auto& d=c.decorations;
            if(!(in>>d.charm>>d.charm_seed>>d.x>>d.y>>d.z))throw std::runtime_error("Malformed charm profile");
        }
        validate(c);
        for (const auto& old : result)
            if (old.team == c.team && ((c.category != "weapon" && old.category == c.category) ||
                (old.category == c.category && old.definition == c.definition)))
                throw std::runtime_error("Duplicate loadout slot");
        result.push_back(c);
    }
    in >> std::ws;
    if (!in.eof()) throw std::runtime_error("Unexpected trailing profile data");
    return result;
}
inline void write(std::ostream& out, const std::vector<Cosmetic>& items) {
    std::ostringstream buffer;
    buffer.imbue(std::locale::classic());
    buffer << "MASIVA_SKIN 3 " << items.size() << '\n';
    buffer << std::setprecision(9);
    for (const auto& c : items) {
        validate(c);
        buffer << c.team << ' ' << c.category << ' ' << c.definition << ' ' << c.paint
            << ' ' << c.seed << ' ' << c.wear << ' ' << c.stattrak << ' ' << std::quoted(c.custom_name);
        for(int id:c.decorations.stickers)buffer<<' '<<id;
        const auto& d=c.decorations;buffer<<' '<<d.charm<<' '<<d.charm_seed<<' '<<d.x<<' '<<d.y<<' '<<d.z<<'\n';
    }
    std::istringstream check(buffer.str());
    check.imbue(std::locale::classic());
    read(check);
    out << buffer.str();
    if (!out) throw std::runtime_error("Profile write failed");
}
}
