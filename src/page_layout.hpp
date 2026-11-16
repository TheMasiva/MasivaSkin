#pragma once
#include <algorithm>
namespace masiva::page {
inline int fit_size(int owner_width,int dpi,int width,int height){return std::max(1,std::min({dpi,(owner_width*1168/1200)*96/width,(owner_width*584/1200)*96/height}));}
}
