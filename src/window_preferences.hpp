#pragma once
#include <algorithm>
namespace masiva::ui {
inline int opacity(int value){return std::clamp(value,50,100);}
inline int scale(int value){return std::clamp(value,70,140);}
inline int alpha(int value){return (opacity(value)*255+50)/100;}
inline int fitted_dpi(int system,int percent,int width,int height) {
    return std::max(24,std::min({system*scale(percent)/100,
        std::max(24,(width-32)*96/1200),std::max(24,(height-32)*96/720)}));
}
}
