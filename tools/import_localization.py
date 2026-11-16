"""Regenerate localized_catalog.hpp from the reviewed metadata shipped in data/."""
import json
from pathlib import Path
root=Path(__file__).resolve().parents[1]
m=json.loads((root/'data/localization-metadata.json').read_text())
weapons=m['weapons'];rows=m['finishes'];decor=m['decorations'];collections=m['collections']
rarity_en=['Consumer Grade','Industrial Grade','Mil-Spec / High Grade','Restricted / Remarkable','Classified / Exotic','Covert / Extraordinary','Contraband'];rarity_tr=['Tüketici Sınıfı','Endüstri Sınıfı','Askeri / Yüksek Kalite','Sınırlı / Dikkat Çekici','Gizli / Egzotik','Çok Gizli / Sıra Dışı','Kaçak']
phases=['Phase 1','Phase 2','Phase 3','Phase 4','Ruby','Sapphire','Black Pearl','Emerald']

def byname(name):return m["collection_translations"].get(name,name)
q=lambda x:json.dumps(x,ensure_ascii=True)
s='''#pragma once
// Generated from data/localization-metadata.json by tools/import_localization.py.
#include "catalog.hpp"
#include <algorithm>
#include <string>
#include <iterator>
namespace masiva::localized {
struct WeaponName {int d;const char* tr;};
inline constexpr WeaponName weapons[]={
'''+''.join('{%s,%s},\n'%(d,q(n)) for d,n in sorted(weapons))+'''};
struct FinishInfo {int d,p;const char* tr;int rarity;const char* collections;int phase;bool stattrak;};
inline constexpr FinishInfo finishes[]={
'''
for r in sorted(rows,key=lambda r:(r['d'],r['p'])):s+='{%d,%d,%s,%d,%s,%d,%s},\n'%(r['d'],r['p'],q(r['tr']),r['rarity'],q(r['collections']),r['phase'],str(r['stattrak']).lower())
s+='''};
struct DecorationName {bool charm;int id;const char* en;const char* tr;};
inline constexpr DecorationName decorations[]={
'''+''.join('{%s,%d,%s,%s},\n'%(str(c).lower(),id,q(en),q(tr)) for c,id,en,tr in sorted(decor))+'''};
struct Choice {int id;const char* en;const char* tr;};
'''
for name,choices in [('rarities',[(i+1,a,b) for i,(a,b) in enumerate(zip(rarity_en,rarity_tr))]),('collections',[(i+1,name,byname(name)) for i,(key,name) in enumerate(collections)]),('phases',[(i+1,a,b) for i,(a,b) in enumerate(zip(phases,['Faz 1','Faz 2','Faz 3','Faz 4','Yakut','Safir','Siyah İnci','Zümrüt']))])]:
 s+='inline constexpr Choice '+name+'[]={\n'+''.join('{%d,%s,%s},\n'%(i,q(a),q(b)) for i,a,b in choices)+'};\n'
s+='''inline const FinishInfo* info(int d,int p){
 auto i=std::lower_bound(std::begin(finishes),std::end(finishes),std::pair<int,int>{d,p},[](const auto& a,const auto& b){return std::pair<int,int>{a.d,a.p}<b;});
 return i!=std::end(finishes)&&i->d==d&&i->p==p?i:nullptr;
}
inline const char* weapon(int d,bool english=true){
 auto w=catalog::weapon(d);if(!w)return "";if(english)return w->name;
 auto i=std::lower_bound(std::begin(weapons),std::end(weapons),d,[](const auto& a,int b){return a.d<b;});return i!=std::end(weapons)&&i->d==d?i->tr:w->name;
}
inline const char* finish(int d,int p,bool english=true){
 auto f=catalog::finish(d,p);if(!f)return "";if(!p)return english?"Default":"Varsayılan";
 if(!english)if(auto i=info(d,p))return i->tr;return f->name;
}
inline const char* decoration(bool charm,int id,bool english=true){
 if(!id)return english?"None":"Yok";
 const auto key=std::pair<bool,int>{charm,id};
 auto i=std::lower_bound(std::begin(decorations),std::end(decorations),key,[](const auto& a,const auto& b){return std::pair<bool,int>{a.charm,a.id}<b;});
 return i!=std::end(decorations)&&i->charm==charm&&i->id==id?(english?i->en:i->tr):"";
}
struct Filter {
 int rarity{},collection{},phase{};bool stattrak{};
 bool active()const{return rarity||collection||phase||stattrak;}
 bool accepts(int d,int p)const{
  if(!active())return true;auto f=info(d,p);if(!f)return false;
  return (!rarity||f->rarity==rarity)&&(!phase||f->phase==phase)&&(!stattrak||f->stattrak)&&
    (!collection||std::string(f->collections).find("|"+std::to_string(collection)+"|")!=std::string::npos);
 }
};
}
'''
# Keep compact single-line helpers warning-clean.
s=s.replace('if(!english)if(auto i=info(d,p))return i->tr;return f->name;','if(!english){if(auto i=info(d,p))return i->tr;}\n return f->name;').replace('if(!active())return true;auto f=info(d,p);if(!f)return false;','if(!active())return true;\n  auto f=info(d,p);if(!f)return false;')
(root/'src/localized_catalog.hpp').write_text(s)
