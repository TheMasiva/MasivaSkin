#include "collection_tools.hpp"
#include "shared_selection.hpp"
#include <iostream>
#include <chrono>
using namespace masiva;
void check(bool value){if(!value)throw std::runtime_error("collection regression");}
template<class F> bool rejects(F f){try{f();return false;}catch(const std::exception&){return true;}}
int main(){
    Favorites f;f.toggle(7,474);check(f.contains(7,474));
    std::istringstream input(f.text());Favorites loaded;loaded.read(input);check(loaded.contains(7,474));
    std::istringstream broken("MASIVA_FAVORITES 1 2\n7 474\n");check(rejects([&]{loaded.read(broken);})&&loaded.contains(7,474));
    loaded.toggle(7,0);loaded.toggle(1,0);loaded.toggle(7,0);check(!loaded.contains(7,0)&&loaded.contains(1,0));
    loaded.toggle(7,474);check(!loaded.contains(7,474));
    check(rejects([&]{loaded.toggle(65535,999999);}));
    Cosmetic a{"T","weapon",7,474,0,.1f};auto b=a;b.wear=.2f;b.custom_name="Talia";b.stattrak=12;
    EditHistory h;h.reset(a,false);h.observe(b,false);check(h.size()==1&&same_cosmetic(*h.peek(),a));
    h.observe(b,false);check(h.size()==1);h.commit_undo();check(!h.peek());
    auto decorated=b;for(const auto& e:extras::stickers)if(e.id){decorated.decorations.stickers[2]=e.id;break;}
    h.observe(decorated,false);check(same_cosmetic(*h.peek(),a));
    // A failed dispatch may inspect the previous item without consuming history.
    auto retry=*h.peek();check(same_cosmetic(*h.peek(),retry)&&h.size()==1);
    h.commit_undo();h.observe(b,true);check(!h.peek());
    auto ct=b;ct.team="CT";h.observe(ct,true);check(!h.peek());
    h.reset(a,false);for(int i=1;i<=45;++i){auto c=a;c.seed=i;h.observe(c,false);}check(h.size()==32);
    Cosmetic knife{"T","knife",500,0,0,.01f};h.reset(knife,false);auto knife2=knife;knife2.definition=505;h.observe(knife2,false);check(h.peek()->definition==500);
    h.observe(a,false);check(!h.peek());
    for(const auto& n:{"../bad","a/b","C:\\bad","CON","con.txt","LPT1","NUL","a."," a","a ",""})check(rejects([&]{valid_profile_name(n);}));
    valid_profile_name("Mor kombin");valid_profile_name(u8"Türkçe kombin");
    auto root=std::filesystem::temp_directory_path()/("masiva-collections-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    try {
        check(list_profiles(root).empty());auto path=named_profile(root,u8"Türkçe kombin");
        std::vector<Cosmetic> profile{decorated};{std::ofstream out(path);write(out,profile);}
        auto roundtrip=read_profile_file(path);check(roundtrip.size()==1&&same_cosmetic(roundtrip.front(),decorated));
        auto copy=named_profile(root,"Copy");std::filesystem::copy_file(path,copy);check(list_profiles(root).size()==2);
        auto renamed=named_profile(root,"Renamed");std::filesystem::rename(copy,renamed);check(read_profile_file(renamed).front().custom_name=="Talia");
        {std::ofstream out(copy);out<<"broken";}check(rejects([&]{read_profile_file(copy);}));
        std::filesystem::remove_all(root);
    }catch(...){std::filesystem::remove_all(root);throw;}
    std::cout<<"Favorites persistence, isolated bounded undo, profile names and disk roundtrip passed\n";
}
