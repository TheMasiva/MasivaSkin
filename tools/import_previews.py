"""Build offline thumbnails from the pinned CSGO-API metadata.
Usage: import_previews.py skins.json base_weapons.json agents.json
Only pinned Steam/GitHub source image URLs are fetched. No runtime network access.
"""
from pathlib import Path
import concurrent.futures,hashlib,io,json,sys,time,urllib.request
from PIL import Image
root=Path(__file__).resolve().parents[1]
skins,bases,agents=[json.loads(Path(p).read_text()) for p in sys.argv[1:4]]
catalog=json.loads((root/'data/catalog.json').read_text())
want={(int(x['definition']),int(x['paint'])) for x in catalog['finishes']}
want.update((int(x['id']),0) for x in json.loads((root/'data/extras.json').read_text())['agents'])
rows={}
for x in skins:
 key=(int(x['weapon']['weapon_id']),int(x['paint_index'] or 0))
 if key in want:rows[key]={'name':x['name'],'url':x['image']}
for x in bases+agents:
 key=(int(x['def_index']),0)
 if key in want and key not in rows:rows[key]={'name':x['name'],'url':x['image']}
assert set(rows)==want,sorted(want-set(rows))
folder=root/'bin/previews';folder.mkdir(parents=True,exist_ok=True)
def fetch(pair):
 (definition,paint),info=pair;name=f'{definition}_{paint}.png';dest=folder/name
 if not dest.exists():
  steam=info['url'].startswith('https://community.akamai.steamstatic.com/economy/image/')
  assert steam or info['url'].startswith('https://raw.githubusercontent.com/ByMykel/')
  for attempt in range(4):
   try:
    raw=urllib.request.urlopen(info['url']+('/360fx260f' if steam else ''),timeout=25).read()
    if len(raw)>4*1024*1024:raise ValueError('Oversized preview')
    image=Image.open(io.BytesIO(raw)).convert('RGBA');image.thumbnail((320,224),Image.Resampling.LANCZOS)
    image=image.quantize(colors=128,method=Image.Quantize.FASTOCTREE)
    temp=dest.with_suffix('.tmp');image.save(temp,format='PNG',optimize=True);temp.replace(dest);break
   except Exception:
    if attempt==3:raise
    time.sleep(attempt+1)
 with Image.open(dest) as im:assert im.format=='PNG' and im.width<=320 and im.height<=224
 return {'definition':definition,'paint':paint,'file':name,**info,'sha256':hashlib.sha256(dest.read_bytes()).hexdigest()}
with concurrent.futures.ThreadPoolExecutor(max_workers=20) as pool:
 results=[]
 for row in pool.map(fetch,sorted(rows.items())):
  results.append(row)
  if len(results)%200==0:print('previews',len(results),'/',len(rows),flush=True)
(root/'data/previews.json').write_text(json.dumps({'source':'https://github.com/ByMykel/CSGO-API','commit':'72f932098c2fb1ddf647abcf22adee90ccffa088','entries':results},ensure_ascii=False,indent=2)+'\n')
print('Complete',len(results),'images',sum(p.stat().st_size for p in folder.glob('*.png')),'bytes',flush=True)
