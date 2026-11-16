#pragma once
#include <windows.h>
#include "../page_layout.hpp"
namespace masiva::page {
inline constexpr wchar_t property[]=L"Masiva.EmbeddedPage";
inline int fit(HWND owner,int dpi,int w,int h){RECT r{};GetClientRect(owner,&r);return fit_size(r.right,dpi,w,h);}
inline HWND create(HWND owner,const wchar_t* cls,int dpi,int width,int height,void* data){
 RECT r{};GetClientRect(owner,&r);int w=MulDiv(width,dpi,96),h=MulDiv(height,dpi,96),top=MulDiv(96,r.right,1200),bottom=MulDiv(680,r.right,1200);
 auto child=CreateWindowExW(WS_EX_CONTROLPARENT,cls,L"",WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,(r.right-w)/2,top+std::max(0,(bottom-top-h)/2),w,h,owner,nullptr,GetModuleHandleW(nullptr),data);
 if(child)SetPropW(owner,property,child);
 return child;
}
inline bool close(HWND owner){auto child=static_cast<HWND>(GetPropW(owner,property));if(!child||!IsWindow(child))return false;DestroyWindow(child);return true;}
inline void loop(HWND owner,HWND child){
 ShowWindow(child,SW_SHOW);SetFocus(GetNextDlgTabItem(child,nullptr,FALSE));MSG msg{};
 while(IsWindow(child)){auto status=GetMessageW(&msg,nullptr,0,0);if(status<=0){if(!status)PostQuitMessage(static_cast<int>(msg.wParam));break;}if(msg.message==WM_KEYDOWN&&msg.wParam==VK_ESCAPE){DestroyWindow(child);break;}if(!IsDialogMessageW(child,&msg)){TranslateMessage(&msg);DispatchMessageW(&msg);}}
 if(IsWindow(child))DestroyWindow(child);
 if(IsWindow(owner)){RemovePropW(owner,property);SetFocus(owner);InvalidateRect(owner,nullptr,FALSE);}
}
}
