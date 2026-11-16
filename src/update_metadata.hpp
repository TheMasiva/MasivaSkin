#pragma once
#include <string>
#include <regex>
#include <stdexcept>
namespace masiva {
struct DumpMetadata {int build{};std::string timestamp;};
inline DumpMetadata parse_dump_metadata(const std::string& json){
 if(json.empty()||json.size()>4096)throw std::runtime_error("Invalid update metadata size");
 std::smatch b,t;
 if(!std::regex_search(json,b,std::regex("\\\"build_number\\\"\\s*:\\s*([0-9]{1,7})\\s*[,}]")) ||
    !std::regex_search(json,t,std::regex("\\\"timestamp\\\"\\s*:\\s*\\\"([0-9]{4}-[0-9]{2}-[0-9]{2}T[^\\\"\\r\\n]{1,50})\\\"")))throw std::runtime_error("Invalid update metadata fields");
 const auto build=std::stoi(b[1]);if(build<=0)throw std::runtime_error("Invalid update build");return {build,t[1]};
}
}
