#pragma once
#include <string>
#include <cctype>
namespace masiva::release {
inline constexpr char version[]="0.9.12";
inline constexpr char codename[]="Asuna";
inline constexpr wchar_t window_title[]=L"Masiva Skin 0.9.12 — Asuna";
inline std::string subtitle() {
    std::string name=codename;
    for(auto& c:name)c=static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return std::string(version)+" - "+name;
}
}
