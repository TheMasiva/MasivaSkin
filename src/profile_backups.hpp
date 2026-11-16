#pragma once
#include <filesystem>
namespace masiva {
// Keep previous versions alongside the profile; a backup failure prevents replacement.
inline void backup_profile(const std::filesystem::path& path) {
    if(path.extension()!=L".masiva" || !std::filesystem::exists(path))return;
    for(int i=4;i>=1;--i){auto from=path;from+=L".bak"+std::to_wstring(i);auto to=path;to+=L".bak"+std::to_wstring(i+1);
        if(std::filesystem::exists(from))std::filesystem::copy_file(from,to,std::filesystem::copy_options::overwrite_existing);}
    auto backup=path;backup+=L".bak1";std::filesystem::copy_file(path,backup,std::filesystem::copy_options::overwrite_existing);
}
}
