#pragma once
#include "game.hpp"
#include "windows/binding.hpp"
#include "windows/engine_binding.hpp"
namespace masiva {
inline void verify_module_identity(const Memory& memory,Address base,std::uint32_t stamp,std::uint32_t size,const char* name) {
    const auto fail=[&]{throw std::runtime_error(std::string(name)+" binary mismatch; game update requires a matching Masiva release.");};
    if(memory.get<std::uint16_t>(base)!=0x5a4d)fail();
    const auto offset=memory.get<std::uint32_t>(base+0x3c);
    if(offset<0x40 || offset>0x1000)fail();
    const auto nt=base+offset;
    if(memory.get<std::uint32_t>(nt)!=0x4550 || memory.get<std::uint16_t>(nt+4)!=0x8664 ||
       memory.get<std::uint32_t>(nt+8)!=stamp || memory.get<std::uint16_t>(nt+24)!=0x20b ||
       memory.get<std::uint32_t>(nt+80)!=size)fail();
}
template<class Codes> inline void verify_module_code(const Memory& memory,Address base,const Codes& codes,const char* name) {
    for(const auto& code:codes) {
        std::array<std::uint8_t,32> actual{};memory.read_bytes(base+code.rva,actual.data(),actual.size());
        if(actual!=code.bytes)throw std::runtime_error(std::string(name)+" function signature mismatch; no game changes made.");
    }
}
inline int verified_patch_version(const Memory& memory,Address client,Address engine) {
    // A numeric patch version alone can be reused across different DLLs.
    // Admit only the supplied client/engine pair, then pin the loaded patch version for this session.
    verify_module_identity(memory,client,binding::timestamp,binding::image_size,"client.dll");
    verify_module_code(memory,client,binding::required,"client.dll");
    verify_module_identity(memory,engine,engine_binding::timestamp,engine_binding::image_size,"engine2.dll");
    verify_module_code(memory,engine,engine_binding::required,"engine2.dll");
    const auto build=memory.get<int>(engine+offsets::build_number);
    if(build<=0)throw std::runtime_error("CS2 PatchVersion is not initialized yet; retry after the game loads.");
    return build;
}
}
