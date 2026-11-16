"""Run against the supplied client.dll, without shipping that DLL.
Reject missing derived scene classes as well as changed callback bytes/layout.
Usage: python tests/preview_binary_contract.py path/to/client.dll
"""
import re
import struct
import sys
from pathlib import Path

raw = Path(sys.argv[1]).read_bytes()
u16 = lambda b, p: struct.unpack_from('<H', b, p)[0]
u32 = lambda b, p: struct.unpack_from('<I', b, p)[0]
u64 = lambda b, p: struct.unpack_from('<Q', b, p)[0]
pe = u32(raw, 0x3c)
assert raw[pe:pe+4] == b'PE\0\0'
optional = pe + 24
base = u64(raw, optional + 24)
image = bytearray(u32(raw, optional + 56))
header_size = u32(raw, optional + 60)
image[:header_size] = raw[:header_size]
sections = optional + u16(raw, pe + 20)
for i in range(u16(raw, pe + 6)):
    s = sections + i * 40
    va, size, offset = (u32(raw, s + n) for n in (12, 16, 20))
    image[va:va+size] = raw[offset:offset+size]
source = (Path(__file__).parents[1] / 'src/windows/preview_binding.hpp').read_text()
for name, rva, values in re.findall(r'Code (\w+)\{(0x[0-9a-f]+), \{([^}]+)\}', source):
    expected = bytes(int(v.strip(), 16) for v in values.split(','))
    rva = int(rva, 16)
    assert image[rva:rva+len(expected)] == expected, name
vt_section = source.split('vtables{', 1)[1].split('};', 1)[0]
expected = {int(v, 16) for v in re.findall(r'0x[0-9a-f]+', vt_section)}
found = set()
needle = struct.pack('<Q', base + 0x781c00)
position = 0
while True:
    position = image.find(needle, position)
    if position < 0:
        break
    vt = position - 146 * 8
    position += 8
    if vt < 8:
        continue
    col = u64(image, vt - 8) - base
    if not 0 <= col < len(image)-24 or u32(image, col) != 1:
        continue
    type_rva = u32(image, col+12)
    name = bytes(image[type_rva+16:type_rva+160]).split(b'\0')[0]
    if b'Position@@' not in name:
        continue
    hierarchy = u32(image, col+16)
    count, array = u32(image, hierarchy+8), u32(image, hierarchy+12)
    assert count < 64
    inherited = False
    for i in range(count):
        descriptor = u32(image, array+4*i)
        type_rva = u32(image, descriptor)
        ancestor = bytes(image[type_rva+16:type_rva+160]).split(b'\0')[0]
        if ancestor == b'.?AVC_CSGO_TeamPreviewCharacterPosition@@':
            assert u32(image, col+4) == 0 and u32(image, descriptor+8) == 0
            inherited = True
    if inherited:
        found.add(vt)
        print(f'{vt:#x}: {name.decode()} — inherited layout / callback verified')
assert expected == found, f'Missing={found-expected}, invalid={expected-found}'
assert len(found) == 10
print('PASS: all 10 scene variants covered; 4 function prefixes match.')
