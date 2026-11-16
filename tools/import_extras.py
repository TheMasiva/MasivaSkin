"""Rebuild extras_catalog.hpp and append agents after import_catalog.py.
Input: the reviewed, schema-filtered data/extras.json snapshot shipped with release.
"""
from pathlib import Path
import json,re
root=Path(__file__).resolve().parents[1]
x=json.loads((root/'data/extras.json').read_text());q=lambda s:json.dumps(s,ensure_ascii=True)
s='#pragma once\n#include <cstddef>\nnamespace masiva::extras {\nstruct Entry { int id; const char* name; };\nstruct Agent { int id,team; const char* model; const char* voice; };\n'
for name in ('stickers','charms'):
 s+='inline constexpr Entry '+name+'[] = {\n{0,"Yok / None"},\n'
 for e in sorted(x[name],key=lambda a:a['name'].casefold()):s+='{'+str(e['id'])+','+q(e['name'])+'},\n'
 s+='};\n'
s+='inline constexpr Agent agents[] = {\n'
for a in x['agents']:s+='{'+str(a['id'])+','+('2' if a['team']=='terrorists' else '3')+','+q(a['model'])+','+q(a['voice'])+'},\n'
s+='''};
inline const Agent* agent(int id) {for(const auto& a:agents)if(a.id==id)return &a;return nullptr;}
template<std::size_t N> inline bool contains(const Entry (&rows)[N],int id) {for(const auto& x:rows)if(x.id==id)return true;return false;}
inline bool buyable(int definition,int team) {
    switch(definition) {
'''
for d,m in x['weapon_teams'].items():s+=f'case {d}:return '+('team==2 || team==3' if m==3 else 'team==2' if m==1 else 'team==3')+';\n'
s+='default:if(const auto a=agent(definition))return a->team==team;return true;\n}}\n}\n'
(root/'src/extras_catalog.hpp').write_text(s)
p=root/'src/catalog.hpp';s=p.read_text();ids={a['id'] for a in x['agents']}
s='\n'.join(l for l in s.splitlines() if not (re.match(r'    \{(\d+),',l) and int(re.match(r'    \{(\d+),',l)[1]) in ids))+'\n'
pos=s.index('};\ninline constexpr Finish');s=s[:pos]+''.join('    {'+str(a['id'])+', '+q(a['name'])+', 7},\n' for a in x['agents'])+s[pos:]
pos=s.index('};\ninline const Weapon*');s=s[:pos]+''.join('    {'+str(a['id'])+', 0, "Varsayilan / Default", 0.f, 1.f, false},\n' for a in x['agents'])+s[pos:]
if 'inline bool agent(int d)' not in s:s=s.replace('inline const char* category(int d) { return','inline bool agent(int d) {auto w=weapon(d);return w && w->group==7;}\ninline const char* category(int d) { return agent(d)?"agent":')
p.write_text(s)
print('Extras rebuilt:',len(x['stickers']),'stickers,',len(x['charms']),'charms,',len(x['agents']),'agents')
