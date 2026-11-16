"""Build the embedded weapon/knife catalog from ByMykel skins.json including gloves."""
import json,sys,hashlib
from pathlib import Path
root=Path(__file__).resolve().parents[1]
source=Path(sys.argv[1]); skins=json.loads(source.read_text())
groups={'Pistols':0,'Rifles':1,'SMGs':2,'Heavy':3,'Equipment':4,'Knives':5,'Gloves':6}
weapons={}
for skin in skins:
 group=skin['category']['name']
 if group not in groups:continue
 w=skin['weapon'];weapons[w['weapon_id']]={'name':w['name'],'group':groups[group]}
rows=[];seen=set()
for skin in skins:
 d=skin['weapon'].get('weapon_id')
 if d not in weapons:continue
 p=int(skin['paint_index'] or 0);key=(d,p)
 if key in seen:raise ValueError(f'duplicate catalog pair {key}')
 seen.add(key)
 name=skin['name'].split(' | ',1)[-1] if p else 'Vanilla / Varsayılan'
 if skin.get('phase'):name+=' — '+skin['phase']
 rows.append(dict(definition=d,paint=p,name=name,minimum=skin['min_float'] if skin['min_float'] is not None else 0.,maximum=skin['max_float'] if skin['max_float'] is not None else 1.,legacy=skin['legacy_model']))
for d in weapons:
 if weapons[d]['group']!=6 and (d,0) not in seen:rows.append(dict(definition=d,paint=0,name='Vanilla / Varsayılan',minimum=0.,maximum=1.,legacy=False))
rows.sort(key=lambda r:(r['definition'],r['paint']!=0,r['name'].casefold(),r['paint']))
meta={'source':'https://github.com/ByMykel/CSGO-API','retrieved':'2026-10-25','source_commit':'72f932098c2fb1ddf647abcf22adee90ccffa088','source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'weapons':weapons,'finishes':rows}
(root/'data/catalog.json').write_text(json.dumps(meta,ensure_ascii=False,indent=2)+'\n')
def q(s):return json.dumps(s,ensure_ascii=True)
def f(n):return repr(float(n))+'f'
s='#pragma once\n#include <cstddef>\n#include <array>\n#include <cstdint>\nnamespace masiva::catalog {\nstruct Weapon { int definition; const char* name; int group; };\nstruct Finish { int definition, paint; const char* name; float minimum, maximum; bool legacy; };\ninline constexpr Weapon weapons[] = {\n'
for d,v in sorted(weapons.items(),key=lambda x:(x[1]["group"],x[1]["name"])):s+=f'    {{{d}, {q(v["name"])}, {v["group"]}}},\n'
s+='};\ninline constexpr Finish finishes[] = {\n'
for r in rows:s+=f'    {{{r["definition"]}, {r["paint"]}, {q(r["name"])}, {f(r["minimum"])}, {f(r["maximum"])}, {str(r["legacy"]).lower()}}},\n'
s+='''};
inline const Weapon* weapon(int d) { for(const auto& w:weapons) if(w.definition==d)return &w; return nullptr; }
constexpr std::size_t finish_bucket(int definition,int paint) {
    const auto key=static_cast<std::uint32_t>(definition)*0x9e3779b1u ^
                   static_cast<std::uint32_t>(paint)*0x85ebca6bu;
    return (key^(key>>16)) & 8191u;
}
inline constexpr auto finish_index=[]() constexpr {
    std::array<std::uint16_t,8192> slots{};
    static_assert(std::size(finishes)<slots.size()/2,"Increase the finish index capacity");
    for(std::size_t i=0;i<std::size(finishes);++i) {
        auto slot=finish_bucket(finishes[i].definition,finishes[i].paint);
        while(slots[slot])slot=(slot+1)%slots.size();
        slots[slot]=static_cast<std::uint16_t>(i+1);
    }
    return slots;
}();
inline const Finish* finish(int definition,int paint) {
    auto slot=finish_bucket(definition,paint);
    while(const auto entry=finish_index[slot]) {
        const auto& candidate=finishes[entry-1];
        if(candidate.definition==definition && candidate.paint==paint)return &candidate;
        slot=(slot+1)%finish_index.size();
    }
    return nullptr;
}
inline bool knife(int d) { auto w=weapon(d); return d==42 || d==59 || (w && w->group==5); }
inline bool firearm(int d) { auto w=weapon(d); return w && w->group<5; }
inline bool glove(int d) { auto w=weapon(d); return w && w->group==6; }
inline const char* category(int d) { return glove(d)?"gloves":knife(d)?"knife":"weapon"; }
inline bool compatible(int active,int chosen) { return knife(chosen) ? knife(active) : firearm(chosen) && active==chosen; }
}
'''
(root/'src/catalog.hpp').write_text(s)
print('catalog entries',len(rows),'weapons',len(weapons))
if (root/'data/extras.json').exists():
 import subprocess
 subprocess.run([sys.executable,str(root/'tools/import_extras.py')],check=True)
