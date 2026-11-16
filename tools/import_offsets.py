"""Import three local cs2-dumper JSON files from ONE pinned Windows commit.

This changes build compatibility; it does NOT validate material regeneration
or the entity identity stride. Review and rebuild after every import.
"""
import argparse
import json
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument("directory", type=Path)
parser.add_argument("--commit", required=True)
args = parser.parse_args()
if len(args.commit) != 40 or any(c not in "0123456789abcdef" for c in args.commit):
    parser.error("--commit must be the exact 40-character commit SHA")
info = json.loads((args.directory / "info.json").read_text())
globals_ = json.loads((args.directory / "offsets.json").read_text())
classes = json.loads((args.directory / "client_dll.json").read_text())["client.dll"]["classes"]
fields = {
    "health": ("C_BaseEntity", "m_iHealth"),
    "team": ("C_BaseEntity", "m_iTeamNum"),
    "life_state": ("C_BaseEntity", "m_lifeState"),
    "owner": ("C_BaseEntity", "m_hOwnerEntity"),
    "identity": ("CEntityInstance", "m_pEntity"),
    "weapon_services": ("C_BasePlayerPawn", "m_pWeaponServices"),
    "active_weapon": ("CPlayer_WeaponServices", "m_hActiveWeapon"),
    "attribute_manager": ("C_EconEntity", "m_AttributeManager"),
    "item": ("C_AttributeContainer", "m_Item"),
    "definition": ("C_EconItemView", "m_iItemDefinitionIndex"),
    "item_id_high": ("C_EconItemView", "m_iItemIDHigh"),
    "paint": ("C_EconEntity", "m_nFallbackPaintKit"),
    "seed": ("C_EconEntity", "m_nFallbackSeed"),
    "wear": ("C_EconEntity", "m_flFallbackWear"),
}
offsets = {k: classes[c]["fields"][f] for k, (c, f) in fields.items()}
offsets.update(local_pawn=globals_["client.dll"]["dwLocalPlayerPawn"],
               entity_list=globals_["client.dll"]["dwEntityList"],
               build_number=globals_["engine2.dll"]["dwBuildNumber"])
if any(type(v) is not int or not 0 < v < 0x20000000 for v in offsets.values()):
    raise ValueError("Invalid offset value")
build = info["build_number"]
if type(build) is not int or not 0 < build < 10000000:
    raise ValueError("Invalid build number")
out = Path(__file__).resolve().parents[1] / "src" / "offsets.hpp"
lines = ["#pragma once", "#include <cstdint>", "namespace masiva::offsets {",
         f"inline constexpr int supported_build = {build};",
         f"inline constexpr const char* source_commit = \"{args.commit}\";",
         f"inline constexpr const char* source_timestamp = {json.dumps(info['timestamp'])};",
         "// Reference layout, NOT emitted by cs2-dumper. Guarded at runtime.",
         "inline constexpr std::uint64_t entity_stride = 0x70;"]
lines += [f"inline constexpr std::uint64_t {name} = 0x{value:x};" for name, value in offsets.items()]
lines += ["}", ""]
out.write_text("\n".join(lines), encoding="utf-8")
print(f"Imported build {build} from {args.commit}; live compatibility remains unverified.")
