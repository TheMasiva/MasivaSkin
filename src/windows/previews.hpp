#pragma once
#include <gdiplus.h>
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>
#include <string>
namespace masiva {
class PreviewCache {
    ULONG_PTR token{};
    struct Entry{int definition,paint;std::unique_ptr<Gdiplus::Bitmap> bitmap;std::uint64_t used;};
    std::vector<Entry> entries;
    struct Scaled {int definition,paint,w,h;std::unique_ptr<Gdiplus::Bitmap> bitmap;std::uint64_t used;};
    std::vector<Scaled> scaled;
    std::uint64_t clock{},resizes{},scaled_hits{};
    std::size_t decoded_bytes()const{std::size_t n=0;for(const auto& e:entries)if(e.bitmap)n+=static_cast<std::size_t>(e.bitmap->GetWidth())*e.bitmap->GetHeight()*4;return n;}
    std::size_t scaled_bytes()const{std::size_t n=0;for(const auto& e:scaled)n+=static_cast<std::size_t>(e.w)*e.h*4;return n;}

    std::filesystem::path folder;
public:
    PreviewCache(){Gdiplus::GdiplusStartupInput input;Gdiplus::GdiplusStartup(&token,&input,nullptr);
        wchar_t exe[32768]{};GetModuleFileNameW(nullptr,exe,32768);folder=std::filesystem::path(exe).parent_path()/L"previews";}
    ~PreviewCache(){scaled.clear();entries.clear();if(token)Gdiplus::GdiplusShutdown(token);}
    Gdiplus::Bitmap* get(int d,int p) {
        if(!token)return nullptr;
        for(auto& e:entries)if(e.definition==d && e.paint==p){e.used=++clock;return e.bitmap.get();}
        if(entries.size()>=48)entries.erase(std::min_element(entries.begin(),entries.end(),[](const auto& a,const auto& b){return a.used<b.used;}));
        std::unique_ptr<Gdiplus::Bitmap> bitmap;
        const auto path=folder/(std::to_wstring(d)+L"_"+std::to_wstring(p)+L".png");
        std::error_code ec;const auto bytes=std::filesystem::file_size(path,ec);
        if(!ec && bytes<2*1024*1024) {
            bitmap=std::make_unique<Gdiplus::Bitmap>(path.c_str());
            if(bitmap->GetLastStatus()!=Gdiplus::Ok || !bitmap->GetWidth() || !bitmap->GetHeight() || bitmap->GetWidth()>2048 || bitmap->GetHeight()>2048)bitmap.reset();
        }
        const auto required=bitmap?static_cast<std::size_t>(bitmap->GetWidth())*bitmap->GetHeight()*4:0;
        while(!entries.empty() && decoded_bytes()+required>32*1024*1024)entries.erase(std::min_element(entries.begin(),entries.end(),[](const auto& a,const auto& b){return a.used<b.used;}));
        entries.push_back({d,p,std::move(bitmap),++clock});return entries.back().bitmap.get();
    }
    std::string diagnostics()const{return "decoded_bytes="+std::to_string(decoded_bytes())+" scaled_bytes="+std::to_string(scaled_bytes())+" scaled_hits="+std::to_string(scaled_hits)+" resize_jobs="+std::to_string(resizes);}
    bool draw(HDC dc,int d,int p,const RECT& area) {
        if(!RectVisible(dc,&area))return true;
        auto* image=get(d,p);if(!image)return false;
        const auto scale=std::min(static_cast<float>(area.right-area.left)/image->GetWidth(),static_cast<float>(area.bottom-area.top)/image->GetHeight());
        const int w=std::max(1,static_cast<int>(image->GetWidth()*scale)),h=std::max(1,static_cast<int>(image->GetHeight()*scale));
        Gdiplus::Bitmap* rendered=nullptr;
        for(auto& e:scaled)if(e.definition==d && e.paint==p && e.w==w && e.h==h){e.used=++clock;rendered=e.bitmap.get();++scaled_hits;break;}
        if(!rendered){
            const auto required=static_cast<std::size_t>(w)*h*4;
            while(!scaled.empty() && (scaled.size()>=64 || scaled_bytes()+required>24*1024*1024))scaled.erase(std::min_element(scaled.begin(),scaled.end(),[](const auto& a,const auto& b){return a.used<b.used;}));
            if(required>24*1024*1024)return false;
            ++resizes;auto bitmap=std::make_unique<Gdiplus::Bitmap>(w,h,PixelFormat32bppPARGB);
            if(bitmap->GetLastStatus()!=Gdiplus::Ok)return false;
            {Gdiplus::Graphics resize(bitmap.get());resize.Clear(Gdiplus::Color(0,0,0,0));resize.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);resize.DrawImage(image,0,0,w,h);}
            scaled.push_back({d,p,w,h,std::move(bitmap),++clock});rendered=scaled.back().bitmap.get();
        }
        Gdiplus::Graphics graphics(dc);graphics.DrawImage(rendered,area.left+(area.right-area.left-w)/2,area.top+(area.bottom-area.top-h)/2,w,h);return true;
    }
};
}
