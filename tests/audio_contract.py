"""Static evidence only: python audio_contract.py soundsystem.dll engine2.dll.
No supplied DLL is loaded/executed or redistributed.
"""
from pathlib import Path
import sys,hashlib,json,struct,re
import pefile,capstone
checks=[]
identities=[('d5eb6daf6aa555d4085f74f4ffef639e745d7b366eb367d2a34687dfe9f49cd5',0x6af4a5af,0x6ba000),
 ('dcb5fe13f60f0db9faa365c7b48c3ca107ce56783748bf9ed9ca41fd892af63c',0x6af4a60d,0xa0a000)]
images=[]
for file,(digest,stamp,size) in zip(sys.argv[1:],identities):
 data=Path(file).read_bytes();assert hashlib.sha256(data).hexdigest()==digest
 p=pefile.PE(data=data,fast_load=True)
 assert p.FILE_HEADER.TimeDateStamp==stamp and p.OPTIONAL_HEADER.SizeOfImage==size and p.FILE_HEADER.Machine==0x8664
 images.append(p.get_memory_mapped_image());checks.append(Path(file).name+' exact identity')
assert len(images)==2,'Both module inputs required'
im=images[0];base=0x180000000
root=Path(__file__).resolve().parents[1]
header=(root/'src/windows/audio_binding.hpp').read_text()
prefixes=re.findall(r'\{(0x[0-9a-f]+),\{([^}]+)\}\}',header)
assert len(prefixes)==7
for address,raw in prefixes:
 code=bytes(int(x,16) for x in raw.split(','));a=int(address,16)
 assert len(code)==32 and im[a:a+32]==code
 checks.append('audio prefix '+address)
for a,b in [(0x4ab260,0x1d1d90),(0x4ab008,0x509a0),(0x4ab010,0x1dab90),(0x4ab018,0x1dab80)]:
 assert struct.unpack_from('<Q',im,a)[0]==base+b
checks.append('operator-system start and dictionary virtual entries')
assert im[0x1d1da0]==0xe8 and 0x1d1da5+struct.unpack_from('<i',im,0x1d1da1)[0]==0x1c1fd0
checks.append('virtual start synchronously copies playback descriptor')
c=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_64);c.detail=True
fields=set()
for i in c.disasm(im[0x1c1fd0:0x1c2159],0x1c1fd0):
 for o in i.operands:
  if o.type==capstone.x86.X86_OP_MEM and o.mem.base==capstone.x86.X86_REG_RBX:fields.add(o.mem.disp)
assert fields=={0,8,0x10,0x14,0x18,0x1c,0x20,0x24,0x26,0x27,0x28},fields
assert im[0x1c205e:0x1c2061]==bytes.fromhex('8b4310')
assert im[0x1c2067:0x1c206a]==bytes.fromhex('8b4314')
checks.append('all descriptor fields through byte 0x28; hash and source distinct from GUID')
engine=images[1]
engine_header=(root/'src/windows/engine_binding.hpp').read_text()
for address,raw in re.findall(r'\{(0x[0-9a-f]+),\{([^}]+)\}\}',engine_header):
    code=bytes(int(x,16) for x in raw.split(','));a=int(address,16)
    assert len(code)==32 and engine[a:a+32]==code
assert engine[0x61fec8:0x61fed5]==b'PatchVersion\0'
assert engine[0x245fd2:0x245fd4]==bytes.fromhex('8905')
assert 0x245fd8+struct.unpack_from('<i',engine,0x245fd4)[0]==0x6b2d18
checks.append('new engine PatchVersion parser, destination and runtime identity gate prefixes')
print(json.dumps({'result':'passed','checks':checks,'live_game_test':False},indent=2))
