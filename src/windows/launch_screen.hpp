#pragma once
#include "embedded_fonts.hpp"
#include "process.hpp"
#include "presence.hpp"
#include "launch_ids.h"
#include "../release.hpp"
#include <shellapi.h>
#include "studio_draw.hpp"

namespace masiva {
class LaunchScreen {
    EmbeddedFonts private_fonts;
    ULONG_PTR graphics_token{};
    static constexpr COLORREF background=RGB(18,16,26);
    static constexpr COLORREF accent=RGB(192,163,245);
    HFONT brand{}, caption{};
    HICON steam{};
    HBITMAP portrait{};
    HFONT release_font{};
    ULONGLONG retry_at{};
    int dpi=96;
    int px(int value) const {return MulDiv(value,dpi,96);}

    void init(HWND window) {
        auto dc=GetDC(window);
        dpi=GetDeviceCaps(dc,LOGPIXELSY);
        ReleaseDC(window,dc);
        private_fonts.load();
        Gdiplus::GdiplusStartupInput graphics_input;
        if(Gdiplus::GdiplusStartup(&graphics_token,&graphics_input,nullptr)!=Gdiplus::Ok)
            throw std::runtime_error("Launcher graphics could not initialize.");
        SendMessageW(window,WM_CHANGEUISTATE,MAKEWPARAM(UIS_SET,UISF_HIDEFOCUS),0);
        brand=CreateFontW(-px(36),0,0,0,FW_EXTRABOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Oxanium ExtraBold");
        caption=CreateFontW(-px(16),0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
        release_font=CreateFontW(-px(16),0,0,0,FW_MEDIUM,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Rajdhani");
        const auto subtitle=release::subtitle();
        SetDlgItemTextW(window,IDC_RELEASE_LABEL,std::wstring(subtitle.begin(),subtitle.end()).c_str());
        auto module=GetModuleHandleW(nullptr);
        portrait=LoadBitmapW(module,MAKEINTRESOURCEW(IDB_LAUNCH_ART));
        steam=static_cast<HICON>(LoadImageW(module,MAKEINTRESOURCEW(IDI_LAUNCH_STEAM),IMAGE_ICON,px(24),px(24),0));
        SetWindowTextW(window,release::window_title);
        SendMessageW(window,WM_SETICON,ICON_BIG,reinterpret_cast<LPARAM>(LoadIconW(module,MAKEINTRESOURCEW(101))));
        if(!SetTimer(window,1,650,nullptr))throw win_error("Launcher timer");
    }
    void draw(const DRAWITEMSTRUCT& item) const {
        HDC dc=item.hDC;
        const auto saved=SaveDC(dc);
        RECT rect=item.rcItem;
        SetDCBrushColor(dc,background);
        FillRect(dc,&rect,static_cast<HBRUSH>(GetStockObject(DC_BRUSH)));
        SetBkMode(dc,TRANSPARENT);
        SetTextColor(dc,RGB(246,245,249));
        if(item.CtlID==IDC_LAUNCH_PORTRAIT) {
            BITMAP source{};
            if(portrait && GetObjectW(portrait,sizeof(source),&source)) {
                const int w=rect.right-rect.left,h=rect.bottom-rect.top;
                const int sw=std::min<int>(source.bmWidth,MulDiv(source.bmHeight,w,h));
                const int sh=std::min<int>(source.bmHeight,MulDiv(source.bmWidth,h,w));
                const auto image=CreateCompatibleDC(dc);
                if(image) {
                    const auto before=SelectObject(image,portrait);
                    SetStretchBltMode(dc,HALFTONE);
                    SetBrushOrgEx(dc,0,0,nullptr);
                    StretchBlt(dc,0,0,w,h,image,(source.bmWidth-sw)/2,(source.bmHeight-sh)/2,sw,sh,SRCCOPY);
                    SelectObject(image,before);DeleteDC(image);
                    Gdiplus::Graphics graphics(dc);
                    const int fade=std::max(1,w/4);
                    Gdiplus::LinearGradientBrush shade(Gdiplus::Point(w-fade,0),Gdiplus::Point(w,0),
                        Gdiplus::Color(0,18,16,26),Gdiplus::Color(255,18,16,26));
                    graphics.FillRectangle(&shade,w-fade,0,fade,h);
                }
            }
        } else if(item.CtlID==IDC_RELEASE_LABEL) {
            wchar_t label[64]{};GetWindowTextW(item.hwndItem,label,64);
            SelectObject(dc,release_font);
            SetTextColor(dc,RGB(172,174,186));
            DrawTextW(dc,label,-1,&rect,DT_RIGHT|DT_VCENTER|DT_SINGLELINE);
        } else if(item.CtlID==IDC_LAUNCH_BRAND) {
            SelectObject(dc,brand);
            DrawTextW(dc,L"MASIVA SKIN",-1,&rect,DT_LEFT|DT_SINGLELINE|DT_VCENTER);
        } else {
            const bool close=item.CtlID==IDCANCEL;
            const bool small=close || item.CtlID==IDC_LAUNCH_MINIMIZE || item.CtlID==IDC_LAUNCH_RPC;
            const bool pressed=(item.itemState&ODS_SELECTED)!=0;
            const bool disabled=(item.itemState&ODS_DISABLED)!=0;
            if(small) {
                if(pressed)studio::rounded(dc,rect,RGB(41,41,50),RGB(41,41,50),background,px(5),0);
                studio::icon(dc,item.CtlID==IDC_LAUNCH_RPC?studio::Settings:close?studio::Close:studio::Minus,(rect.right-px(18))/2,
                             (rect.bottom-px(18))/2,px(18),RGB(186,188,200));
            } else {
                const auto top=disabled?RGB(38,38,44):pressed?RGB(70,46,102):RGB(92,63,139);
                studio::rounded(dc,rect,top,RGB(55,40,82),disabled?RGB(66,64,70):accent,px(9));
                SelectObject(dc,caption);
                SetTextColor(dc,disabled?RGB(166,168,181):RGB(246,242,233));
                wchar_t label[40]{};const int length=GetWindowTextW(item.hwndItem,label,40);
                SIZE size{};GetTextExtentPoint32W(dc,label,length,&size);
                const int icon=px(26),gap=px(12),x=(rect.right-size.cx-icon-gap)/2;
                if(item.CtlID==IDC_LAUNCH_PREVIEW)
                    studio::icon(dc,studio::Layers,x,(rect.bottom-icon)/2,icon,accent);
                else DrawIconEx(dc,x,(rect.bottom-icon)/2,steam,icon,icon,0,nullptr,DI_NORMAL);
                rect.left=x+icon+gap;
                DrawTextW(dc,label,length,&rect,DT_LEFT|DT_SINGLELINE|DT_VCENTER);
            }
            if((item.itemState&ODS_FOCUS) && !(item.itemState&ODS_NOFOCUSRECT)) {
                rect=item.rcItem;InflateRect(&rect,-px(3),-px(3));
                studio::outline(dc,rect,accent,static_cast<float>(px(7)));
            }
        }
        RestoreDC(dc,saved);
    }
    void ready(HWND window) {
        retry_at=0;
        SetDlgItemTextW(window,IDOK,L"Launch CS2");
        EnableWindow(GetDlgItem(window,IDOK),TRUE);
        SetFocus(GetDlgItem(window,IDOK));
    }
    void launch(HWND window) {
        if(retry_at)return;
        if(Process::find_pid()){EndDialog(window,IDOK);return;}
        retry_at=GetTickCount64()+120000;
        SetDlgItemTextW(window,IDOK,L"Launching CS2…");
        EnableWindow(GetDlgItem(window,IDOK),FALSE);
        if(reinterpret_cast<INT_PTR>(ShellExecuteW(window,L"open",L"steam://rungameid/730",
                                                   nullptr,nullptr,SW_SHOWNORMAL))<=32) {
            ready(window);
            MessageBoxW(window,L"Steam could not open the launch request. Open Steam and try again.",
                        release::window_title,MB_OK|MB_ICONERROR);
        }
    }
    static INT_PTR CALLBACK dialog(HWND w,UINT msg,WPARAM wp,LPARAM lp) {
        auto self=reinterpret_cast<LaunchScreen*>(GetWindowLongPtrW(w,DWLP_USER));
        try {
            if(msg==WM_INITDIALOG) {
                self=reinterpret_cast<LaunchScreen*>(lp);
                SetWindowLongPtrW(w,DWLP_USER,lp);self->init(w);SetFocus(GetDlgItem(w,IDOK));return FALSE;
            }
            if(!self)return FALSE;
            switch(msg) {
            case WM_CTLCOLORDLG:
                SetDCBrushColor(reinterpret_cast<HDC>(wp),background);
                return reinterpret_cast<INT_PTR>(GetStockObject(DC_BRUSH));
            case WM_LBUTTONDOWN:
                if(HIWORD(lp)<self->px(42)) {
                    ReleaseCapture();SendMessageW(w,WM_NCLBUTTONDOWN,HTCAPTION,0);
                    return TRUE;
                }
                break;
            case WM_DRAWITEM:self->draw(*reinterpret_cast<DRAWITEMSTRUCT*>(lp));return TRUE;
            case WM_COMMAND:
                if(LOWORD(wp)==IDC_LAUNCH_RPC){rpc::settings(w);return TRUE;}
                if(LOWORD(wp)==IDC_LAUNCH_MINIMIZE){ShowWindow(w,SW_MINIMIZE);return TRUE;}
                if(LOWORD(wp)==IDOK){self->launch(w);return TRUE;}
                if(LOWORD(wp)==IDC_LAUNCH_PREVIEW){EndDialog(w,IDC_LAUNCH_PREVIEW);return TRUE;}
                if(LOWORD(wp)==IDCANCEL){EndDialog(w,IDCANCEL);return TRUE;}
                break;
            case WM_CLOSE:EndDialog(w,IDCANCEL);return TRUE;
            case WM_TIMER:
                if(Process::find_pid())EndDialog(w,IDOK);
                else if(self->retry_at && GetTickCount64()>=self->retry_at)self->ready(w);
                return TRUE;
            }
        } catch(const std::exception& error) {
            KillTimer(w,1);
            MessageBoxA(w,error.what(),"Masiva Skin",MB_OK|MB_ICONERROR);
            EndDialog(w,IDCANCEL);return TRUE;
        }
        return FALSE;
    }
public:
    ~LaunchScreen(){if(portrait)DeleteObject(portrait);DeleteObject(release_font);DeleteObject(brand);DeleteObject(caption);if(steam)DestroyIcon(steam);if(graphics_token)Gdiplus::GdiplusShutdown(graphics_token);}
    static int wait_for_game(HINSTANCE instance) {
        if(Process::find_pid())return IDOK;
        LaunchScreen screen;
        const auto result=DialogBoxParamW(instance,MAKEINTRESOURCEW(IDD_CS2_LAUNCH),nullptr,dialog,
                                          reinterpret_cast<LPARAM>(&screen));
        if(result==-1)throw win_error("CS2 launcher");
        return static_cast<int>(result);
    }
};
}
