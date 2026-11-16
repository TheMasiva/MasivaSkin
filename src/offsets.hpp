#pragma once
#include <cstdint>
namespace masiva::offsets {
inline constexpr const char* source_commit = "Unknown Branch supplied-binary verification; client SHA256 99a6720b29d8b477";
inline constexpr const char* source_timestamp = "2026-11-12";
// Reference layout, NOT emitted by cs2-dumper. Guarded at runtime.
inline constexpr std::uint64_t entity_stride = 0x70;
inline constexpr std::uint64_t health = 0x34c;
inline constexpr std::uint64_t team = 0x3e7;
inline constexpr std::uint64_t life_state = 0x354;
inline constexpr std::uint64_t owner = 0x520;
inline constexpr std::uint64_t original_owner = 0x18a0;
inline constexpr std::uint64_t pawn_controller = 0x14bc;
inline constexpr std::uint64_t controller_pawn = 0x92c;
inline constexpr std::uint64_t controller_steam_id = 0x788;
inline constexpr std::uint64_t identity = 0x10;
inline constexpr std::uint64_t weapon_services = 0x12f0;
inline constexpr std::uint64_t active_weapon = 0x60;
inline constexpr std::uint64_t attribute_manager = 0x1290;
inline constexpr std::uint64_t item = 0x50;
inline constexpr std::uint64_t definition = 0x1ba;
inline constexpr std::uint64_t item_id_high = 0x1d0;
inline constexpr std::uint64_t paint = 0x18a8;
inline constexpr std::uint64_t seed = 0x18ac;
inline constexpr std::uint64_t wear = 0x18b0;
inline constexpr std::uint64_t local_pawn = 0x2553d38;
inline constexpr std::uint64_t entity_list = 0x2709010;
inline constexpr std::uint64_t build_number = 0x6b2d18;
}
