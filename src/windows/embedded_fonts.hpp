#pragma once
#include <windows.h>
#include <array>
#include <stdexcept>
namespace masiva {
// Process-private registrations keep the header typography independent of installed fonts.
class EmbeddedFonts {
    std::array<HANDLE,2> handles{};
public:
    EmbeddedFonts()=default;
    EmbeddedFonts(const EmbeddedFonts&)=delete;
    EmbeddedFonts& operator=(const EmbeddedFonts&)=delete;
    ~EmbeddedFonts(){for(auto h:handles)if(h)RemoveFontMemResourceEx(h);}
    void load() {
        for(unsigned i=0;i<handles.size();++i) {
            if(handles[i])continue;
            const auto module=GetModuleHandleW(nullptr);
            const auto resource=FindResourceW(module,MAKEINTRESOURCEW(201+i),RT_RCDATA);
            const auto data=resource?LoadResource(module,resource):nullptr;
            const auto bytes=data?LockResource(data):nullptr;
            DWORD count=0;
            if(bytes)handles[i]=AddFontMemResourceEx(bytes,SizeofResource(module,resource),nullptr,&count);
            if(!handles[i] || !count)throw std::runtime_error("Embedded header font could not be loaded.");
        }
    }
};
}
