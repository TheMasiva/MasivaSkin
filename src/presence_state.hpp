#pragma once
#include <string>
#include <string_view>
#include <algorithm>
#include <cctype>
#include <stdexcept>
namespace masiva::presence {
struct State {std::string map,mode,activity,event;};
// Read only current fields, never the previous-state and observer subtrees.
class Reader {
    std::string_view s; size_t pos=0;
    char peek(){while(pos<s.size() && (s[pos]==' ' || s[pos]=='\n' || s[pos]=='\r' || s[pos]=='\t'))++pos;return pos<s.size()?s[pos]:0;}
    void take(char c){if(peek()!=c)throw std::invalid_argument("JSON");++pos;}
    std::string str(){
        take('"');std::string value;
        while(pos<s.size()) {
            char c=s[pos++];if(c=='"')return value;
            if(static_cast<unsigned char>(c)<32)break;
            if(c=='\\'){if(pos>=s.size())break;c=s[pos++];if(c=='u'){for(int i=0;i<4;++i)if(pos>=s.size() || !std::isxdigit(static_cast<unsigned char>(s[pos++])))throw std::invalid_argument("escape");c='?';}}
            value+=c;
        }
        throw std::invalid_argument("string");
    }
    void value(State& out,int depth,std::string_view group,std::string_view key){
        if(depth>32)throw std::invalid_argument("depth");
        char c=peek();
        if(c=='{' || c=='['){
            ++pos;char end=c=='{'?'}':']';
            if(peek()!=end)for(;;){
                std::string field;if(c=='{'){field=str();take(':');}
                value(out,depth+1,key,field);
                if(peek()!=',')break;
                ++pos;
            }
            take(end);
        }else if(c=='"'){
            auto v=str();
            if(depth==2 && group=="map" && key=="name")out.map=v;
            if(depth==2 && group=="map" && key=="mode")out.mode=v;
            if(depth==2 && group=="player" && key=="activity")out.activity=v;
            if(depth==1 && key=="evt")out.event=v;
        }else{
            size_t start=pos;
            while(pos<s.size() && std::string_view(",}] \n\t\r").find(s[pos])==std::string_view::npos)++pos;
            if(pos==start)throw std::invalid_argument("value");
        }
    }
public:
    explicit Reader(std::string_view in):s(in){}
    State parse(){State r;if(peek()!='{')throw std::invalid_argument("object");value(r,0,"","");if(peek())throw std::invalid_argument("tail");return r;}
};
inline std::string safe_label(std::string s){
    if(s.size()>80 || !std::all_of(s.begin(),s.end(),[](unsigned char c){return (c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9') || c=='_' || c=='-';}))return {};
    return s;
}
inline std::string details(const State& s){
    auto map=safe_label(s.map),mode=safe_label(s.mode);
    if(map.empty())return s.activity=="menu"?"In the lobby":"In Counter-Strike 2";
    if(mode=="scrimcomp2v2")mode="Wingman";
    else if(mode=="gungameprogressive")mode="Arms Race";
    else if(!mode.empty())mode[0]=static_cast<char>(std::toupper(static_cast<unsigned char>(mode[0])));
    return mode.empty()?map:mode+" | "+map;
}
}
