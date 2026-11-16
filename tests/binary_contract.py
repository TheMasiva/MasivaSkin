"""Static contract checks against the supplied DLL; does not load or execute it.
Usage: python binary_contract.py /path/to/client.dll
Dependencies: pefile, capstone. These are development tools, not EXE dependencies.
"""
from pathlib import Path
import hashlib
import json
import re
import struct
import sys
import pefile
import capstone

source = Path(sys.argv[1])
data = source.read_bytes()
expected = '99a6720b29d8b477d55f0a2d7fd4333b5df13f354ba723804e996bead1231aa8'
if hashlib.sha256(data).hexdigest() != expected:
    raise SystemExit('Unsupported DLL hash; no addresses validated.')
pe = pefile.PE(data=data, fast_load=True)
im = pe.get_memory_mapped_image()
assert pe.FILE_HEADER.Machine == 0x8664
assert pe.FILE_HEADER.TimeDateStamp == 0x6af4a59c
assert pe.OPTIONAL_HEADER.SizeOfImage == 0x298a000
base = pe.OPTIONAL_HEADER.ImageBase
root = Path(__file__).resolve().parents[1]
header = (root/'src/windows/binding.hpp').read_text()
found = re.findall(r'Code (\w+)\{(0x[0-9a-f]+), \{([^}]+)\}', header)
assert len(found) == 38
checks = ['exact file hash', 'x64 PE identity']
for name, rva, content in found:
    code = bytes(int(b.strip(), 16) for b in content.split(','))
    assert len(code) == 32
    actual = pe.get_data(int(rva, 16), 32)
    assert actual == code, name
    damaged = bytearray(actual); damaged[0] ^= 1
    assert damaged != code, 'mismatched prefix must be rejected'
    checks.append(name+' prefix')
assert struct.unpack_from('<Q', im, 0x1c5aa90+36*8)[0]-base == 0xb628b0
checks.append('primary client vtable slot 36')
for table in (0x1d08050, 0x1d164f0):
    assert [struct.unpack_from('<Q', im, table+i*8)[0]-base for i in (3,4,5)] == [0x1187f90,0x112aae0,0x112ac10]
checks.append('entity and base item getter vtables')
dis = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
def call(at, target):
    i = next(dis.disasm(im[at:at+5], at))
    assert i.mnemonic == 'call' and int(i.op_str,16) == target, hex(at)
call(0x113a9ee, 0x119c340)
call(0x113aa0f, 0x119c340)
call(0x113aa33, 0x119c340)
call(0x82550b, 0x14e0a90)
call(0x825515, 0x7fd820)
call(0x82558f, 0x14e0a90)
call(0x8255b0, 0x7fa510)
checks.append('attribute and material native call sites')
# Validate the getter semantics rather than relying on a guessed vtable label.
for pointer, text in ((0x2245710,b'set item texture seed'),(0x2245728,b'set item texture wear')):
    address=struct.unpack_from('<Q',im,pointer)[0]-base
    assert im[address:address+len(text)] == text
checks.append('seed/wear getter schema names')
assert struct.unpack_from('<I',im,0xb62e78+(8-3)*4)[0] == 0xb62cdf
assert im[0x1c5bfa0:0x1c5bfa0+14] == b'FrameNetUpdate'
assert im[0x1c5bfb8:0x1c5bfb8+19] == b'FramePostDataUpdate'
checks.append('stage dispatch and update markers (static evidence)')
i = next(dis.disasm(im[0x8254e0:0x8254e3],0x8254e0))
assert i.size == 3 and i.mnemonic == 'mov'
assert i.address < 0x825490+0x52 < i.address+i.size
checks.append('reject old +0x52 instruction patch')
assert im[0xc84e90:0xc84e98] == bytes.fromhex('c6 81 9d 17 00 00 01 c3')
assert im[0xc5b2ba:0xc5b2c1] == bytes.fromhex('49 8d be a0 17 00 00')
assert im[0xc5b2e2:0xc5b2e9] == bytes.fromhex('80 bf e8 01 00 00 00')
checks.append('native glove refresh flag and initialized item path')
assert im[0x119bc36:0x119bc43] == bytes.fromhex('41 b8 a1 00 00 00 48 81 c1 99 03 00 00')
checks.append('name override bounded 161-byte native setter')
assert struct.unpack_from('<Q',im,0x1cd9938+6*8)[0]-base == 0xea9800
checks.append('weapon-selection HUD callback table and slot')
assert im[0xed1400:0xed140f] == bytes.fromhex('48 63 c7 48 8d 0c c0 49 8b 47 58 8b 54 c8 38')
checks.append('HUD 72-byte row layout and weapon handle field')
assert im[0xea93f5:0xea9402] == bytes.fromhex('48 8d 54 24 30 c7 44 24 30 c4 5f 41 0b')
call(0xea9402,0x17ada10)
checks.append('native weapon_name token and dialog setter call')
assert im[0xc5e304:0xc5e30b] == bytes.fromhex('48 8d 8b f8 15 00 00')
call(0xc5e30b,0xc5b100)
assert im[0xc5b115:0xc5b11c] == bytes.fromhex('4c 8b b1 b0 00 00 00')
checks.append('native glove controller call site and owner layout')
assert im[0x7c7290:0x7c7297] == bytes.fromhex('8b 81 b4 18 00 00 c3')
assert im[0x113aa38:0x113aa3e] == bytes.fromhex('8b 83 b4 18 00 00')
checks.append('native StatTrak fallback getter and attribute builder')
# 0.6.1: the glove world controller is only the producer. The arms callback
# consumes its model/materials after asynchronous work has completed.
call(0x10ce717,0x10ced00)
call(0x10cee48,0x10bfec0)
call(0x10bfedc,0x10c01c0)
assert im[0x10cef93:0x10cef9a] == bytes.fromhex('83 bd c8 0a 00 00 00')
call(0x10ceff3,0x10cd1c0)
call(0x10cf07d,0x14e0bd0)
checks.append('first-person glove load gate, creation and material transfer')
assert im[0x10cee41:0x10cee48] == bytes.fromhex('49 8d 8d 28 13 00 00')
assert im[0x10cee67:0x10cee6b] == bytes.fromhex('48 c1 e6 04')
assert im[0x10cef1c:0x10cef23] == bytes.fromhex('c7 46 0c ff ff ff ff')
checks.append('glove attachment map, stride and native invalidation path')
call(0xca44a5,0x7fccc0)
call(0xca44b0,0x7fc220)
call(0x7fcd2a,0x1182b80)
checks.append('native physical StatTrak/name-tag builder call sequence')
assert im[0x113aa45:0x113aa49] == bytes.fromhex('66 0f 6e d0')
assert im[0x1ced058:0x1ced058+10] == b'kill eater'
call(0x113aa53,0x119c340)
call(0x113aa65,0x119c340)
checks.append('StatTrak integer-bit ABI and dynamic-attribute writes')
assert im[0x119a586:0x119a58d] == bytes.fromhex('48 63 81 10 02 00 00')
assert im[0x119a5bb:0x119a5bf] == bytes.fromhex('48 83 c0 48')
call(0x119a5d8,0x1199fb0)
assert im[0x119a5f6:0x119a5fb] == bytes.fromhex('e9 75 d8 ff ff')
checks.append('native per-attribute removal, 72-byte entries and cache invalidation')
# World attachment creation and the HUD attachment copy/creation are distinct.
call(0x10ceaa9,0x10d0570)
call(0x10ceab1,0x10d2260)
call(0x10d061b,0x8058a0)
call(0x10d238f,0x805a10)
call(0x10d0903,0x10cd1c0)
call(0x10d243e,0x10cd1c0)
checks.append('separate first-person name-tag and StatTrak creation paths')
call(0x10cef17,0xa16160)
assert im[0x10cef1c:0x10cef23] == bytes.fromhex('c7 46 0c ff ff ff ff')
checks.append('native addon removal and handle invalidation sequence')
call(0xc5b302,0x118b7a0)
call(0xc5b31a,0x1187ef0)
assert im[0xc5b43c:0xc5b443] == bytes.fromhex('41 88 9e 9d 17 00 00')
checks.append('glove model/paint preflight and missing-producer creation branch')
call(0xc5b1de,0x784b80)
call(0xc5b1f3,0x9c1210)
call(0xc5b1fb,0xa16160)
checks.append('default glove bodygroup predicate and custom model deletion path')
call(0x791a16,0x95e750)
for address, token in ((0x1baf6c8,b'firstperson_default'),(0x1baf6e0,b'firstperson_hide_default_gloves'),(0x1baf690,b'thirdperson_default'),(0x1baf6a8,b'thirdperson_hide_default_gloves')):
    assert im[address:address+len(token)+1] == token+b'\0'
checks.append('native default glove bodygroup setter and first/third person states')
call(0xca44be,0x7fb160)
call(0x10ceab9,0x10cf5a0)
checks.append('native world and HUD charm creation call sequence')
call(0xab7c0e,0x176f160)
assert im[0x176f160:0x176f164] == bytes.fromhex('48 8b 51 08')
checks.append('loaded model resource name accessor, independent of network symbol')
call(0xcb109e,0x10c8920)
assert im[0x10c895a:0x10c8961] == bytes.fromhex('4d 8d a6 d8 1d 00 00')
call(0x10c8a7b,0x21c6f0)
call(0x10c8adb,0xa16160)
checks.append('native arms owner controller, cached pawn resource comparison and replacement')
call(0x10ccf17,0x21c6f0)
call(0x10ccf22,0x95f900)
call(0x10cbb78,0x21c6f0)
assert im[0x10cbb80:0x10cbb87] == bytes.fromhex('48 89 8f b0 13 00 00')
checks.append('arms initialize from pawn model and record source resource')
assert struct.unpack_from('<Q',im,0x1cce968)[0] == base+0xe7ab90
call(0xe7abbb,0x1a02690)
call(0xe81906,0x1661e0)
call(0xe81985,0x1661e0)
call(0xe81af7,0xb65250)
checks.append('death notice listener and event weapon/attacker token ABI')
assert im[0xe81902:0xe81906] == bytes.fromhex('48 8b 58 50')
assert im[0xe81979:0xe8197d] == bytes.fromhex('48 8b 58 78')
assert im[0x1b27728:0x1b27731] == b'attacker\0'
checks.append('event string and player-slot virtual entries')
call(0xea17a2,0x17e6e00)
assert im[0x1cafa68:0x1cafa68+len(b'file://{images}/icons/equipment/%s.svg')] == b'file://{images}/icons/equipment/%s.svg'
checks.append('native HUD row image vector and SVG source setter')
assert im[0x1cbcc98:0x1cbcca9] == b'WeaponIcon--name\0'
assert im[0xea19f4:0xea19fc] == bytes.fromhex('c7 44 24 40 51 45 61 01')
call(0xea1a0c,0x17ada10)
checks.append('weapon-selection caption uses WeaponIcon--name token, not ammo HUD weapon_name')
assert im[0x1e55178:0x1e55189] == b'SoundOpSystem001\0'
assert struct.unpack_from('<Q',im,0x1e54a18)[0]==base+0x278b690
assert im[0x3ae075:0x3ae07a] == bytes.fromhex('44 89 64 24 68')
assert im[0x3ae081:0x3ae085] == bytes.fromhex('89 5c 24 6c')
assert im[0x3ae0b3:0x3ae0b6] == bytes.fromhex('ff 50 58')
checks.append('SoundOpSystem001 binding, descriptor hash/source fields and virtual start slot')
# Ownership and core schema offsets are read from the new supplied DLL itself.
# Names may occur in multiple classes; the required exact name/offset pair must exist.
fields={'m_EconGloves':0x17a0,'m_bNeedToReApplyGloves':0x179d,'m_OriginalOwnerXuidLow':0x18a0,'m_OriginalOwnerXuidHigh':0x18a4,
'm_steamID':0x788,'m_hController':0x14bc,'m_hPlayerPawn':0x92c,
'm_iHealth':0x34c,'m_iTeamNum':0x3e7,'m_lifeState':0x354,'m_hOwnerEntity':0x520,
'm_pWeaponServices':0x12f0,'m_hActiveWeapon':0x60,'m_AttributeManager':0x1290,
'm_Item':0x50,'m_nFallbackPaintKit':0x18a8,'m_nFallbackSeed':0x18ac,'m_flFallbackWear':0x18b0,
'm_iItemDefinitionIndex':0x1ba,'m_iItemIDHigh':0x1d0}
for name,offset in fields.items():
    matching=[]
    for text in re.finditer(re.escape(name.encode()+b'\0'),im):
        for ref in re.finditer(re.escape(struct.pack('<Q',base+text.start())),im):
            if struct.unpack_from('<I',im,ref.start()+16)[0]==offset:matching.append(ref.start())
    assert matching,(name,hex(offset))
checks.append('new binary schema: original owner, controller identity and core item offsets')
assert 0xbeccb0+7+struct.unpack_from('<i',im,0xbeccb0+3)[0]==0x2553c40
assert struct.unpack_from('<I',im,0xbc5ab5+3)[0]==0xf8
assert 0x2553c40+0xf8==0x2553d38
checks.append('local pawn derives from native prediction object plus validated member')
for at in (0x14a060,0x16b2ac,0x16b2b5):
    assert at+7+struct.unpack_from('<i',im,at+3)[0]==0x2709010
checks.append('entity list global confirmed by three independent native references')
for table,expected_name in ((0x1d02800,b'.?AVC_CS2HudModelArms@@'),(0x1d01e80,b'.?AVC_CS2HudModelWeapon@@')):
    locator=struct.unpack_from('<Q',im,table-8)[0]-base
    descriptor=struct.unpack_from('<I',im,locator+12)[0]
    assert im[descriptor+16:descriptor+16+len(expected_name)]==expected_name
checks.append('arms and weapon vtables retain their exact RTTI class identities')
print(json.dumps({'result':'passed','checks':checks,'live_game_test':False},indent=2))
