#pragma once
#include "game.hpp"
#include "cosmetic_refresh.hpp"
namespace masiva {
struct StatAttributes { std::optional<std::uint32_t> count, type; };
inline std::optional<std::uint32_t> read_stat_attribute(const Memory& memory,Address item,std::uint16_t id) {
    const auto count=memory.get<int>(item+0x210);
    if(count<0 || count>256)throw std::runtime_error("Attribute list count invalid");
    if(!count)return {};
    const auto data=pointer(memory.get<Address>(item+0x218));
    std::optional<std::uint32_t> result;
    for(int i=0;i<count;++i) {
        const auto entry=data+static_cast<Address>(i)*0x48;
        if(memory.get<std::uint16_t>(entry+0x30)!=id)continue;
        if(result || memory.get<std::uint8_t>(entry+0x40))
            throw std::runtime_error("Ambiguous StatTrak attribute");
        result=memory.get<std::uint32_t>(entry+0x34);
    }
    return result;
}
// Native setters/removers own allocation, compaction, notifications and cache
// invalidation. Never replace the whole list or disturb other attributes.
template<class Set,class Remove>
void write_stat_attributes(const Memory& memory,Address item,const StatAttributes& values,Set set,Remove remove) {
    const auto old_count=read_stat_attribute(memory,item,80);
    const auto old_type=read_stat_attribute(memory,item,81);
    const auto write=[&](std::uint16_t id,const char* name,std::optional<std::uint32_t> value,
                         std::optional<std::uint32_t> old) {
        if(value!=old) {
            if(value)set(item,name,integer_attribute(*value));
            else if(old)remove(item,id);
        }
        if(read_stat_attribute(memory,item,id)!=value)
            throw std::runtime_error("StatTrak attribute readback mismatch");
    };
    write(80,"kill eater",values.count,old_count);
    write(81,"kill eater score type",values.type,old_type);
}
}
