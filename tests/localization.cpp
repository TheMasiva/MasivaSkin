#include "item_label.hpp"
#include "extras_catalog.hpp"
#include "update_metadata.hpp"
#include "release.hpp"
#include "gallery_catalog.hpp"
#include "auto_float.hpp"
#include <iomanip>
#include <sstream>
#include <limits>
#include <iostream>
#include <set>
using namespace masiva;
void checked(bool v,int line){if(!v)throw std::runtime_error("Lucy regression at line "+std::to_string(line));}
#define check(v) checked((v),__LINE__)
template<class F>bool rejects(F f){try{f();return false;}catch(const std::exception&){return true;}}
int main(){
 check(auto_float::cleanest(0,1)==.01f);
 check(!auto_float::choose(.10f,.70f,0));
 check(auto_float::cleanest(.10f,.70f)==.10f);
 for(float boundary:{.07f,.15f,.38f,.45f,1.f}) {
  int count=0;
  for(std::size_t i=0;i<5;++i)if(auto v=auto_float::choose(boundary,boundary,i)){++count;check(auto_float::grade_of(*v)==static_cast<int>(i));}
  check(count==1);
 }
 check(!auto_float::choose(1,0,0));check(!auto_float::choose(0,1,5));
 check(!auto_float::choose(std::numeric_limits<float>::quiet_NaN(),1,0));
 std::size_t wear_checks=0;
 for(const auto& f:catalog::finishes) {
  const auto first=auto_float::cleanest(f.minimum,f.maximum);
  check(first>=f.minimum && first<=f.maximum);
  bool available=false;
  for(std::size_t i=0;i<5;++i)if(auto v=auto_float::choose(f.minimum,f.maximum,i)) {
   available=true;++wear_checks;
   check(*v>=f.minimum && *v<=f.maximum);check(auto_float::grade_of(*v)==static_cast<int>(i));
   std::ostringstream out;out<<std::setprecision(9)<<*v;
   check(std::stof(out.str())==*v);
  }
  check(available);
 }
 std::cout<<"Auto Float catalog/category/serialization checks: "<<wear_checks<<" passed\n";
 std::set<int> all_agents;
 for(int team:{2,3}) {
  const auto cards=gallery_entries(3,team,false,0);std::set<int> found;
  for(const auto* f:cards){check(catalog::agent(f->definition));check(extras::buyable(f->definition,team));check(found.insert(f->definition).second);check(all_agents.insert(f->definition).second);check(f->paint==0);}
  for(const auto& a:extras::agents)if(a.team==team)check(found.count(a.id)==1);
  check(found.size()>20);
 }
 check(all_agents.size()==std::size(extras::agents));
 check(gallery_entries(3,2,true,0).empty());
 check(!gallery_entries(0,2,false,7).empty());check(gallery_entries(0,3,false,7).empty());
 check(!gallery_entries(0,3,true,1).empty());
 check(release::subtitle()=="0.9.12 - ASUNA");
 check(std::string(hud_item_label(509,409,true,-1).data())==u8"★ Huntsman Knife | Tiger Tooth");
 check(std::string(hud_item_label(509,409,false,-1).data())==u8"★ Avcı Bıçağı | Kaplan Dişi");
 check(std::string(hud_item_label(7,282,true,-1).data())=="AK-47 | Redline");
 check(std::string(hud_item_label(7,282,true,0).data())==u8"StatTrak™ AK-47 | Redline");
 ItemLabel custom{};std::string(u8"Masiva'nın bıçağı").copy(custom.data(),std::string(u8"Masiva'nın bıçağı").size());
 check(hud_item_label(509,409,false,100,custom)==custom);
 std::size_t dopplers{};
 for(const auto& f:catalog::finishes)if(catalog::knife(f.definition)) {
  const std::string canonical=f.name;const bool gamma=canonical.rfind("Gamma Doppler",0)==0;
  if(!gamma && canonical.rfind("Doppler",0)!=0)continue;
  ++dopplers;
  for(bool english:{true,false})for(int stat:{-1,0,999999}) {
   const std::string actual=hud_item_label(f.definition,f.paint,english,stat).data();
   const std::string expected=std::string(u8"★ ")+(stat<0?"":u8"StatTrak™ ")+localized::weapon(f.definition,english)+" | "+(gamma?"Gamma Doppler":"Doppler");
   check(actual==expected);check(hud_item_label(f.definition,f.paint,english,stat,custom)==custom);
  }
 }
 check(dopplers>100);

 check(std::string(item_label(509,409,false).data())==u8"Avcı Bıçağı | Kaplan Dişi");
 check(std::string(item_label(509,409,true).data())=="Huntsman Knife | Tiger Tooth");
 check(std::string(item_label(7,282).data())=="AK-47 | Redline");
 check(std::string(localized::finish(7,0,false))==u8"Varsayılan");
 std::set<std::pair<int,int>> keys;
 for(const auto& f:catalog::finishes){auto row=localized::info(f.definition,f.paint);check(row&&keys.insert({row->d,row->p}).second);check(*localized::weapon(f.definition,false)&&*localized::finish(f.definition,f.paint,false));check(localized::Filter{}.accepts(f.definition,f.paint));}
 check(keys.size()==std::size(localized::finishes));
 for(const auto& e:extras::stickers)check(*localized::decoration(false,e.id,false));
 for(const auto& e:extras::charms)check(*localized::decoration(true,e.id,false));
 localized::Filter filter;filter.rarity=6;check(filter.accepts(509,409));filter.rarity=1;check(!filter.accepts(509,409));filter={};filter.stattrak=true;check(filter.accepts(509,409));check(!filter.accepts(5032,10010));
 for(const auto& row:localized::finishes){
  if(row.phase){filter={};filter.phase=row.phase;check(filter.accepts(row.d,row.p));filter.phase=row.phase%8+1;check(!filter.accepts(row.d,row.p));}
  if(*row.collections){auto value=std::stoi(std::string(row.collections+1));filter={};filter.collection=value;check(filter.accepts(row.d,row.p));filter.collection=99999;check(!filter.accepts(row.d,row.p));}
 }
 auto meta=parse_dump_metadata("{\"build_number\":14192,\"timestamp\":\"2026-10-23T17:14:58.380890300+00:00\"}");check(meta.build==14192);
 for(auto s:{"{}","{\"build_number\":-1}","{\"build_number\":999999999999999}","<html>network error</html>"})check(rejects([&]{parse_dump_metadata(s);}));
 check(rejects([]{parse_dump_metadata(std::string(5000,'x'));}));
 std::cout<<"EN/TR labels, complete catalog coverage, combined filters and update metadata passed\n";
}
