#pragma once
#include "profile_packet.hpp"
#include "selections.hpp"
#include <filesystem>
#include <fstream>
#include <set>
#include <optional>
#include <cctype>
namespace masiva {
inline bool same_cosmetic(const Cosmetic& a,const Cosmetic& b) {
    return a.team==b.team && a.category==b.category && a.definition==b.definition && a.paint==b.paint && a.seed==b.seed && a.wear==b.wear && a.stattrak==b.stattrak && a.custom_name==b.custom_name && a.decorations==b.decorations;
}
class EditHistory {
    std::optional<Cosmetic> current_;
    std::vector<Cosmetic> previous_;
    bool both_{};
public:
    void reset(const Cosmetic& c,bool both){current_=c;both_=both;previous_.clear();}
    void observe(const Cosmetic& c,bool both) {
        if(!current_ || both_!=both || current_->team!=c.team || current_->category!=c.category || (c.category=="weapon" && c.definition!=current_->definition)){reset(c,both);return;}
        if(same_cosmetic(*current_,c))return;
        if(previous_.size()==32)previous_.erase(previous_.begin());
        previous_.push_back(*current_);current_=c;
    }
    const Cosmetic* peek()const{return previous_.empty()?nullptr:&previous_.back();}
    void commit_undo(){if(previous_.empty())throw std::runtime_error("No previous edit");current_=previous_.back();previous_.pop_back();}
    std::size_t size()const{return previous_.size();}
};
class Favorites {
    std::set<std::pair<int,int>> items_;
public:
    bool contains(int definition,int paint)const{return items_.count({definition,paint})!=0;}
    void toggle(int definition,int paint){if(!catalog::finish(definition,paint))throw std::runtime_error("Unknown favorite");auto key=std::make_pair(definition,paint);if(!items_.erase(key))items_.insert(key);}
    void read(std::istream& in){std::set<std::pair<int,int>> next;std::string magic;int version{};std::size_t n{};
        if(!(in>>magic>>version>>n)||magic!="MASIVA_FAVORITES"||version!=1||n>20000)throw std::runtime_error("Invalid favorites file");
        for(std::size_t i=0;i<n;++i){int d{},p{};if(!(in>>d>>p))throw std::runtime_error("Truncated favorites file");if(catalog::finish(d,p))next.insert({d,p});}
        in>>std::ws;if(!in.eof())throw std::runtime_error("Invalid favorites data");items_.swap(next);
    }
    std::string text()const{std::ostringstream out;out<<"MASIVA_FAVORITES 1 "<<items_.size()<<'\n';for(const auto& p:items_)out<<p.first<<' '<<p.second<<'\n';return out.str();}
};
inline std::vector<Cosmetic> read_profile_file(const std::filesystem::path& path){
    if(std::filesystem::file_size(path)>1048576)throw std::runtime_error("Profile exceeds 1 MB");
    std::ifstream in(path);in.imbue(std::locale::classic());auto items=read(in);Selections checked;checked.set_all(items);(void)encode_profile(items);return items;
}
inline void valid_profile_name(const std::string& name){
    if(name.empty()||name.size()>120||name.front()==' '||name.back()==' '||name.back()=='.')throw std::runtime_error("Use a profile name of 1-120 UTF-8 bytes, without leading/trailing spaces or dots.");
    for(unsigned char c:name)if(c<32 || std::string("<>:\"/\\|?*").find(static_cast<char>(c))!=std::string::npos)throw std::runtime_error("Profile name contains an invalid character.");
    auto stem=name.substr(0,name.find('.'));for(auto& c:stem)c=static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    if(stem=="CON"||stem=="PRN"||stem=="AUX"||stem=="NUL"||(stem.size()==4&&(stem.substr(0,3)=="COM"||stem.substr(0,3)=="LPT")&&stem[3]>='1'&&stem[3]<='9'))throw std::runtime_error("Reserved Windows filename.");
}
inline std::filesystem::path named_profile(const std::filesystem::path& root,const std::string& name){valid_profile_name(name);return root/std::filesystem::u8path(name+".masiva");}
inline std::vector<std::filesystem::path> list_profiles(const std::filesystem::path& root){
    std::filesystem::create_directories(root);std::vector<std::filesystem::path> result;
    for(const auto& entry:std::filesystem::directory_iterator(root))if(entry.is_regular_file()&&!entry.is_symlink()&&entry.path().extension()==".masiva")result.push_back(entry.path());
    std::sort(result.begin(),result.end());return result;
}
}
