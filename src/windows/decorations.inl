DecorationState read_decorations(Address item,StatAttributes* stat=nullptr) {
    DecorationState out{};
    const auto count=memory.get<int>(item+0x210);
    if(count<0 || count>256)throw std::runtime_error("Decoration attribute count invalid");
    if(!count)return out;
    const auto data=pointer(memory.get<Address>(item+0x218));
    std::vector<std::array<std::uint8_t,72>> entries(static_cast<std::size_t>(count));
    memory.read_bytes(data,entries.data(),entries.size()*72);
    for(const auto& entry:entries) {
        std::uint16_t id{};std::memcpy(&id,entry.data()+0x30,2);
        if(stat && (id==80 || id==81)) {
            auto& slot=id==80?stat->count:stat->type;
            if(slot || entry[0x40])throw std::runtime_error("Ambiguous StatTrak attribute");
            std::uint32_t bits{};
            std::memcpy(&bits,entry.data()+0x34,sizeof(bits));
            slot=bits;
            continue;
        }
        for(std::size_t n=0;n<out.size();++n)if(decoration_attributes[n].id==id) {
            if(out[n] || entry[0x40])throw std::runtime_error("Ambiguous decoration attribute");
            std::uint32_t bits{};std::memcpy(&bits,entry.data()+0x34,4);out[n]=bits;
        }
    }
    return out;
}
void write_decorations(Address item,const DecorationState& desired) {
    const auto old=read_decorations(item);
    auto write=[&](const DecorationState& values) {
        const auto current=read_decorations(item);
        for(std::size_t i=0;i<values.size();++i)if(values[i]!=current[i]) {
            const auto& a=decoration_attributes[i];
            if(values[i])function<void(*)(Address,const char*,float)>(binding::set_attribute)(item,a.name,integer_attribute(*values[i]));
            else function<void(*)(Address,std::uint16_t)>(binding::remove_attribute)(item,a.id);
        }
    };
    try {write(desired);if(read_decorations(item)!=desired)throw std::runtime_error("Sticker/charm attribute readback mismatch");}
    catch(...) {write(old);throw;}
}
