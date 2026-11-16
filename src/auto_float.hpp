#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <optional>

namespace masiva::auto_float {
struct Grade {
    float minimum, maximum, preferred;
    const wchar_t* english;
    const wchar_t* turkish;
};
inline constexpr std::array<Grade,5> grades{{
    {0.f,.07f,.01f,L"Factory New",L"Fabrikadan Yeni"},
    {.07f,.15f,.10f,L"Minimal Wear",L"Az Aşınmış"},
    {.15f,.38f,.25f,L"Field-Tested",L"Görevde Kullanılmış"},
    {.38f,.45f,.40f,L"Well-Worn",L"Eskimiş"},
    {.45f,1.f,.60f,L"Battle-Scarred",L"Savaş Görmüş"}
}};
inline int grade_of(float wear) {
    for(int i=0;i<4;++i)if(wear<grades[i].maximum)return i;
    return 4;
}
inline std::optional<float> choose(float minimum,float maximum,std::size_t grade) {
    if(grade>=grades.size() || !std::isfinite(minimum) || !std::isfinite(maximum) ||
       minimum<0 || maximum>1 || minimum>maximum)return {};
    const auto& band=grades[grade];
    // Wear categories share boundaries. The upper boundary belongs to the next category.
    const float ceiling=grade==4?1.f:std::nextafter(band.maximum,band.minimum);
    const float low=std::max(minimum,band.minimum);
    const float high=std::min(maximum,ceiling);
    if(low>high)return {};
    return std::clamp(band.preferred,low,high);
}
inline float cleanest(float minimum,float maximum) {
    for(std::size_t i=0;i<grades.size();++i)if(auto value=choose(minimum,maximum,i))return *value;
    return minimum;
}
}
