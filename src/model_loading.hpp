#pragma once
#include <string>
#include <algorithm>
#include <cctype>
#include <cstdint>
namespace masiva {
inline std::string normalize_model_path(std::string path) {
    if(path.empty() || path.size()>=512 || path.find('\0')!=std::string::npos)return {};
    std::replace(path.begin(),path.end(),'\\','/');
    std::transform(path.begin(),path.end(),path.begin(),[](unsigned char c){return static_cast<char>(std::tolower(c));});
    if(path.find("..")!=std::string::npos)return {};
    if(path.size()>=7 && path.substr(path.size()-7)==".vmdl_c")path.resize(path.size()-2);
    if(path.size()<6 || path.substr(path.size()-5)!=".vmdl")return {};
    return path;
}
// Resource loads can complete on later frames. Never treat a changed binding
// as proof that the requested model is loaded, or reissue while it is pending.
class ModelLoadWindow {
    std::uint64_t deadline_{};
public:
    void begin(std::uint64_t now){deadline_=now+5000;}
    bool pending(std::uint64_t now)const{return deadline_ && now<deadline_;}
    bool expired(std::uint64_t now)const{return deadline_ && now>=deadline_;}
    void clear(){deadline_=0;}
};
}
