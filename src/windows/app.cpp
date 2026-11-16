#include "launch_screen.hpp"
#include "../auto_float.hpp"
#include "bridge_client.hpp"
#include "../input.hpp"
#include "../module_contract.hpp"
#include "../shared_selection.hpp"
#include "../ui_palette.hpp"
#include "../window_preferences.hpp"
#include "../collection_tools.hpp"
#include "../profile_backups.hpp"
#include <functional>
#include <tuple>
#include "../release.hpp"
#include "../item_label.hpp"
#include <commdlg.h>
#include <windowsx.h>
#include <commctrl.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include <algorithm>
#include <cwctype>
#include <cmath>
#include "menu_host.hpp"
#include "previews.hpp"
#include "update_check.hpp"
#include "studio_draw.hpp"
#include "../gallery_catalog.hpp"
#include "embedded_page.hpp"

namespace {
using namespace masiva;
COLORREF bg = RGB(15, 16, 23), panel = RGB(24, 25, 36);
COLORREF text = RGB(235, 234, 245), muted = RGB(160, 158, 179), purple = RGB(132, 92, 236);
enum Id { Connect = 100, Disconnect, Apply, Restore, Save, Load, Report, Team, Weapon, Seed, Wear, Preset, Log, Group, Filter, Advanced, Language, StatEnabled, StatCount, NameTag, DecorationsButton, HudCompatibility, TeamT, TeamCT, TabWeapons, TabKnives, TabGloves, TabAgents, PreviousPage, NextPage, MenuKey, HideMenu, ExitMenu, TeamBoth, ThemeMode, AccentColor, Opacity, MenuScale, FavoriteToggle, FavoritesOnly, UndoEdit, AdvancedFilters, UpdateCheck, Settings, Customize, ModelPicker, AutoFloat, DiscordPresence, Recent, QuickLoadout, PalettePreset, ClearFilters, Home, Appearance, GeneralSettings, MainColor, ResetMainColor };
std::wstring wide(const std::string& s) {
    if (s.empty()) return {};
    int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), static_cast<int>(s.size()), nullptr, 0);
    if (count <= 0) return L"Metin dönüştürülemedi";
    std::wstring out(count, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), out.data(), count);
    return out;
}
std::string utf8(const std::wstring& s) {
    if (s.empty()) return {};
    int count = WideCharToMultiByte(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), nullptr, 0, nullptr, nullptr);
    std::string out(count, '\0');
    WideCharToMultiByte(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), out.data(), count, nullptr, nullptr);
    return out;
}
std::wstring search_fold(std::wstring s){
    if(s.empty())return s;
    std::wstring result(s.size(),L'\0');
    const auto count=LCMapStringEx(LOCALE_NAME_INVARIANT,LCMAP_LOWERCASE,s.data(),static_cast<int>(s.size()),result.data(),static_cast<int>(result.size()),nullptr,nullptr,0);
    if(count<=0)result=s;
    for(auto& ch:result)if(ch==L'ı'||ch==L'İ')ch=L'i';
    return result;
}
#include "decoration_editor.inl"
#include "collection_dialogs.inl"
#include "filter_editor.inl"
struct App {
    HWND window{}, log{}, team{}, preset{}, weapon{}, seed{}, wear{}, group{}, filter{}, language{}, stat_enabled{}, stat_count{}, name_tag{};
    ULONGLONG animation_started{};
    bool fading=false;
    int opacity_percent=100,scale_percent=100,system_dpi=96;
    HWND opacity_control{},scale_control{};
    HWND hovered_button{};
    HANDLE watched_game{};
    bool exit_posted=false;
    bool settings_open=false,customize_open=false,appearance_open=false,page_open=false;
    bool custom_main=false;COLORREF main_color=RGB(7,8,11);int page_route=0;
    struct PageScope {App& app;bool settings,customize;
        PageScope(App& a,int route):app(a),settings(a.settings_open),customize(a.customize_open){app.page_open=true;app.page_route=route;app.settings_open=false;app.customize_open=false;app.resize();}
        ~PageScope(){app.page_open=false;app.page_route=0;app.settings_open=settings;app.customize_open=customize;if(IsWindow(app.window))app.resize();}
    };
    bool preview_only=false;
    ULONGLONG next_connect{};
    std::wstring connection_error;
    struct Placement {HWND handle;int x,y,w,h;};
    std::vector<Placement> placements;

    bool english=true, light_theme=false;
    ui::Palette palette=ui::palette(false,RGB(137,66,230));
    HICON portrait{};
    COLORREF custom_colors[16]{};
    MenuHost menu_host;
    PreviewCache previews;
    std::size_t gallery_page{};
    int desired_gallery_definition{};
    int hovered_card=-1;
    bool agents_view()const{return SendMessageW(group,CB_GETCURSEL,0,0)==3;}
    Decorations decoration_selection{};
    bool advanced=false, updating=false;
    std::vector<const catalog::Weapon*> visible_weapons;
    std::vector<const catalog::Finish*> visible_finishes;
    EmbeddedFonts embedded_fonts;
    HFONT brand_font{}, release_font{};
    HFONT body{}, title{}, small{}, medium{};
    HBRUSH background = CreateSolidBrush(bg), surface = CreateSolidBrush(panel), input_surface=CreateSolidBrush(RGB(17,18,22));
    int dpi = 96;
    std::wstring state = L"Waiting CS2.exe", details, entries;
    std::unique_ptr<Process> process;
    std::unique_ptr<Game> game;
    std::unique_ptr<BridgeClient> bridge_client;
    std::string last_bridge_report;
    std::shared_ptr<UpdateResult> upstream;
    std::string update_status;
    void check_updates(){if(!upstream){upstream=check_upstream_dump();EnableWindow(GetDlgItem(window,UpdateCheck),FALSE);}}
    void poll_updates(){
        if(upstream&&upstream->done.load(std::memory_order_acquire)){
            if(!upstream->error.empty())update_status=english?"Update check unavailable; current verified bindings retained.":"Güncelleme kontrolüne erişilemedi; doğrulanmış mevcut bağlantılar korunuyor.";
            else update_status=(english?"Upstream dumper build: ":"Dumper kaynak build: ")+std::to_string(upstream->metadata.build)+" · "+upstream->metadata.timestamp+
                (english?". Informational only: offsets alone do not verify HUD/material/audio compatibility.":". Bilgilendirme: adresler tek başına HUD/materyal/ses uyumluluğunu doğrulamaz.");
            add_log(wide(update_status));upstream.reset();EnableWindow(GetDlgItem(window,UpdateCheck),TRUE);
        }
    }
    bool ready = false;
    std::wstring feedback=L"Eşya → kaplama → Uygula. Elde olması gerekmez.";
    std::vector<Cosmetic> profile;
    bool profile_pending=false;
    Favorites favorites;
    bool favorites_only=false;
    std::vector<std::pair<int,int>> recent;
    std::vector<Cosmetic> submitted;
    std::filesystem::path active_profile;
    std::array<float,6> hover_levels{};
    std::vector<std::pair<menu::CardRect,int>> filter_chips;
    ULONGLONG last_animation{},toast_until{},last_header{};
    std::wstring toast_text;
    bool toast_error=false;
    void notify(const std::wstring& message,bool error=false) {
        toast_text=message;toast_error=error;toast_until=GetTickCount64()+(error?6500:3200);
        RECT region{px(20),px(576),px(1180),px(630)};InvalidateRect(window,&region,FALSE);
    }
    void remember_recent(const Cosmetic& c) {
        auto key=std::make_pair(c.definition,c.paint);
        recent.erase(std::remove(recent.begin(),recent.end(),key),recent.end());recent.insert(recent.begin(),key);
        if(recent.size()>20)recent.resize(20);
        std::ostringstream out;for(auto v:recent)out<<v.first<<' '<<v.second<<'\n';
        save_text(settings_path().parent_path()/L"Masiva-Recent.txt",out.str());
    }
    void theme_presets() {
        auto menu=CreatePopupMenu();int i=0;for(auto name:{L"Midnight",L"Emerald",L"Amethyst",L"Snow"})AppendMenuW(menu,MF_STRING,4100+i++,name);
        POINT pt{};GetCursorPos(&pt);auto id=TrackPopupMenu(menu,TPM_RETURNCMD|TPM_NONOTIFY,pt.x,pt.y,0,window,nullptr);DestroyMenu(menu);
        if(id<4100||id>4103)return;
        const COLORREF colors[]={RGB(102,151,246),RGB(53,205,157),RGB(166,116,245),RGB(104,91,214)};
        custom_main=false;light_theme=id==4103;purple=colors[id-4100];update_theme();notify(english?L"Theme saved":L"Tema kaydedildi");
    }

    localized::Filter catalog_filter;
    EditHistory edit_history;
    void track_edit() {
        if(updating)return;
        try {edit_history.observe(selection(),both());}catch(const std::exception&){}
        EnableWindow(GetDlgItem(window,UndoEdit),edit_history.peek()!=nullptr);
        InvalidateRect(GetDlgItem(window,FavoriteToggle),nullptr,FALSE);
        InvalidateRect(GetDlgItem(window,FavoritesOnly),nullptr,FALSE);
    }
    void undo_edit() {
        track_edit();const auto previous=edit_history.peek();
        if(!previous)return;
        const auto restored=*previous;
        // Preserve the undo entry if game submission fails, so it can be retried.
        if(bridge_client){activate_profile();add_log(wide(submit(restored)));remember_targets(submitted,restored,both());}
        put_selection(restored,true);remember(restored);edit_history.commit_undo();
        feedback=english?L"Previous edit restored. Save your profile to keep it.":L"Önceki değişiklik geri alındı. Kalıcı olması için profilini kaydet.";
        InvalidateRect(window,nullptr,FALSE);
    }
    void favorite_toggle() {
        const auto i=SendMessageW(preset,CB_GETCURSEL,0,0);
        if(i<0 || static_cast<std::size_t>(i)>=visible_finishes.size())return;
        const auto f=*visible_finishes[i];auto next=favorites;next.toggle(f.definition,f.paint);
        save_text(settings_path().parent_path()/L"Masiva-Favorites.txt",next.text());favorites=std::move(next);
        if(favorites_only)reload_finishes(f.paint,true);
        RedrawWindow(window,nullptr,nullptr,RDW_INVALIDATE|RDW_ALLCHILDREN);
    }
    void open_collection(bool loadout,bool recent_view=false) {
        PageScope page(*this,recent_view?Recent:loadout?Load:Save);
        CollectionDialog dialog;dialog.owner=window;dialog.english=english;dialog.loadout=loadout;
        dialog.dpi=dpi;dialog.font=body;dialog.palette=palette;dialog.previews=&previews;
        dialog.directory=settings_path().parent_path()/L"profiles";dialog.snapshot=profile;dialog.active=active_profile;dialog.recent_mode=recent_view;
        if(recent_view){dialog.snapshot.clear();for(auto key:recent){const auto* f=catalog::finish(key.first,key.second);if(!f)continue;Cosmetic c;c.team=SendMessageW(team,CB_GETCURSEL,0,0)==1?"CT":"T";c.category=catalog::category(key.first);c.definition=key.first;c.paint=key.second;c.wear=auto_float::cleanest(f->minimum,f->maximum);if(!extras::buyable(c.definition,c.team=="T"?2:3))c.team=c.team=="T"?"CT":"T";dialog.snapshot.push_back(c);}}
        if(!loadout){try {remember_targets(dialog.snapshot,selection(),both());}catch(const std::exception&){dialog.valid_draft=false;}}
        dialog.save=[this](const auto& path,const auto& bytes){save_text(path,bytes);};
        const auto chosen=dialog.run();if(dialog.saved_current){profile=dialog.snapshot;active_profile=dialog.saved_path;notify(english?L"Profile saved; previous version backed up.":L"Profil kaydedildi; önceki sürüm yedeklendi.");}
        if(!chosen.empty())load_profile(chosen);
        if(dialog.reapply_current){profile_pending=true;activate_profile();notify(english?L"Loadout ready":L"Kombin hazır");}
        if(dialog.chosen_item){put_selection(*dialog.chosen_item);track_edit();}
    }

    std::filesystem::path settings_path() const {
        wchar_t exe[32768]{};GetModuleFileNameW(nullptr,exe,32768);
        return std::filesystem::path(exe).parent_path()/L"Masiva-Skin.ini";
    }
    std::wstring tr(std::wstring value) const;
    void language_changed() {
        english=SendMessageW(language,CB_GETCURSEL,0,0)==1;
        WritePrivateProfileStringW(L"UI",L"OverlayLanguage",english?L"en":L"tr",settings_path().c_str());
        const auto selected=SendMessageW(group,CB_GETCURSEL,0,0);
        SendMessageW(group,CB_RESETCONTENT,0,0);
        for(auto name:{L"Silahlar",L"Bıçaklar",L"Eldivenler",L"Ajanlar"}) {
            const auto t=tr(name);SendMessageW(group,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(t.c_str()));
        }
        SendMessageW(group,CB_SETCURSEL,selected<0?0:selected,0);
        auto cue=tr(L"Kaplama ara…");SendMessageW(filter,EM_SETCUEBANNER,FALSE,reinterpret_cast<LPARAM>(cue.c_str()));
        const auto d=selected_definition();auto index=SendMessageW(preset,CB_GETCURSEL,0,0);
        const auto paint=index>=0&&static_cast<std::size_t>(index)<visible_finishes.size()?visible_finishes[index]->paint:0;
        updating=true;reload_weapons(d);reload_finishes(paint,true);updating=false;
        refresh_key_label();resize();poll();InvalidateRect(window,nullptr,TRUE);RedrawWindow(window,nullptr,nullptr,RDW_INVALIDATE|RDW_ALLCHILDREN);
    }
    bool both()const{return SendMessageW(team,CB_GETCURSEL,0,0)==2;}
    bool available(int definition)const{return both()?shared_item(definition):extras::buyable(definition,SendMessageW(team,CB_GETCURSEL,0,0)==0?2:3);}
    void remember(const Cosmetic& c){remember_targets(profile,c,both());}
    std::string submit(const Cosmetic& c) {
        return bridge_client->request(both()?bridge::QueueBoth:bridge::Queue,WeaponState{},&c);
    }
    void scope_controls(){ShowWindow(GetDlgItem(window,TabAgents),(settings_open || page_open || customize_open || both())?SW_HIDE:SW_SHOW);}
    void update_theme(bool persist=true) {
        palette=custom_main?ui::custom_palette(main_color,purple):ui::palette(light_theme,purple);bg=palette.background;panel=palette.panel;text=palette.text;muted=palette.muted;
        DeleteObject(background);DeleteObject(surface);background=CreateSolidBrush(bg);surface=CreateSolidBrush(panel);DeleteObject(input_surface);input_surface=CreateSolidBrush(palette.card);
        if(persist){WritePrivateProfileStringW(L"UI",L"CustomMain",custom_main?L"1":L"0",settings_path().c_str());WritePrivateProfileStringW(L"UI",L"MainColor",std::to_wstring(main_color).c_str(),settings_path().c_str());WritePrivateProfileStringW(L"UI",L"LightTheme",light_theme?L"1":L"0",settings_path().c_str());WritePrivateProfileStringW(L"UI",L"Accent",std::to_wstring(purple).c_str(),settings_path().c_str());}
        SetWindowTextW(GetDlgItem(window,ThemeMode),light_theme?L"Tema: Beyaz":L"Tema: Siyah");
        RedrawWindow(window,nullptr,nullptr,RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_ERASE);
    }
    void choose_color(bool main=false) {
        CHOOSECOLORW dialog{};dialog.lStructSize=sizeof(dialog);dialog.hwndOwner=window;dialog.rgbResult=main?bg:purple;
        dialog.lpCustColors=custom_colors;dialog.Flags=CC_FULLOPEN|CC_RGBINIT;
        if(ChooseColorW(&dialog)){if(main){main_color=dialog.rgbResult&0xffffff;custom_main=true;light_theme=ui::luminance(main_color)>0.179;}else purple=dialog.rgbResult&0xffffff;update_theme();}
    }
    void restore_selection() {
        const auto selected_team=SendMessageW(team,CB_GETCURSEL,0,0)==1?"CT":"T";
        auto i=std::find_if(profile.begin(),profile.end(),[&](const auto& c){return c.team==selected_team && c.definition==selected_definition();});
        if(i!=profile.end())put_selection(*i,true);
        else { decoration_selection={};SetWindowTextW(name_tag,L"");SetWindowTextW(stat_count,L"0");SendMessageW(stat_enabled,BM_SETCHECK,BST_UNCHECKED,0); }
    }
    int px(int v) const { return MulDiv(v, dpi, 96); }
    ~App() {
        if(watched_game)CloseHandle(watched_game);
        DeleteObject(brand_font); DeleteObject(release_font);
        DeleteObject(body); DeleteObject(title); DeleteObject(small); DeleteObject(medium);
        DeleteObject(background); DeleteObject(surface);DeleteObject(input_surface);if(portrait)DestroyIcon(portrait);
    }
    HFONT font(int size, int weight = FW_NORMAL, const wchar_t* face = L"Segoe UI") {
        return CreateFontW(-px(size), 0, 0, 0, weight, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                           OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, face);
    }
    HWND control(const wchar_t* cls, const wchar_t* name, int id, int x, int y, int w, int h, DWORD style) {
        HWND result = CreateWindowExW(0, cls, name, WS_CHILD | WS_VISIBLE | style,
            px(x), px(y), px(w), px(h), window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), GetModuleHandleW(nullptr), nullptr);
        if (!result) throw win_error("Arayuz olusturulamadi");
        SendMessageW(result, WM_SETFONT, reinterpret_cast<WPARAM>(body), TRUE);
        placements.push_back({result,x,y,w,h});
        return result;
    }
    void button(const wchar_t* name, int id, int x, int y, int w) {
        control(L"BUTTON", name, id, x, y, w, 38, BS_OWNERDRAW | WS_TABSTOP);
    }
    std::wstring value(HWND h) const {
        int n = GetWindowTextLengthW(h);
        std::wstring result(static_cast<std::size_t>(n) + 1, L'\0');
        GetWindowTextW(h, result.data(), n + 1); result.resize(n); return result;
    }
    void add_log(const std::wstring& message) {
        SYSTEMTIME now{}; GetLocalTime(&now);
        wchar_t stamp[32]; swprintf_s(stamp, L"[%02u:%02u:%02u] ", now.wHour, now.wMinute, now.wSecond);
        entries += stamp; entries += tr(message); entries += L"\r\n";
        if (entries.size() > 40000) entries.erase(0, entries.find(L'\n', entries.size() - 30000) + 1);
        SetWindowTextW(log, entries.c_str());
        SendMessageW(log, EM_SETSEL, static_cast<WPARAM>(-1), static_cast<LPARAM>(-1));
        SendMessageW(log, EM_SCROLLCARET, 0, 0);
    }
    int selected_definition() const {
        if(agents_view()) {
            const auto choice=SendMessageW(preset,CB_GETCURSEL,0,0);
            if(choice>=0 && static_cast<std::size_t>(choice)<visible_finishes.size()) {
                const auto d=visible_finishes[choice]->definition;
                if(catalog::agent(d) && available(d))return d;
            }
        }
        auto i=SendMessageW(weapon,CB_GETCURSEL,0,0);
        if(i<0 || static_cast<std::size_t>(i)>=visible_weapons.size()) return 0;
        return visible_weapons[i]->definition;
    }
    Cosmetic selection() const {
        auto text_value = utf8(value(wear)); std::replace(text_value.begin(), text_value.end(), ',', '.');
        std::istringstream in(text_value); in.imbue(std::locale::classic());
        float f{};if(!(in>>f))throw std::runtime_error("Float sayisal olmali.");
        in>>std::ws;if(!in.eof())throw std::runtime_error("Float alaninda gecersiz karakter var.");
        auto index=SendMessageW(preset,CB_GETCURSEL,0,0);
        if(index<0 || static_cast<std::size_t>(index)>=visible_finishes.size())throw std::runtime_error("Kaplama sec.");
        const auto& finish=*visible_finishes[index];
        int definition=selected_definition();
        Cosmetic c{SendMessageW(team,CB_GETCURSEL,0,0)==1 ? "CT" : "T",catalog::category(definition),
            definition,finish.paint,parse_integer(utf8(value(seed)),"Seed",1000,true),f};
        if(c.category!="gloves" && c.category!="agent") {
            c.custom_name=utf8(value(name_tag));
            if(SendMessageW(stat_enabled,BM_GETCHECK,0,0)==BST_CHECKED)
                c.stattrak=parse_integer(utf8(value(stat_count)),"StatTrak",999999,true);
        }
        if(c.category=="weapon")c.decorations=decoration_selection;
        validate(c);
        if(f<finish.minimum || f>finish.maximum)throw std::runtime_error("Float secilen kaplamanin araligi disinda.");
        return c;
    }
    void reload_weapons(int desired=0) {
        SendMessageW(weapon,CB_RESETCONTENT,0,0);visible_weapons.clear();
        const int selected_group=static_cast<int>(SendMessageW(group,CB_GETCURSEL,0,0));
        int choice=0;
        for(const auto& w:catalog::weapons) if(((selected_group==0 && w.group<5) || w.group==selected_group+4) && available(w.definition)) {
            if(w.definition==desired)choice=static_cast<int>(visible_weapons.size());
            visible_weapons.push_back(&w);auto name=wide(localized::weapon(w.definition,english));
            SendMessageW(weapon,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(name.c_str()));
        }
        SendMessageW(weapon,CB_SETCURSEL,choice,0);
    }
    static std::wstring lowercase(std::wstring s) {
        return search_fold(std::move(s));
    }
    void reload_finishes(int selected_paint=0,bool preserve_wear=false) {
        const auto query=lowercase(value(filter));
        const int definition=desired_gallery_definition?desired_gallery_definition:selected_definition();
        desired_gallery_definition=0;
        SendMessageW(preset,CB_RESETCONTENT,0,0);visible_finishes.clear();
        int choice=0;bool kept=false;
        const auto candidates=gallery_entries(static_cast<int>(SendMessageW(group,CB_GETCURSEL,0,0)),
            SendMessageW(team,CB_GETCURSEL,0,0)==1?3:2,both(),definition);
        for(const auto* item:candidates) {
            const auto& f=*item;
            if(favorites_only && !favorites.contains(f.definition,f.paint))continue;
            auto name=wide(agents_view()?localized::weapon(f.definition,english):localized::finish(f.definition,f.paint,english));
            const auto fallback=agents_view()?catalog::weapon(f.definition)->name:f.name;
            if(lowercase(name).find(query)==std::wstring::npos && lowercase(wide(fallback)).find(query)==std::wstring::npos)continue;
            if(!agents_view() && !catalog_filter.accepts(f.definition,f.paint))continue;
            if(f.paint==selected_paint && (!agents_view() || f.definition==definition)) {choice=static_cast<int>(visible_finishes.size());kept=true;}
            visible_finishes.push_back(item);
            SendMessageW(preset,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(name.c_str()));
        }
        SendMessageW(preset,CB_SETCURSEL,visible_finishes.empty()?-1:choice,0);
        if(!preserve_wear || !kept)choose_preset();
        EnableWindow(GetDlgItem(window,AutoFloat),!agents_view() && !visible_finishes.empty());
        gallery_page=static_cast<std::size_t>(std::max(0,choice))/menu::page_size;
        InvalidateRect(window,nullptr,FALSE);InvalidateRect(GetDlgItem(window,ModelPicker),nullptr,FALSE);
    }
    void put_selection(const Cosmetic& c,bool preserve_scope=false) {
        const bool keep_both=preserve_scope && both() && shared_item(c.definition);
        auto f=catalog::finish(c.definition,c.paint);auto w=catalog::weapon(c.definition);
        if(!f || !w)throw std::runtime_error("Profildeki esya/kaplama bu katalogda bulunmuyor.");
        updating=true;favorites_only=false;catalog_filter={};
        SendMessageW(team,CB_SETCURSEL,keep_both?2:c.team=="T"?0:1,0);scope_controls();
        SendMessageW(group,CB_SETCURSEL,w->group<5?0:w->group-4,0);reload_weapons(c.definition);
        decoration_selection=c.decorations;
        SetWindowTextW(filter,L"");desired_gallery_definition=c.definition;reload_finishes(c.paint);
        SetWindowTextW(seed,std::to_wstring(c.seed).c_str());
        std::ostringstream out;out.imbue(std::locale::classic());out<<std::setprecision(9)<<c.wear;
        SetWindowTextW(wear,wide(out.str()).c_str());
        SetWindowTextW(name_tag,wide(c.custom_name).c_str());SetWindowTextW(stat_count,std::to_wstring(std::max(0,c.stattrak)).c_str());
        SendMessageW(stat_enabled,BM_SETCHECK,c.stattrak>=0?BST_CHECKED:BST_UNCHECKED,0);updating=false;
        RedrawWindow(window,nullptr,nullptr,RDW_INVALIDATE|RDW_ALLCHILDREN);
    }
    void choose_preset() {
        auto i=SendMessageW(preset,CB_GETCURSEL,0,0);
        if(i<0 || static_cast<std::size_t>(i)>=visible_finishes.size())return;
        auto f=visible_finishes[i];
        gallery_page=static_cast<std::size_t>(i)/menu::page_size;
        std::ostringstream float_text;float_text.imbue(std::locale::classic());
        float_text<<std::setprecision(9)<<auto_float::cleanest(f->minimum,f->maximum);
        SetWindowTextW(wear,wide(float_text.str()).c_str());
        EnableWindow(GetDlgItem(window,AutoFloat),!agents_view());
        InvalidateRect(window,nullptr,FALSE);
    }
    std::filesystem::path pick_file(bool saving, bool report = false) {
        wchar_t path[32768]{};
        if (saving) wcscpy_s(path, report ? L"Masiva-Skin-tani.txt" : L"Masiva-Skin.masiva");
        OPENFILENAMEW dialog{}; dialog.lStructSize = sizeof(dialog); dialog.hwndOwner = window;
        dialog.lpstrFile = path; dialog.nMaxFile = static_cast<DWORD>(std::size(path));
        dialog.lpstrFilter = report ? L"Report (*.txt)\0*.txt\0\0" : L"Masiva profile (*.masiva)\0*.masiva\0\0";
        dialog.lpstrDefExt = report ? L"txt" : L"masiva";
        dialog.Flags = OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | (saving ? OFN_OVERWRITEPROMPT : OFN_FILEMUSTEXIST);
        if (!(saving ? GetSaveFileNameW(&dialog) : GetOpenFileNameW(&dialog))) {
            if (CommDlgExtendedError()) throw std::runtime_error("Dosya penceresi acilamadi.");
            return {};
        }
        return std::filesystem::path(path);
    }
    void save_text(const std::filesystem::path& path, const std::string& bytes) {
        backup_profile(path);
        auto temporary = path; temporary += L".tmp-" + std::to_wstring(GetCurrentProcessId());
        try {
            std::ofstream out(temporary, std::ios::binary | std::ios::trunc);
            out.exceptions(std::ios::failbit | std::ios::badbit);
            out.write(bytes.data(), static_cast<std::streamsize>(bytes.size())); out.close();
            if (!MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
                throw win_error("Dosya kaydedilemedi");
        } catch (...) { std::error_code ec; std::filesystem::remove(temporary, ec); throw; }
    }
    void save_profile() {
        const auto c = selection();
        auto next = profile;
        remember_targets(next,c,both());
        std::ostringstream out; write(out, next);
        auto path = pick_file(true); if (path.empty()) return;
        save_text(path, out.str()); profile = std::move(next);
        add_log(L"Profil kaydedildi. Diğer takım ve eşya kayıtları korundu.");
    }
    void load_profile(std::filesystem::path path={}) {
        if(path.empty())path=pick_file(false);
        if (path.empty()) return;
        if (std::filesystem::file_size(path) > 1048576) throw std::runtime_error("Profil dosyasi cok buyuk.");
        std::ifstream in(path); in.imbue(std::locale::classic());
        auto next = read(in);
        (void)encode_profile(next); // Validate every row before replacing the loaded profile.
        const auto desired_team = SendMessageW(team, CB_GETCURSEL, 0, 0) == 1 ? "CT" : "T";
        auto it = std::find_if(next.begin(), next.end(), [&](const auto& c) {
            return c.team == desired_team && (c.definition==selected_definition() || (c.category!="weapon" && c.category==catalog::category(selected_definition())));
        });
        if (it == next.end()) it = std::find_if(next.begin(), next.end(), [](const auto& c) { return catalog::finish(c.definition,c.paint)!=nullptr && extras::buyable(c.definition,c.team=="T"?2:3); });
        if (it != next.end()) put_selection(*it,true);
        profile = std::move(next);active_profile=path;
        notify(english?L"Profile loaded":L"Profil yüklendi");
        try{edit_history.reset(selection(),both());}catch(const std::exception&){}
        profile_pending=true;
        add_log(L"Profil yüklendi; tüm kayıtlı seçimler etkinleştiriliyor.");
        try {
            if(game)activate_profile();
            else feedback=english?L"Profile ready. Waiting CS2.exe":L"Profil hazır. Waiting CS2.exe";
        } catch(const std::exception& e) {
            add_log(wide(e.what()));
            feedback=english?L"Profile ready; apply again when the game is ready.":L"Profil hazır; oyun hazır olduğunda Uygula ile tekrar deneyebilirsin.";
            add_log(feedback);
        }
        InvalidateRect(window,nullptr,FALSE);
    }
    void activate_profile() {
        if(!profile_pending || !bridge_client)return;
        add_log(wide(bridge_client->request_profile(profile)));
        profile_pending=false;submitted=profile;
        feedback=L"Profil etkin · tüm kayıtlı silah, bıçak ve eldiven seçimleri hazır.";
        add_log(feedback);
    }
    void connect() {
        auto next = std::make_unique<Process>();
        const auto validated_build=verified_patch_version(*next,next->client,next->engine);
        auto session = std::make_unique<Game>(*next, next->client, next->engine,validated_build);
        const auto build = session->build();
        auto next_bridge = std::make_unique<BridgeClient>(next->pid());
        process = std::move(next); game = std::move(session);
        rpc::game_started(process->path());
        bridge_client = std::move(next_bridge);
        sync_hud_mode();
        activate_profile();
        add_log(L"CS2 bağlantısı kuruldu. PID " + std::to_wstring(process->pid()) + L", build " + std::to_wstring(build));
    }
    void sync_hud_mode() {
        if(bridge_client)bridge_client->hud_mode(false);
    }
    bool agent_selected()const{return catalog::agent(selected_definition());}
    void disconnect() {
        if (bridge_client && bridge_client->status() && bridge_client->status()->initialized==1) {
            try { add_log(wide(bridge_client->request(bridge::Stop, WeaponState{}))); }
            catch(const std::exception& e) { add_log(wide(e.what())); }
        }
        if (bridge_client && bridge_client->backup_count()) {
            try { add_log(wide(bridge_client->request(bridge::Restore, game->snapshot()))); }
            catch (const std::exception& e) { add_log(L"Görünüm geri alınamadı: " + wide(e.what())); }
            if(bridge_client->glove_backup()) {
                try { add_log(wide(bridge_client->request(bridge::RestoreGloves,game->glove_snapshot()))); }
                catch(const std::exception& e) { add_log(L"Eldiven geri alınamadı: "+wide(e.what())); }
            }
            if (bridge_client->backup_count()) add_log(L"Bekleyen görünüm yedekleri var; tamamen temizlemek için CS2'yi yeniden başlat.");
        }
        if(bridge_client && bridge_client->status() && bridge_client->status()->agent_backup) {
            try {add_log(wide(bridge_client->request(bridge::RestoreAgent,WeaponState{})));}
            catch(const std::exception& e) {add_log(L"Ajan geri alınamadı: "+wide(e.what()));}
        }
        if (bridge_client) last_bridge_report = bridge_report();
        bridge_client.reset();submitted.clear();
        game.reset(); process.reset(); ready = false;
        state = L"Waiting CS2.exe"; details.clear();
    }
    bool glove_selected() const { return catalog::glove(selected_definition()); }
    WeaponState selected_snapshot() const { return glove_selected()?game->glove_snapshot():game->snapshot(); }
    std::string last_auto_status,last_glove_status,last_part_status,last_agent_status;
    void show_bridge_progress() {
        if(!bridge_client || !bridge_client->status())return;
        const auto* b=bridge_client->status();
        const auto show=[&](const char* text,std::size_t size,std::string& previous,const wchar_t* prefix) {
            const std::string current(text,std::find(text,text+size,'\0'));
            if(current.empty() || current==previous)return;
            previous=current;add_log(std::wstring(prefix)+wide(current));
        };
        show(b->auto_message,sizeof(b->auto_message),last_auto_status,L"Otomatik: ");
        show(b->glove_message,sizeof(b->glove_message),last_glove_status,L"Eldiven: ");
        show(b->agent_message,sizeof(b->agent_message),last_agent_status,L"Ajan: ");
        show(b->part_message,sizeof(b->part_message),last_part_status,L"Model parçaları: ");
    }
    void poll() {
        if(preview_only) {
            state=english?L"Menu preview":L"Menü önizlemesi";
            details=english?L"Siesta · Menu preview":L"Siesta · Menü önizlemesi";
            EnableWindow(GetDlgItem(window,Apply),FALSE);
            EnableWindow(GetDlgItem(window,Restore),FALSE);
            EnableWindow(GetDlgItem(window,UndoEdit),edit_history.peek()!=nullptr);
            EnableWindow(GetDlgItem(window,DecorationsButton),catalog::firearm(selected_definition()));
            InvalidateRect(window,nullptr,FALSE);
            return;
        }
        poll_updates();
        menu_host.locate_game(process?process->pid():0);
        if(!watched_game) {
            DWORD pid{};
            const auto game_window=menu_host.game.load();
            if(game_window)GetWindowThreadProcessId(game_window,&pid);
            if(pid)watched_game=OpenProcess(SYNCHRONIZE,FALSE,pid);
        }
        if((watched_game && WaitForSingleObject(watched_game,0)==WAIT_OBJECT_0) ||
           (process && !process->alive())) {
            if(!exit_posted){exit_posted=true;PostMessageW(window,WM_APP+90,0,0);}
            return;
        }
        if(updating || exit_posted)return;
        if(!game && GetTickCount64()>=next_connect) {
            next_connect=GetTickCount64()+2000;
            try {
                const auto pid=Process::find_pid();
                if(pid) {
                    if(!watched_game)watched_game=OpenProcess(SYNCHRONIZE,FALSE,pid);
                    connect();connection_error.clear();
                } else connection_error.clear();
            } catch(const std::exception& e) {
                const auto error=wide(e.what());
                if(connection_error!=error)add_log(error);
                connection_error=error;
            }
        }
        if(!IsWindowVisible(window) || IsIconic(window))return;
        EnableWindow(GetDlgItem(window,UndoEdit),edit_history.peek()!=nullptr);
        EnableWindow(GetDlgItem(window,FavoriteToggle),!visible_finishes.empty());
        const auto old_state=state,old_details=details;
        show_bridge_progress();
        ready = game && selected_definition()!=0 && !visible_finishes.empty();
        if(!game) {
            state=connection_error.empty()?L"Waiting CS2.exe":(english?L"Waiting for CS2 readiness":L"CS2 hazır olması bekleniyor");
            details=connection_error.empty()?(english?L"Connection starts automatically when CS2 opens.":L"CS2 açıldığında otomatik bağlanılır."):connection_error;
        }
        bool can_restore=false;
        if (game) {
            try {
                ready=process->alive() && selected_definition()!=0 && !visible_finishes.empty();
                state=ready?L"Seçim kaydedilebilir":L"Bir eşya ve kaplama seç";
                details=L"Elde olması gerekmez; eline alınca otomatik uygulanır.";
                try {
                    auto active=game->snapshot();
                    auto w=catalog::weapon(active.definition);
                    details=L"Elde: "+(w?wide(localized::weapon(w->definition,english)):L"diğer eşya")+L" · Seçimler ayrı korunur.";
                    can_restore=bridge_client && bridge_client->active_backup() &&
                        catalog::compatible(active.definition,selected_definition());
                } catch(...) {}
                if(glove_selected())can_restore=bridge_client && bridge_client->glove_backup();
                if(agent_selected())can_restore=bridge_client && bridge_client->status() && bridge_client->status()->agent_backup;
            } catch (const std::exception& e) { state = L"Uygulama hazır değil"; details = wide(e.what()); }
        }
        EnableWindow(GetDlgItem(window, Apply), ready);
        EnableWindow(GetDlgItem(window, Restore),can_restore || !profile.empty() || !submitted.empty() || (bridge_client && bridge_client->backup_count()));
        EnableWindow(stat_enabled,!glove_selected() && !agent_selected());EnableWindow(name_tag,!glove_selected() && !agent_selected());
        EnableWindow(GetDlgItem(window,DecorationsButton),catalog::firearm(selected_definition()));
        for(HWND control:{seed,wear,preset})EnableWindow(control,!agent_selected());
        EnableWindow(filter,TRUE);
        EnableWindow(stat_count,!glove_selected() && !agent_selected() && SendMessageW(stat_enabled,BM_GETCHECK,0,0)==BST_CHECKED);
        RECT status{0,px(684),px(1200),px(720)};
        if(old_state!=state || old_details!=details)InvalidateRect(window,&status,FALSE);
    }
    std::string bridge_report() const {
        std::ostringstream out;
        if(bridge_client)out<<bridge_client->loader_report();
        if (bridge_client && bridge_client->status()) {
            const auto* b = bridge_client->status();
            out << "Glove lifetime backups: " << b->glove_backups << " expired pruned=" << b->glove_pruned << '\n';
            out << "Bridge protocol: " << b->protocol << " initialized=" << b->initialized
                << " state=" << b->state << " heartbeat=" << b->heartbeat
                << " stage=" << b->stage << " thread=" << b->frame_thread
                << "\nRequest: " << b->request_id << " completed=" << b->completed_id << " success=" << b->success
                << "\nAutomatic updates: " << b->auto_updates << "\nAutomatic status: " << b->auto_message
                << "\nPhysical attachment updates: " << b->part_updates << "\nAttachment status: " << b->part_message
                << "\nGlove updates: " << b->glove_updates << "\nGlove status: " << b->glove_message << "\nGlove details: " << b->glove_details
                << "\nAgent backup: " << b->agent_backup << "\nAgent status: " << b->agent_message
                << "\nAgent details: " << b->agent_details << "\nAgent model requests: " << b->agent_requests
                << "\nMaterial cache reuses: " << b->material_reuses << "\nHUD row rebuilds: " << b->hud_row_rebuilds
                << "\nKill feed events: " << b->death_events << " local icon requests=" << b->death_icons
                << "\nKill feed status: " << b->death_message
                << "\nAgent voice ready: " << b->voice_ready << " local events=" << b->voice_local_events
                << " matched=" << b->voice_matches << " replaced=" << b->voice_replacements << " errors=" << b->voice_errors
                << "\nAgent voice status: " << b->voice_message
                << "\nHUD language: " << b->hud_language
                << "\nKnife HUD: change-aware labels/icons; 2 second recovery refresh; material validation 750 ms"
                << "\nLast requested stickers: " << b->decorations.stickers[0] << "," << b->decorations.stickers[1] << "," << b->decorations.stickers[2] << "," << b->decorations.stickers[3] << "," << b->decorations.stickers[4]
                << "\nLast requested charm: " << b->decorations.charm << " seed=" << b->decorations.charm_seed
                << "\nGlove backup: " << b->glove_backup
                << "\nMasiva callback elapsed us (frame avg/max): " << (b->frame_work_calls?b->frame_work_us/b->frame_work_calls:0) << "/" << b->frame_max_us
                << "\nMasiva callback elapsed us (HUD avg/max): " << (b->hud_work_calls?b->hud_work_us/b->hud_work_calls:0) << "/" << b->hud_max_us
                << "\nHUD unchanged rows skipped: " << b->hud_skipped
                << "\nStable weapon validations skipped: " << b->watch_checks_skipped
                << "\nExpired weapon backups pruned: " << b->weapon_backups_pruned
                << "\nMissing glove world refreshes: " << b->glove_recoveries
                << "\nHUD updates: " << b->hud_updates << "\nHUD status: " << b->hud_message
                << "\nBridge backups: " << b->pending_backups
                << "\nBridge last message: " << b->message;
            out << "\nTeam preview: ready=" << b->preview_ready << " ticks=" << b->preview_ticks << " thread_skips=" << b->preview_thread_skips << " busy_skips=" << b->preview_busy_skips << " nonlocal=" << b->preview_nonlocal << " local_calls=" << b->preview_local_calls << " updates=" << b->preview_updates << " errors=" << b->preview_errors << " message=" << b->preview_message << "\n";
            out << "Team preview work us (avg/max): " << (b->preview_work_calls?b->preview_work_us/b->preview_work_calls:0) << "/" << b->preview_max_us
                << " idle_skips=" << b->preview_idle_skips << " item_reads_skipped=" << b->preview_item_skips << "\n";
            const auto voice_count=static_cast<unsigned long>(b->voice_history_count);
            out << "\nRecent local voice events (oldest first):";
            for(unsigned long i=voice_count>12?voice_count-12:0;i<voice_count;++i)
                out << "\n  " << b->voice_history[i%12];
            if (b->state == bridge::Done) out << "\nEffective: paint=" << b->effective_paint
                << " seed=" << b->effective_seed << " wear=" << b->effective_wear
                << "\nWorld material entries: " << b->world_materials << " HUD material entries: " << b->hud_materials;
            out << '\n';
        }
        return out.str();
    }
    void report() {
        auto path = pick_file(true, true); if (path.empty()) return;
        std::ostringstream out;
        out << "Masiva Skin " << release::version << " — " << release::codename << " diagnostics\nValidated session PatchVersion: " << (game?std::to_string(game->expected_build()):"not connected")
            << "\nOffset commit: " << offsets::source_commit << "\nOffset timestamp: " << offsets::source_timestamp
            << "\nStatus: " << utf8(state) << "\nDetails: " << utf8(details)
            << "\nAgent: " << (bridge_client && bridge_client->status()?bridge_client->status()->agent_message:"")
            << "\nVisual skin change: NOT VERIFIED BY APPLICATION\n"
            << "Menu: external borderless overlay; key=" << menu_host.key
            << " keyboard_hook=" << (menu_host.keyboard?1:0) << " game_window=" << (menu_host.game?1:0)
            << " language=" << (english?"en":"tr") << " dpi=" << dpi
            << " theme=" << (light_theme?"white":"black") << " accent=" << purple
            << " scope=" << (both()?"Both":SendMessageW(team,CB_GETCURSEL,0,0)==1?"CT":"T") << "\n";
        out << "Update check: " << update_status << '\n';
        if (process) out << "PID: " << process->pid() << '\n';
        try {
            const auto c = selection();
            out << "Selection: definition=" << c.definition << " team=" << c.team << " paint=" << c.paint << " seed=" << c.seed << " wear=" << c.wear << " stattrak=" << c.stattrak << " name=" << std::quoted(c.custom_name) << '\n';
        } catch (const std::exception& e) { out << "Input error: " << e.what() << '\n'; }
        if (game) {
            out << "Pending bridge backups: " << (bridge_client ? bridge_client->backup_count() : 0) << '\n';
            try {
                const auto s = game->snapshot(); const auto v = game->values(s);
                out << "Active definition: " << s.definition << "\nActive handle: " << s.handle << "\nReadback: paint=" << v.paint << " seed=" << v.seed
                    << " wear=" << v.wear << " item_id_high=" << v.item_id_high << '\n';
                auto name=process->get<ItemLabel>(s.item+0x399);name.back()=0;
                out<<"Local name override: "<<name.data()<<'\n';
            } catch (const std::exception& e) { out << "Snapshot error: " << e.what() << '\n'; }
        }
        if(game)try {
            const auto g=game->glove_snapshot();
            out<<"Glove definition: "<<g.definition<<" initialized="<<static_cast<int>(process->get<std::uint8_t>(g.item+0x1e8))
                <<" refresh_pending="<<static_cast<int>(process->get<std::uint8_t>(g.pawn+0x179d))
                <<" model_handle="<<process->get<std::uint32_t>(g.pawn+0x15f8)<<'\n';
        } catch(const std::exception& e) { out<<"Glove snapshot: "<<e.what()<<'\n'; }
        out << "Preview cache: " << previews.diagnostics() << "\nUI resources: GDI=" << GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS) << " USER=" << GetGuiResources(GetCurrentProcess(),GR_USEROBJECTS) << '\n';
        out << "Material regeneration: GAME-THREAD BRIDGE IMPLEMENTED; VISUAL RESULT UNVERIFIED\n";
        out << (bridge_client ? bridge_report() : last_bridge_report);
        out << "\n" << utf8(entries);
        save_text(path, out.str()); add_log(L"Tanı raporu kaydedildi.");
    }
    void command(int id, int notification) {
        try {
            if(updating)return;
            if(page_open&&notification==BN_CLICKED&&(id==Home||id==Load||id==Save||id==Settings||id==Recent)){if(page::close(window))PostMessageW(window,WM_COMMAND,MAKEWPARAM(id,BN_CLICKED),0);
                return;}
            if((id==Seed || id==Wear || id==NameTag || id==StatCount) && notification==EN_CHANGE){SetTimer(window,3,350,nullptr);return;}
            if((id==Seed || id==Wear || id==NameTag || id==StatCount) && notification==EN_KILLFOCUS){track_edit();return;}
            if(notification==BN_CLICKED) {
                if(id==Recent){open_collection(true,true);return;}
                if(id==Home){settings_open=false;customize_open=false;appearance_open=false;resize();return;}
                if(id==Appearance){appearance_open=true;advanced=false;resize();return;}
                if(id==GeneralSettings){appearance_open=false;resize();return;}
                if(id==MainColor){choose_color(true);return;}
                if(id==ResetMainColor){custom_main=false;update_theme();return;}
                if(id==PalettePreset){theme_presets();return;}
                if(id==ClearFilters){catalog_filter={};favorites_only=false;SetWindowTextW(filter,L"");reload_finishes(0,true);notify(english?L"Filters cleared":L"Filtreler temizlendi");return;}
                if(id==DiscordPresence){rpc::set_enabled(!rpc::is_enabled());InvalidateRect(GetDlgItem(window,DiscordPresence),nullptr,FALSE);notify(english?L"Discord setting saved":L"Discord ayarı kaydedildi");return;}
                if(id==AutoFloat){open_auto_float();return;}
                if(id==ModelPicker){open_model_picker();return;}
                if(id==FavoriteToggle){favorite_toggle();return;}
                if(id==FavoritesOnly){favorites_only=!favorites_only;reload_finishes(0,true);RedrawWindow(window,nullptr,nullptr,RDW_INVALIDATE|RDW_ALLCHILDREN);poll();return;}
                if(id==UndoEdit){undo_edit();poll();return;}
                if(id==TeamT || id==TeamCT || id==TeamBoth) {SendMessageW(team,CB_SETCURSEL,id==TeamBoth?2:id==TeamT?0:1,0);command(Team,CBN_SELCHANGE);RedrawWindow(window,nullptr,nullptr,RDW_INVALIDATE|RDW_ALLCHILDREN);return;}
                if(id>=TabWeapons && id<=TabAgents) {if(both() && id==TabAgents)return;SendMessageW(group,CB_SETCURSEL,id-TabWeapons,0);command(Group,CBN_SELCHANGE);RedrawWindow(window,nullptr,nullptr,RDW_INVALIDATE|RDW_ALLCHILDREN);return;}
                if(id==PreviousPage || id==NextPage) {turn_page(id==NextPage?1:-1);return;}
                if(id==MenuKey) {menu_host.capturing=true;SetFocus(window);refresh_key_label();return;}
                if(id==HideMenu) {menu_host.hide();refresh_key_label();return;}
                if(id==Settings) {settings_open=!settings_open;customize_open=false;appearance_open=false;resize();SetFocus(GetDlgItem(window,Settings));return;}
                if(id==Customize) {customize_open=!customize_open;settings_open=false;resize();SetFocus(GetDlgItem(window,Customize));return;}
                if(id==ExitMenu) {exit_posted=true;disconnect();DestroyWindow(window);return;}
            }
            if(id==Group && notification==CBN_SELCHANGE) {
                if(agents_view()){favorites_only=false;catalog_filter={};}
                updating=true;reload_weapons();SetWindowTextW(filter,L"");reload_finishes();updating=false;restore_selection();track_edit();resize();poll();return;
            }
            if(id==Weapon && notification==CBN_SELCHANGE) {
                updating=true;SetWindowTextW(filter,L"");reload_finishes();updating=false;restore_selection();track_edit();resize();poll();return;
            }
            if(id==Team && notification==CBN_SELCHANGE) {
                if(agents_view()){favorites_only=false;catalog_filter={};}
                if(both() && SendMessageW(group,CB_GETCURSEL,0,0)==3)SendMessageW(group,CB_SETCURSEL,0,0);
                scope_controls();
                const auto previous=selected_definition();updating=true;reload_weapons(previous);SetWindowTextW(filter,L"");reload_finishes();updating=false;restore_selection();track_edit();resize();poll();return;
            }
            if(id==Opacity && notification==CBN_SELCHANGE) {
                const auto selected=SendMessageW(opacity_control,CB_GETCURSEL,0,0);
                if(selected>=0){opacity_percent=50+static_cast<int>(selected)*10;fading=false;
                    SetLayeredWindowAttributes(window,0,static_cast<BYTE>(ui::alpha(opacity_percent)),LWA_ALPHA);
                    WritePrivateProfileStringW(L"UI",L"Opacity",std::to_wstring(opacity_percent).c_str(),settings_path().c_str());}
                return;
            }
            if(id==MenuScale && notification==CBN_SELCHANGE) {
                const auto selected=SendMessageW(scale_control,CB_GETCURSEL,0,0);
                if(selected>=0){scale_percent=70+static_cast<int>(selected)*10;reflow();
                    WritePrivateProfileStringW(L"UI",L"Scale",std::to_wstring(scale_percent).c_str(),settings_path().c_str());}
                return;
            }
            if(id==Language && notification==CBN_SELCHANGE) { language_changed();return; }
            if(id==StatEnabled && notification==BN_CLICKED) { track_edit();poll();return; }
            if(id==Filter && notification==EN_CHANGE) {
                auto i=SendMessageW(preset,CB_GETCURSEL,0,0);
                int paint=i>=0 && static_cast<std::size_t>(i)<visible_finishes.size()?visible_finishes[i]->paint:0;
                reload_finishes(paint,true);poll();return;
            }
            if (id == Preset && notification == CBN_SELCHANGE) { track_edit();choose_preset();track_edit();return; }
            if (notification != BN_CLICKED) return;
            switch (id) {
                case UpdateCheck: check_updates();break;
                case ThemeMode: custom_main=false;light_theme=!light_theme;update_theme();break;
                case AccentColor: choose_color();break;
                case AdvancedFilters: {
                    PageScope page(*this,AdvancedFilters);
                    auto i=SendMessageW(preset,CB_GETCURSEL,0,0);int paint=i>=0&&static_cast<std::size_t>(i)<visible_finishes.size()?visible_finishes[i]->paint:0;
                    if(FilterEditor::edit(window,catalog_filter,english,dpi,body,palette)){reload_finishes(paint,true);poll();RedrawWindow(window,nullptr,nullptr,RDW_INVALIDATE|RDW_ALLCHILDREN);}
                    break;
                }
                case DecorationsButton: {
                    PageScope page(*this,DecorationsButton);
                    track_edit();
                    if(DecorationEditor::edit(window,decoration_selection,english,dpi,body,previews,palette)) {
                        track_edit();const auto c=selection();remember(c);
                        feedback=L"Sticker / Charm seçimi hazır; Uygula de.";
                        if(bridge_client) {
                            activate_profile();
                            add_log(wide(submit(c)));remember_targets(submitted,c,both());
                            feedback=L"Sticker / Charm seçimi oyuna gönderildi.";
                        }
                        add_log(feedback);InvalidateRect(window,nullptr,FALSE);
                    }
                    break;
                }
                case Advanced: advanced=!advanced;resize();break;
                
                case Apply: {
                    track_edit();
                    if (!game) connect();
                    activate_profile();
                    const auto c = selection();
                    SetWindowTextW(seed, std::to_wstring(c.seed).c_str());
                    if (!bridge_client) throw std::runtime_error("Oyun ici kopru yok.");
                    add_log(L"Oyun içi köprü ve materyal isteği bekleniyor...");
                    UpdateWindow(window);
                    add_log(wide(submit(c)));
                    add_log(L"İstek: paint=" + std::to_wstring(c.paint) + L", seed=" + std::to_wstring(c.seed) + L", float=" + std::to_wstring(c.wear));
                    remember(c);remember_targets(submitted,c,both());try{remember_recent(c);}catch(const std::exception& e){add_log(wide(e.what()));}notify(english?L"Selection sent. Waiting for the matching item.":L"Seçim gönderildi. Uygun eşya bekleniyor.");feedback=L"Seçim kaydedildi · uygun eşya elde olduğunda uygulanır.";
                    InvalidateRect(window,nullptr,FALSE);
                    break;
                }
                case Restore:
                    if(bridge_client) add_log(wide(bridge_client->request(bridge::RestoreAll,WeaponState{})));
                    profile.clear();submitted.clear();profile_pending=false;active_profile.clear();
                    feedback=english?L"All selections cleared. Holstered weapons restore when equipped.":L"Tüm seçimler temizlendi. Elde olmayan silahlar ele alınınca geri döner.";
                    notify(feedback);poll();break;
                case Save: open_collection(false); break;
                case Load: open_collection(true); break;
                case Report: poll(); report(); break;
                default: return;
            }
        } catch (const std::exception& e) {
            updating=false;feedback=L"İşlem tamamlanamadı; aşağıdaki hata mesajını kontrol et.";
            add_log(L"İşlem tamamlanamadı: " + wide(e.what()));
            notify(wide(e.what()),true);
        }
        poll();
    }
    void refresh_key_label() {
        wchar_t name[64]{};
        const auto scan=MapVirtualKeyW(menu_host.key,MAPVK_VK_TO_VSC);
        const bool extended=menu_host.key>=VK_PRIOR && menu_host.key<=VK_DELETE;
        GetKeyNameTextW(static_cast<LONG>((scan<<16)|(extended?1U<<24:0)),name,64);
        std::wstring caption=menu_host.capturing?tr(L"Bir tuşa bas · Esc iptal"):
            tr(L"Menü tuşu: ")+(name[0]?name:std::to_wstring(menu_host.key));
        SetWindowTextW(GetDlgItem(window,MenuKey),caption.c_str());
    }
    void bind_key(unsigned key) {
        menu_host.capturing=false;
        if(key!=VK_ESCAPE && menu::valid_key(key)) {
            menu_host.key=key;
            if(!WritePrivateProfileStringW(L"UI",L"MenuKey",std::to_wstring(key).c_str(),settings_path().c_str()))
                add_log(english?L"Menu key changed for this session; settings could not be saved.":L"Tuş değişti; ayar dosyası kaydedilemedi.");
        }
        refresh_key_label();
    }
    void turn_page(int direction) {
        if(direction<0){if(gallery_page)--gallery_page;}
        else gallery_page=menu::clamp_page(gallery_page+1,visible_finishes.size());
        SetFocus(window);InvalidateRect(window,nullptr,FALSE);
    }
    bool gallery_click(int x,int y) {
        if(!settings_open&&!page_open&&!customize_open){const int lx=MulDiv(x,96,dpi),ly=MulDiv(y,96,dpi);for(const auto& chip:filter_chips)if(chip.first.contains(lx,ly)){
            switch(chip.second){case 1:favorites_only=false;break;case 2:catalog_filter.rarity=0;break;case 3:catalog_filter.collection=0;break;case 4:catalog_filter.phase=0;break;case 5:catalog_filter.stattrak=false;break;case 6:SetWindowTextW(filter,L"");break;}
            reload_finishes(0,true);return true;}}

        if(settings_open||page_open||customize_open)return false;
        x=MulDiv(x,96,dpi);y=MulDiv(y,96,dpi);
        for(std::size_t card=0;card<menu::page_size;++card) {
            const auto area=menu::card_rect(card);
            if(area.contains(x,y)) {
                const auto index=menu::card_index(gallery_page,card,visible_finishes.size());
                if(index<visible_finishes.size()) {track_edit();SendMessageW(preset,CB_SETCURSEL,index,0);choose_preset();track_edit();SetFocus(window);InvalidateRect(GetDlgItem(window,FavoriteToggle),nullptr,FALSE);poll();}
                return true;
            }
        }
        return false;
    }
    void reflow() {
        const bool was_visible=IsWindowVisible(window)!=FALSE;
        MONITORINFO monitor{};monitor.cbSize=sizeof(monitor);
        GetMonitorInfoW(MonitorFromWindow(window,MONITOR_DEFAULTTONEAREST),&monitor);
        dpi=ui::fitted_dpi(system_dpi,scale_percent,monitor.rcWork.right-monitor.rcWork.left,monitor.rcWork.bottom-monitor.rcWork.top);
        auto old_body=body,old_title=title,old_medium=medium,old_small=small;
        auto old_brand=brand_font,old_release=release_font;
        embedded_fonts.load();
        brand_font=font(27,FW_EXTRABOLD,L"Oxanium ExtraBold");release_font=font(17,FW_MEDIUM,L"Rajdhani");
        body=font(14);title=font(27,FW_BOLD);medium=font(18,FW_SEMIBOLD);small=font(12);
        SendMessageW(window,WM_SETREDRAW,FALSE,0);
        for(const auto& p:placements){SetWindowPos(p.handle,nullptr,px(p.x),px(p.y),px(p.w),px(p.h),SWP_NOZORDER|SWP_NOACTIVATE);SendMessageW(p.handle,WM_SETFONT,reinterpret_cast<WPARAM>(body),FALSE);}
        DeleteObject(old_brand);DeleteObject(old_release);
        DeleteObject(old_body);DeleteObject(old_title);DeleteObject(old_medium);DeleteObject(old_small);
        if(portrait)DestroyIcon(portrait);
        portrait=static_cast<HICON>(LoadImageW(GetModuleHandleW(nullptr),MAKEINTRESOURCEW(101),IMAGE_ICON,px(64),px(64),0));
        resize();
        RECT bounds{};GetWindowRect(window,&bounds);
        SetWindowPos(window,nullptr,std::clamp<LONG>(bounds.left,monitor.rcWork.left,std::max(monitor.rcWork.left,monitor.rcWork.right-px(1200))),
            std::clamp<LONG>(bounds.top,monitor.rcWork.top,std::max(monitor.rcWork.top,monitor.rcWork.bottom-px(720))),0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
        SendMessageW(window,WM_SETREDRAW,TRUE,0);
        if(!was_visible)ShowWindow(window,SW_HIDE);
        RedrawWindow(window,nullptr,nullptr,RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_ERASE);
    }
    void place(int id,int x,int y,int w,int h=38){for(auto& p:placements)if(GetDlgCtrlID(p.handle)==id){p.x=x;p.y=y;p.w=w;p.h=h;SetWindowPos(p.handle,nullptr,px(x),px(y),px(w),px(h),SWP_NOZORDER|SWP_NOACTIVATE);return;}}
    void resize() {
        SetWindowPos(window,nullptr,0,0,px(1200),px(720),SWP_NOMOVE|SWP_NOZORDER);
        const auto visible=[&](int id,bool show){ShowWindow(GetDlgItem(window,id),show?SW_SHOW:SW_HIDE);};
        const bool home=!settings_open&&!page_open&&!customize_open,appearance=settings_open&&appearance_open,general=settings_open&&!appearance_open;
        for(int id:{TeamT,TeamCT,TeamBoth,TabWeapons,TabKnives,TabGloves,TabAgents,ModelPicker,Filter,FavoritesOnly,AdvancedFilters,PreviousPage,NextPage,Recent,ClearFilters,FavoriteToggle,Restore})visible(id,home);
        for(int id:{NameTag,StatEnabled,StatCount,Seed,Wear})visible(id,customize_open&&!page_open);
        visible(Customize,home&&!agents_view());visible(DecorationsButton,!settings_open&&!page_open&&!agents_view());
        visible(Apply,home||customize_open);visible(UndoEdit,home||customize_open);visible(AdvancedFilters,home&&!agents_view());visible(AutoFloat,home&&!agents_view());visible(Weapon,false);visible(Home,!home);
        for(int id:{Appearance,GeneralSettings})visible(id,settings_open);
        for(int id:{ThemeMode,AccentColor,Opacity,MenuScale,PalettePreset,MainColor,ResetMainColor})visible(id,appearance);
        for(int id:{Language,MenuKey,Advanced,DiscordPresence})visible(id,general);
        for(int id:{Log,Report,UpdateCheck})visible(id,general&&advanced);
        EnableWindow(GetDlgItem(window,ModelPicker),!agents_view());
        place(Appearance,20,208,238,52);place(GeneralSettings,20,152,238,42);
        place(MainColor,610,190,240);place(ResetMainColor,866,190,200);place(ThemeMode,610,286,130);place(AccentColor,754,286,144);place(PalettePreset,912,286,154);
        place(Opacity,610,400,240,160);place(MenuScale,610,490,240,200);place(Language,610,190,220,120);place(MenuKey,610,276,330);place(DiscordPresence,610,358,330);place(Advanced,300,420,250);
        place(Log,300,478,850,150);place(Report,300,638,240);place(UpdateCheck,560,638,270);
        place(NameTag,738,228,404,34);place(StatEnabled,738,300,150,26);place(StatCount,994,300,148,30);place(Seed,738,390,180,30);place(Wear,964,390,180,30);
        place(DecorationsButton,customize_open?738:864,customize_open?478:422,customize_open?404:320);
        scope_controls();SetWindowTextW(GetDlgItem(window,Advanced),advanced?L"Günlüğü kapat":L"Günlük / tanı");RedrawWindow(window,nullptr,nullptr,RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_ERASE);
    }
    void create() {
        HDC dc=GetDC(window);dpi=GetDeviceCaps(dc,LOGPIXELSX);ReleaseDC(window,dc);
        using DpiWindowFn=UINT(WINAPI*)(HWND);auto get_dpi=reinterpret_cast<DpiWindowFn>(reinterpret_cast<void*>(GetProcAddress(GetModuleHandleW(L"user32.dll"),"GetDpiForWindow")));if(get_dpi)dpi=static_cast<int>(get_dpi(window));
        MONITORINFO monitor{};monitor.cbSize=sizeof(monitor);GetMonitorInfoW(MonitorFromWindow(window,MONITOR_DEFAULTTONEAREST),&monitor);
        system_dpi=dpi;
        opacity_percent=ui::opacity(static_cast<int>(GetPrivateProfileIntW(L"UI",L"Opacity",100,settings_path().c_str())));
        scale_percent=ui::scale(static_cast<int>(GetPrivateProfileIntW(L"UI",L"Scale",100,settings_path().c_str())));
        opacity_percent=50+((opacity_percent-50+5)/10)*10;
        scale_percent=70+((scale_percent-70+5)/10)*10;
        dpi=ui::fitted_dpi(system_dpi,scale_percent,monitor.rcWork.right-monitor.rcWork.left,monitor.rcWork.bottom-monitor.rcWork.top);
        embedded_fonts.load();
        brand_font=font(27,FW_EXTRABOLD,L"Oxanium ExtraBold");release_font=font(17,FW_MEDIUM,L"Rajdhani");
        body=font(14);title=font(27,FW_BOLD);medium=font(18,FW_SEMIBOLD);small=font(12);
        updating=true;
        custom_main=GetPrivateProfileIntW(L"UI",L"CustomMain",0,settings_path().c_str())!=0;main_color=GetPrivateProfileIntW(L"UI",L"MainColor",RGB(7,8,11),settings_path().c_str())&0xffffff;
        light_theme=GetPrivateProfileIntW(L"UI",L"LightTheme",0,settings_path().c_str())!=0;
        purple=GetPrivateProfileIntW(L"UI",L"Accent",RGB(137,66,230),settings_path().c_str())&0xffffff;
        portrait=static_cast<HICON>(LoadImageW(GetModuleHandleW(nullptr),MAKEINTRESOURCEW(101),IMAGE_ICON,px(64),px(64),0));
        wchar_t lang[8]{};GetPrivateProfileStringW(L"UI",L"OverlayLanguage",L"en",lang,8,settings_path().c_str());english=std::wstring(lang)!=L"tr";
        language=control(L"COMBOBOX",L"",Language,380,244,200,120,CBS_DROPDOWNLIST|WS_TABSTOP);
        SendMessageW(language,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(L"Türkçe"));SendMessageW(language,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(L"English"));
        SendMessageW(language,CB_SETCURSEL,english?1:0,0);
        button(L"Menü rengi",AccentColor,624,162,240);
        button(L"Menü tuşu: Insert",MenuKey,624,238,240);button(L"Gizle",HideMenu,1084,20,40);button(L"Çıkış",ExitMenu,1140,20,40);
        button(L"Ayarlar",Settings,988,28,44);
        team=control(L"COMBOBOX",L"",Team,0,0,0,120,CBS_DROPDOWNLIST);
        SendMessageW(team,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(L"T"));SendMessageW(team,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(L"CT"));SendMessageW(team,CB_SETCURSEL,0,0);
        button(L"T",TeamT,184,104,52);button(L"CT",TeamCT,236,104,52);button(L"Both",TeamBoth,288,104,68);
        SendMessageW(team,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(L"Both"));
        group=control(L"COMBOBOX",L"",Group,0,0,0,180,CBS_DROPDOWNLIST);
        for(auto name:{L"Silahlar",L"Bıçaklar",L"Eldivenler",L"Ajanlar"}) {auto t=tr(name);SendMessageW(group,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(t.c_str()));}
        SendMessageW(group,CB_SETCURSEL,0,0);
        button(L"Silahlar",TabWeapons,12,116,144);button(L"Bıçaklar",TabKnives,12,168,144);
        button(L"Eldivenler",TabGloves,12,220,144);button(L"Ajanlar",TabAgents,12,272,144);
        button(L"Tema: Siyah",ThemeMode,380,162,200);
        button(L"Back",Home,574,28,148);button(L"Loadout",Load,736,28,108);button(L"Profiller",Save,856,28,108);
        button(L"Filtreler",AdvancedFilters,736,104,88);button(L"Recent",Recent,12,382,144);
        button(L"Theme presets",PalettePreset,880,162,260);
        button(L"Menu & Theme Colour",Appearance,20,208,238);button(L"General",GeneralSettings,20,152,238);button(L"Main menu colour",MainColor,610,190,240);button(L"Use theme colour",ResetMainColor,866,190,200);
        control(L"BUTTON",L"Clear",ClearFilters,760,151,64,26,BS_OWNERDRAW|WS_TABSTOP);
        weapon=control(L"COMBOBOX",L"",Weapon,394,115,306,350,CBS_DROPDOWNLIST|WS_VSCROLL|WS_TABSTOP);
        filter=control(L"EDIT",L"",Filter,592,114,120,22,ES_AUTOHSCROLL|WS_TABSTOP);
        control(L"BUTTON",L"Favoriler",FavoritesOnly,12,332,144,38,BS_OWNERDRAW|WS_TABSTOP);
        const auto cue=tr(L"Kaplama ara…");SendMessageW(filter,EM_SETCUEBANNER,FALSE,reinterpret_cast<LPARAM>(cue.c_str()));SendMessageW(filter,EM_SETLIMITTEXT,80,0);
        preset=control(L"COMBOBOX",L"",Preset,0,0,0,400,CBS_DROPDOWNLIST);
        for(HWND h:{team,group,preset,weapon})ShowWindow(h,SW_HIDE);
        button(L"",ModelPicker,382,104,170);
        control(L"BUTTON",L"Auto Float",AutoFloat,864,165,200,30,BS_OWNERDRAW|WS_TABSTOP);
        for(const auto id:{PreviousPage,NextPage}) {
            const auto r=menu::page_button_rect(id==NextPage);
            control(L"BUTTON",id==PreviousPage?L"‹":L"›",id,r.x,r.y,r.w,r.h,BS_OWNERDRAW|WS_TABSTOP);
        }
        button(L"Discord Rich Presence",DiscordPresence,462,420,240);button(L"Günlük / tanı",Advanced,210,420,240);button(L"Son değişikliği geri al",UndoEdit,864,636,116);button(L"Tanı raporu",Report,210,644,236);button(L"Güncelleme kontrolü",UpdateCheck,458,644,242);
        button(L"Özelleştir",Customize,864,370,320);
        log=control(L"EDIT",L"",Log,210,474,774,156,ES_MULTILINE|ES_READONLY|ES_AUTOVSCROLL|WS_VSCROLL);
        control(L"BUTTON",L"Favori",FavoriteToggle,1144,100,40,38,BS_OWNERDRAW|WS_TABSTOP);
        name_tag=control(L"EDIT",L"",NameTag,880,436,288,26,ES_AUTOHSCROLL|WS_TABSTOP);SendMessageW(name_tag,EM_SETLIMITTEXT,80,0);
        stat_enabled=control(L"BUTTON",L"StatTrak™",StatEnabled,880,476,108,24,BS_AUTOCHECKBOX|WS_TABSTOP);
        stat_count=control(L"EDIT",L"0",StatCount,1028,474,140,26,ES_AUTOHSCROLL|WS_TABSTOP);SendMessageW(stat_count,EM_SETLIMITTEXT,6,0);
        seed=control(L"EDIT",L"0",Seed,880,534,128,26,ES_AUTOHSCROLL|WS_TABSTOP);
        wear=control(L"EDIT",L"0.01",Wear,1040,534,128,26,ES_AUTOHSCROLL|WS_TABSTOP);
        for(HWND edit:{seed,wear})SendMessageW(edit,EM_SETLIMITTEXT,24,0);
        button(L"Sticker / Charm",DecorationsButton,864,570,320);
        button(L"Uygula",Apply,992,636,192);button(L"Geri al",Restore,16,636,136);
        opacity_control=control(L"COMBOBOX",L"",Opacity,380,336,200,160,CBS_DROPDOWNLIST|WS_TABSTOP);
        scale_control=control(L"COMBOBOX",L"",MenuScale,624,336,240,200,CBS_DROPDOWNLIST|WS_TABSTOP);
        for(int v=50;v<=100;v+=10){auto n=std::to_wstring(v)+L"%";SendMessageW(opacity_control,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(n.c_str()));}
        for(int v=70;v<=140;v+=10){auto n=std::to_wstring(v)+L"%";SendMessageW(scale_control,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(n.c_str()));}
        SendMessageW(opacity_control,CB_SETCURSEL,(opacity_percent-50)/10,0);
        SendMessageW(scale_control,CB_SETCURSEL,(scale_percent-70)/10,0);
        try {auto path=settings_path().parent_path()/L"Masiva-Favorites.txt";if(std::filesystem::exists(path)){if(std::filesystem::file_size(path)>1048576)throw std::runtime_error("Favorites file too large");std::ifstream in(path);favorites.read(in);}}
        catch(const std::exception& e){add_log(wide(e.what()));}
        try {auto path=settings_path().parent_path()/L"Masiva-Recent.txt";if(std::filesystem::exists(path)&&std::filesystem::file_size(path)<4096){std::ifstream in(path);int d,p;while(recent.size()<20 && in>>d>>p)if(catalog::finish(d,p))recent.push_back({d,p});}}catch(const std::exception& e){add_log(wide(e.what()));}
        put_selection({"T","weapon",7,474,0,0.01f});edit_history.reset(selection(),both());
        const auto selected_key=GetPrivateProfileIntW(L"UI",L"MenuKey",menu::default_key,settings_path().c_str());
        if(!menu_host.install(window,selected_key))add_log(english?L"Menu key unavailable. Use the tray icon to open the menu.":L"Menü tuşu etkinleşmedi. Tepsi simgesini kullan.");
        refresh_key_label();
        add_log(release::window_title);
        add_log(english?L"Use borderless fullscreen. Menu key opens and closes this overlay.":L"Pencereli tam ekran kullan. Menü tuşuyla açıp kapatabilirsin.");
        updating=false;update_theme(false);resize();if(!preview_only)SetTimer(window,1,900,nullptr);poll();if(!preview_only)check_updates();
    }
    void label(HDC dc, const std::wstring& value, int x, int y, int w, int h, HFONT f, COLORREF c = text) {
        RECT r{px(x), px(y), px(x + w), px(y + h)};
        auto old = SelectObject(dc, f); SetTextColor(dc, c); SetBkMode(dc, TRANSPARENT);
        const auto translated=tr(value);
        DrawTextW(dc, translated.c_str(), -1, &r, DT_LEFT | DT_WORDBREAK | DT_END_ELLIPSIS | DT_NOPREFIX);
        SelectObject(dc, old);
    }
    void box(HDC dc, int x, int y, int w, int h) {
        auto border=CreatePen(PS_SOLID,1,light_theme?RGB(220,222,228):RGB(34,34,34));
        auto brush=SelectObject(dc,surface);auto pen=SelectObject(dc,border);
        RoundRect(dc,px(x),px(y),px(x+w),px(y+h),px(12),px(12));
        SelectObject(dc,brush);SelectObject(dc,pen);DeleteObject(border);
    }
    void preview(HDC dc,int definition,int paint,int x,int y,int w,int h) {
        RECT area{px(x),px(y),px(x+w),px(y+h)};
        if(!RectVisible(dc,&area))return;
        if(!previews.draw(dc,definition,paint,area))label(dc,L"Görsel yok",x,y+h/2-9,w,24,small,muted);
    }
    COLORREF animated_title() const {
        const double phase=static_cast<double>(GetTickCount64()%12000)/12000.0*6.283185307;
        const double base=light_theme?85:185, amplitude=light_theme?55:65;
        return RGB(static_cast<int>(base+amplitude*std::sin(phase)),
                   static_cast<int>(base+amplitude*std::sin(phase+2.094395102)),
                   static_cast<int>(base+amplitude*std::sin(phase+4.188790205)));
    }
    void animate() {
        if(!IsWindowVisible(window) || IsIconic(window))return;
        POINT pointer{};GetCursorPos(&pointer);const HWND pointed=WindowFromPoint(pointer);HWND hovered=pointed;
        if(GetParent(hovered)!=window || (GetWindowLongPtrW(hovered,GWL_STYLE)&BS_TYPEMASK)!=BS_OWNERDRAW)hovered=nullptr;
        if(hovered!=hovered_button){if(hovered_button)InvalidateRect(hovered_button,nullptr,FALSE);hovered_button=hovered;if(hovered_button)InvalidateRect(hovered_button,nullptr,FALSE);}
        POINT local=pointer;ScreenToClient(window,&local);int card_hover=-1;
        if(!settings_open && !page_open && !customize_open && pointed==window) {
            for(std::size_t c=0;c<menu::page_size;++c)if(menu::card_rect(c).contains(MulDiv(local.x,96,dpi),MulDiv(local.y,96,dpi)))card_hover=static_cast<int>(c);
        }
        hovered_card=card_hover;
        const auto now=GetTickCount64();const auto dt=last_animation?std::min<ULONGLONG>(now-last_animation,60):16;last_animation=now;
        for(std::size_t i=0;i<hover_levels.size();++i){const float target=static_cast<int>(i)==hovered_card?1.f:0.f;const auto before=hover_levels[i];
            const float step=static_cast<float>(dt)/150.f;hover_levels[i]=before<target?std::min(target,before+step):std::max(target,before-step);
            if(before!=hover_levels[i]){auto c=menu::card_rect(i);RECT r{px(c.x),px(c.y),px(c.x+c.w),px(c.y+c.h)};InvalidateRect(window,&r,FALSE);}}
        if(toast_until && now>=toast_until){toast_until=0;toast_text.clear();RECT r{px(20),px(576),px(1180),px(630)};InvalidateRect(window,&r,FALSE);}
        if(fading) {
            const auto elapsed=GetTickCount64()-animation_started;
            const double t=std::min(1.0,static_cast<double>(elapsed)/180.0);
            const auto alpha=static_cast<BYTE>(ui::alpha(opacity_percent)*(0.75+0.25*(1-(1-t)*(1-t))));
            SetLayeredWindowAttributes(window,0,alpha,LWA_ALPHA);
            fading=t<1;
        }
        RECT header{px(100),px(54),px(500),px(76)};
        if(now-last_header>=80){last_header=now;InvalidateRect(window,&header,FALSE);}
    }
    void plate(HDC dc,int x,int y,int w,int h,COLORREF top,COLORREF bottom,COLORREF edge,int radius=8,int stroke=1) {
        RECT r{px(x),px(y),px(x+w),px(y+h)};studio::rounded(dc,r,top,bottom,edge,static_cast<float>(px(radius)),static_cast<float>(px(stroke)));
    }
    void rule(HDC dc,int x,int y,int xx,int yy) {
        auto pen=CreatePen(PS_SOLID,1,light_theme?RGB(211,214,223):RGB(37,38,43));auto old=SelectObject(dc,pen);
        MoveToEx(dc,px(x),px(y),nullptr);LineTo(dc,px(xx),px(yy));SelectObject(dc,old);DeleteObject(pen);
    }
    void draw(HDC dc) {
        RECT whole{};GetClientRect(window,&whole);FillRect(dc,&whole,background);
        const auto edge=light_theme?RGB(204,209,220):RGB(49,50,57);
        const auto field=palette.card;
        plate(dc,16,8,72,72,palette.panel,palette.panel,ui::mix(purple,bg,35),12,1);
        if(portrait)DrawIconEx(dc,px(20),px(12),portrait,px(64),px(64),0,nullptr,DI_NORMAL);
        label(dc,L"MASIVA SKIN",100,18,460,34,brand_font);
        label(dc,wide(release::subtitle()),100,54,400,22,release_font,animated_title());
        rule(dc,0,88,1200,88);rule(dc,0,684,1200,684);
        const auto dot=CreateSolidBrush(game?RGB(75,202,130):RGB(226,172,75));auto old=SelectObject(dc,dot);auto pen=SelectObject(dc,GetStockObject(NULL_PEN));
        Ellipse(dc,px(14),px(699),px(22),px(707));SelectObject(dc,old);SelectObject(dc,pen);DeleteObject(dot);
        label(dc,game?(english?L"Connected to CS2":L"CS2 bağlantısı hazır"):state,32,695,360,20,small,muted);
        label(dc,game?feedback:details,400,695,780,20,small,muted);
        if(page_open){draw_toast(dc);return;}
        if(customize_open){
            label(dc,english?L"Customize selection":L"Seçimi özelleştir",210,112,800,40,title);
            const auto i=SendMessageW(preset,CB_GETCURSEL,0,0);if(i>=0&&static_cast<std::size_t>(i)<visible_finishes.size()){const auto* f=visible_finishes[i];plate(dc,210,176,460,370,palette.panel,palette.panel,edge,12);preview(dc,f->definition,f->paint,230,210,420,250);label(dc,wide(item_label(f->definition,f->paint,english).data()),230,478,420,52,medium);}
            label(dc,english?L"Name tag":L"İsim etiketi",738,200,400,22,small,muted);label(dc,english?L"Pattern / seed":L"Desen / seed",738,362,180,22,small,muted);label(dc,english?L"Wear / float":L"Aşınma / float",964,362,180,22,small,muted);draw_toast(dc);return;
        }
        if(settings_open){
            rule(dc,278,88,278,684);label(dc,english?L"SETTINGS":L"AYARLAR",24,112,240,26,medium);label(dc,appearance_open?L"Menu & Theme Colour":(english?L"General settings":L"Genel ayarlar"),300,112,830,36,title);
            if(appearance_open){for(int y:{164,258,370,460})plate(dc,300,y,850,y==258?94:78,palette.panel,palette.panel,edge,10);
                label(dc,english?L"1  Main menu colour":L"1  Ana menü rengi",320,188,270,30,medium);label(dc,english?L"2  Theme & accent":L"2  Tema ve vurgu",320,286,270,30,medium);label(dc,english?L"3  Opacity":L"3  Opaklık",320,396,270,30,medium);label(dc,english?L"4  Size":L"4  Boyut",320,486,270,30,medium);
                label(dc,english?L"Main colour changes the surfaces. Accent controls borders and highlights.":L"Ana renk menü yüzeylerini; vurgu rengi çerçeveleri ve seçili alanları değiştirir.",320,566,810,54,body,muted);
            }else{label(dc,english?L"Language":L"Dil",320,194,270,24,medium);label(dc,english?L"Menu shortcut":L"Menü kısayolu",320,282,270,24,medium);label(dc,L"Discord",320,364,270,24,medium);}draw_toast(dc);return;
        }
        plate(dc,848,96,344,528,palette.panel,palette.panel,edge,12);
        rule(dc,168,88,168,684);rule(dc,844,88,844,684);
        std::vector<std::pair<int,std::wstring>> tags;
        if(favorites_only)tags.push_back({1,english?L"Favorites":L"Favoriler"});
        if(catalog_filter.rarity)for(const auto& v:localized::rarities)if(v.id==catalog_filter.rarity)tags.push_back({2,wide(english?v.en:v.tr)});
        if(catalog_filter.collection)for(const auto& v:localized::collections)if(v.id==catalog_filter.collection)tags.push_back({3,wide(english?v.en:v.tr)});
        if(catalog_filter.phase)for(const auto& v:localized::phases)if(v.id==catalog_filter.phase)tags.push_back({4,wide(english?v.en:v.tr)});
        if(catalog_filter.stattrak)tags.push_back({5,L"StatTrak"});
        if(!value(filter).empty())tags.push_back({6,value(filter)});
        filter_chips.clear();int chip_x=184;
        for(std::size_t n=0;n<tags.size();++n){const int width=std::min(152,std::max(84,static_cast<int>(tags[n].second.size())*7+30));
            if(chip_x+width>750){label(dc,L"+"+std::to_wstring(tags.size()-n),std::min(chip_x,708),155,40,22,small,muted);break;}
            plate(dc,chip_x,151,width,26,palette.card,palette.card,edge,7);label(dc,tags[n].second,chip_x+8,155,width-29,20,small,muted);
            label(dc,L"×",chip_x+width-20,154,16,20,small,muted);filter_chips.push_back({{chip_x,151,width,26},tags[n].first});chip_x+=width+6;
        }
        if(tags.empty())label(dc,english?L"All finishes":L"Tüm kaplamalar",188,155,480,22,small,muted);
        plate(dc,184,104,172,38,field,palette.button,edge);
        plate(dc,562,104,162,38,field,field,edge);
        studio::icon(dc,studio::Search,px(572),px(114),px(18),muted);
        if(agents_view())label(dc,english?L"Choose an agent from your team's collection.":L"Takımının koleksiyonundan bir ajan seç.",20,442,132,94,small,muted);
        else if(both())label(dc,english?L"Applies to both teams":L"İki takıma uygulanır",20,442,132,54,small,muted);
        gallery_page=menu::clamp_page(gallery_page,visible_finishes.size());
        const auto selected=SendMessageW(preset,CB_GETCURSEL,0,0);
        for(std::size_t card=0;card<menu::page_size;++card) {
            const auto index=menu::card_index(gallery_page,card,visible_finishes.size());if(index>=visible_finishes.size())break;
            const auto& finish=*visible_finishes[index];const auto area=menu::card_rect(card);const int x=area.x,y=area.y;
            RECT bounds{px(x),px(y),px(x+area.w),px(y+area.h)};
            if(!RectVisible(dc,&bounds))continue;
            const bool picked=static_cast<LRESULT>(index)==selected;const auto hover=static_cast<unsigned>(hover_levels[card]*10);
            const auto base=palette.card;
            const auto top=picked?ui::mix(base,purple,light_theme?12:12):ui::mix(base,purple,hover);
            const auto bottom=picked?ui::mix(base,purple,light_theme?20:23):palette.panel;
            plate(dc,x,y,area.w,area.h,top,bottom,picked?purple:edge,10,1);
            if(agents_view()) {
                preview(dc,finish.definition,finish.paint,x+8,y+4,106,118);
                label(dc,wide(localized::weapon(finish.definition,english)),x+120,y+36,174,72,medium);
                label(dc,SendMessageW(team,CB_GETCURSEL,0,0)==1?L"CT":L"T",x+120,y+104,168,20,small,muted);
            } else {
                preview(dc,finish.definition,finish.paint,x+10,y+4,292,92);
                const auto caption=finish.paint==0?(english?L"Default":L"Varsayılan"):wide(localized::finish(finish.definition,finish.paint,english));
                label(dc,caption,x+14,y+100,280,30,body);
            }
            const auto team_name=SendMessageW(team,CB_GETCURSEL,0,0)==1?"CT":"T";
            const auto has=[&](const char* side){return std::any_of(submitted.begin(),submitted.end(),[&](const auto& c){return c.team==side && c.definition==finish.definition && c.paint==finish.paint;});};
            const bool sent=both()?(has("T")&&has("CT")):has(team_name);
            if(sent) {
                Gdiplus::Graphics g(dc);g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);Gdiplus::SolidBrush b(studio::color(RGB(55,205,151)));
                g.FillEllipse(&b,px(x+area.w-31),px(y+10),px(22),px(22));
                studio::icon(dc,studio::Check,px(x+area.w-29),px(y+12),px(18),RGB(6,24,16));
            } else if(favorites.contains(finish.definition,finish.paint))studio::icon(dc,studio::Star,px(x+area.w-30),px(y+10),px(20),purple);
        }
        if(visible_finishes.empty())label(dc,english?L"No matches. Try another search or turn off Favorites.":L"Eşleşme yok. Aramayı değiştir veya Favoriler filtresini kapat.",240,320,520,80,medium,muted);
        const auto page=std::to_wstring(gallery_page+1)+L" / "+std::to_wstring(menu::pages(visible_finishes.size()))+L"   ·   "+std::to_wstring(visible_finishes.size())+(agents_view()?(english?L" agents":L" ajan"):(english?L" finishes":L" kaplama"));
        label(dc,page,398,650,330,20,small,muted);
        if(selected>=0 && static_cast<std::size_t>(selected)<visible_finishes.size()) {
            const auto f=visible_finishes[selected];
            label(dc,wide(item_label(f->definition,f->paint,english).data()),864,110,276,52,medium);
            preview(dc,f->definition,f->paint,850,198,344,agents_view()?320:166);
        }
        if(!agents_view() && customize_open) {
            plate(dc,864,414,320,150,field,field,edge,8);
            plate(dc,876,432,296,34,field,field,edge,5);
            plate(dc,1024,470,148,34,field,field,edge,5);
            plate(dc,876,530,136,34,field,field,edge,5);plate(dc,1036,530,136,34,field,field,edge,5);
            label(dc,english?L"Name tag":L"İsim etiketi",880,414,288,16,small,muted);
            label(dc,L"Desen / seed",880,512,128,18,small,muted);
            label(dc,L"Aşınma / float",1040,512,128,18,small,muted);
        }
        if(agents_view())label(dc,english?L"Select a portrait, then Apply.":L"Portreyi seç, ardından Uygula.",864,540,320,44,body,muted);
        draw_toast(dc);
    }
    void draw_toast(HDC dc) {
        if(toast_text.empty() || GetTickCount64()>=toast_until)return;
        const auto accent=toast_error?RGB(239,118,128):RGB(55,205,151);
        plate(dc,370,578,460,48,palette.panel,palette.panel,accent,10);
        label(dc,toast_text,386,584,428,36,small,text);
    }
    void record_ui_error(const char* reason) noexcept {
        try {
            wchar_t exe[32768]{};GetModuleFileNameW(nullptr,exe,32768);
            const auto error_path=std::filesystem::path(exe).parent_path()/L"Masiva-Skin-ui-errors.txt";
            std::error_code ec;if(std::filesystem::file_size(error_path,ec)>262144 && !ec){auto old=error_path;old+=L".old";std::filesystem::copy_file(error_path,old,std::filesystem::copy_options::overwrite_existing,ec);if(!ec){std::ofstream truncate(error_path,std::ios::trunc);}}
            std::ofstream out(error_path,std::ios::app);
            out<<"UI callback: "<<reason<<'\n';
            feedback=L"Arayüz hatası kaydedildi; seçimini tekrar yapabilirsin.";
        } catch(...) {}
    }
    void open_auto_float() {
        const auto selected=SendMessageW(preset,CB_GETCURSEL,0,0);
        if(agents_view() || selected<0 || static_cast<std::size_t>(selected)>=visible_finishes.size())return;
        const auto& finish=*visible_finishes[selected];
        const auto popup=CreatePopupMenu();
        if(!popup)throw win_error("Auto Float menu");
        const float current=static_cast<float>(wcstod(value(wear).c_str(),nullptr));
        for(std::size_t i=0;i<auto_float::grades.size();++i) {
            if(!auto_float::choose(finish.minimum,finish.maximum,i))continue;
            MENUITEMINFOW info{};info.cbSize=sizeof(info);
            info.fMask=MIIM_ID|MIIM_FTYPE|MIIM_DATA|MIIM_STATE;
            info.fType=MFT_OWNERDRAW;info.wID=static_cast<UINT>(i+1);info.dwItemData=0x10000|i;
            info.fState=std::isfinite(current) && current>=finish.minimum && current<=finish.maximum &&
                auto_float::grade_of(current)==static_cast<int>(i)?MFS_CHECKED:MFS_UNCHECKED;
            InsertMenuItemW(popup,static_cast<UINT>(-1),TRUE,&info);
        }
        RECT bounds{};GetWindowRect(GetDlgItem(window,AutoFloat),&bounds);
        const auto chosen=TrackPopupMenu(popup,TPM_RETURNCMD|TPM_LEFTALIGN|TPM_TOPALIGN,bounds.left,bounds.bottom+px(4),0,window,nullptr);
        DestroyMenu(popup);
        if(!chosen)return;
        const auto number=auto_float::choose(finish.minimum,finish.maximum,chosen-1);
        if(!number)return;
        track_edit();
        std::ostringstream out;out.imbue(std::locale::classic());out<<std::setprecision(9)<<*number;
        SetWindowTextW(wear,wide(out.str()).c_str());
        track_edit();
        const auto& band=auto_float::grades[chosen-1];
        feedback=std::wstring(L"Auto Float · ")+(english?band.english:band.turkish);
        InvalidateRect(window,nullptr,FALSE);
    }
    void open_model_picker() {
        if(agents_view())return;
        const auto menu=CreatePopupMenu();if(!menu)return;
        for(std::size_t i=0;i<visible_weapons.size();++i) {
            MENUITEMINFOW info{};info.cbSize=sizeof(info);info.fMask=MIIM_ID|MIIM_FTYPE|MIIM_DATA;
            info.fType=MFT_OWNERDRAW;info.wID=static_cast<UINT>(i+1);info.dwItemData=i;
            InsertMenuItemW(menu,static_cast<UINT>(i),TRUE,&info);
        }
        RECT r{};GetWindowRect(GetDlgItem(window,ModelPicker),&r);
        const auto choice=TrackPopupMenu(menu,TPM_RETURNCMD|TPM_LEFTALIGN|TPM_TOPALIGN,r.left,r.bottom+px(4),0,window,nullptr);
        DestroyMenu(menu);
        if(choice>0 && static_cast<std::size_t>(choice)<=visible_weapons.size()) {SendMessageW(weapon,CB_SETCURSEL,choice-1,0);command(Weapon,CBN_SELCHANGE);}
    }
    void draw_button(const DRAWITEMSTRUCT& item) {
        if(item.CtlType==ODT_MENU && (item.itemData&0x10000)) {
            const auto grade=static_cast<std::size_t>(item.itemData&0xffff);
            if(grade>=auto_float::grades.size())return;
            const auto brush=CreateSolidBrush((item.itemState&ODS_SELECTED)?palette.selected:palette.card);
            FillRect(item.hDC,&item.rcItem,brush);DeleteObject(brush);
            const auto state=SaveDC(item.hDC);
            SelectObject(item.hDC,body);SetBkMode(item.hDC,TRANSPARENT);SetTextColor(item.hDC,text);
            auto bounds=item.rcItem;bounds.left+=px(34);bounds.right-=px(12);
            const auto& band=auto_float::grades[grade];
            DrawTextW(item.hDC,english?band.english:band.turkish,-1,&bounds,DT_LEFT|DT_SINGLELINE|DT_VCENTER);
            if(item.itemState&ODS_CHECKED)studio::icon(item.hDC,studio::Check,item.rcItem.left+px(8),item.rcItem.top+px(9),px(18),purple);
            RestoreDC(item.hDC,state);return;
        }
        if(item.CtlType==ODT_MENU) {
            const auto i=static_cast<std::size_t>(item.itemData);if(i>=visible_weapons.size())return;
            const bool hot=(item.itemState&ODS_SELECTED)!=0;
            const auto b=CreateSolidBrush(hot?palette.selected:palette.card);FillRect(item.hDC,&item.rcItem,b);DeleteObject(b);
            auto r=item.rcItem;r.left+=px(16);r.right-=px(12);SetBkMode(item.hDC,TRANSPARENT);SetTextColor(item.hDC,text);
            const auto old=SelectObject(item.hDC,body);const auto name=wide(localized::weapon(visible_weapons[i]->definition,english));
            DrawTextW(item.hDC,name.c_str(),-1,&r,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_NOPREFIX);SelectObject(item.hDC,old);return;
        }
        const bool disabled=(item.itemState&ODS_DISABLED)!=0,pressed=(item.itemState&ODS_SELECTED)!=0;
        const bool nav=item.CtlID>=TabWeapons && item.CtlID<=TabAgents;
        const bool active_nav=nav && SendMessageW(group,CB_GETCURSEL,0,0)==item.CtlID-TabWeapons;
        const bool team_button=item.CtlID==TeamT || item.CtlID==TeamCT || item.CtlID==TeamBoth;
        const bool active_team=(item.CtlID==TeamT && SendMessageW(team,CB_GETCURSEL,0,0)==0) || (item.CtlID==TeamCT && SendMessageW(team,CB_GETCURSEL,0,0)==1) || (item.CtlID==TeamBoth && both());
        const bool primary=item.CtlID==Apply || item.CtlID==AccentColor || active_team;
        const bool quiet=(nav&&!active_nav)||item.CtlID==FavoritesOnly||item.CtlID==FavoriteToggle||item.CtlID==HideMenu||item.CtlID==ExitMenu|| (team_button&&!active_team);
        auto top=primary?ui::mix(purple,RGB(255,255,255),7):active_nav?ui::mix(bg,purple,42):palette.button;
        auto bottom=primary?ui::mix(purple,RGB(0,0,0),12):active_nav?ui::mix(bg,purple,24):palette.panel;
        auto edge=primary?purple:light_theme?RGB(194,201,215):RGB(65,66,76);
        if(quiet){top=bottom=bg;edge=bg;}
        if(item.hwndItem==hovered_button&&!disabled){top=ui::mix(top,purple,18);edge=purple;}
        if(pressed&&!disabled){top=ui::mix(top,purple,28);bottom=top;}
        if(disabled){top=bottom=palette.disabled;edge=palette.card;}
        FillRect(item.hDC,&item.rcItem,background);
        studio::rounded(item.hDC,item.rcItem,top,bottom,edge,static_cast<float>(px(7)),quiet?0.f:1.f);
        const auto ink=disabled?muted:primary?ui::ink(purple):text;
        auto old=SelectObject(item.hDC,body);SetBkMode(item.hDC,TRANSPARENT);SetTextColor(item.hDC,ink);
        auto r=item.rcItem;auto name=tr(value(item.hwndItem));
        studio::Icon symbol=studio::Check;bool has_icon=true,icon_only=false,left=false;
        switch(item.CtlID) {
        case TabWeapons:symbol=studio::Gun;left=true;break;
        case TabKnives:symbol=studio::Knife;left=true;break;
        case TabGloves:symbol=studio::Glove;left=true;break;
        case TabAgents:symbol=studio::Agent;left=true;break;
        case FavoritesOnly:symbol=studio::Star;left=true;name=english?L"Favorites":L"Favoriler";if(favorites_only)name+=L" ✓";break;
        case FavoriteToggle:symbol=studio::Star;icon_only=true;break;
        case DiscordPresence:has_icon=false;name=rpc::is_enabled()?(english?L"Discord presence: On":L"Discord görünürlüğü: Açık"):(english?L"Discord presence: Off":L"Discord görünürlüğü: Kapalı");break;
        case Home:symbol=studio::Undo;name=english?L"Back":L"Geri";break;
        case Appearance:has_icon=false;name=L"Menu & Theme Colour";break;
        case GeneralSettings:has_icon=false;name=english?L"General":L"Genel";break;
        case MainColor:has_icon=false;name=english?L"Choose main colour":L"Ana rengi seç";break;
        case ResetMainColor:has_icon=false;name=english?L"Use theme colour":L"Tema rengini kullan";break;
        case AccentColor:has_icon=false;name=english?L"Accent colour":L"Vurgu rengi";break;
        case Settings:symbol=settings_open?studio::Undo:studio::Settings;icon_only=true;break;
        case HideMenu:symbol=studio::Minus;icon_only=true;break;
        case ExitMenu:symbol=studio::Close;icon_only=true;break;
        case Recent:has_icon=false;left=true;name=english?L"Recent skins":L"Son kullanılanlar";break;
        case ClearFilters:has_icon=false;name=english?L"Clear":L"Temizle";break;
        case PalettePreset:has_icon=false;name=english?L"Theme presets":L"Hazır temalar";break;
        
        case Load:symbol=studio::Layers;name=L"Loadout";break;
        case Save:symbol=studio::Agent;name=english?L"Profiles":L"Profiller";break;
        case Apply:symbol=studio::Check;break;
        case UndoEdit:symbol=studio::Undo;name=english?L"Undo":L"Geri al";break;
        case Restore:symbol=studio::Undo;name=english?L"Restore all":L"Tümünü geri al";break;
        case Customize:symbol=studio::Sliders;left=true;name=english?L"Customize":L"Özelleştir";break;
        case DecorationsButton:symbol=studio::Tag;left=true;name=english?L"Stickers & Charms":L"Çıkartma ve Tılsım";break;
        case AdvancedFilters:symbol=studio::Filter;name=english?L"Filters":L"Filtre";break;
        case AutoFloat:has_icon=false;left=true;name=L"Auto Float";break;
        case ModelPicker:{has_icon=false;left=true;const auto d=selected_definition();name=agents_view()?(SendMessageW(team,CB_GETCURSEL,0,0)==1?L"CT Agents":L"T Agents"):(d?wide(localized::weapon(d,english)):L"—");if(agents_view()&&!english)name=SendMessageW(team,CB_GETCURSEL,0,0)==1?L"CT Ajanları":L"T Ajanları";break;}
        default:has_icon=false;break;
        }
        int icon_x=r.left+px(12);
        if(icon_only)icon_x=r.left+(r.right-r.left-px(22))/2;
        if(item.CtlID==Apply)icon_x=r.left+px(42);
        if(has_icon) {
            auto tint=ink;
            if(item.CtlID==FavoriteToggle){const auto i=SendMessageW(preset,CB_GETCURSEL,0,0);if(i>=0&&static_cast<std::size_t>(i)<visible_finishes.size()&&favorites.contains(visible_finishes[i]->definition,visible_finishes[i]->paint))tint=purple;}
            studio::icon(item.hDC,symbol,icon_x,r.top+(r.bottom-r.top-px(20))/2,px(20),tint);
            r.left=icon_x+px(28);
        } else if(left)r.left+=px(14);
        if(item.CtlID==Customize || item.CtlID==ModelPicker || item.CtlID==AutoFloat) {
            if(item.CtlID!=ModelPicker || !agents_view())studio::icon(item.hDC,item.CtlID==Customize&&customize_open?studio::Minus:studio::Chevron,item.rcItem.right-px(30),item.rcItem.top+(item.rcItem.bottom-item.rcItem.top-px(18))/2,px(18),ink);
            r.right-=px(36);
        } else r.right-=px(8);
        if(item.CtlID==AdvancedFilters && catalog_filter.active())SetTextColor(item.hDC,purple);
        if(!icon_only)DrawTextW(item.hDC,name.c_str(),-1,&r,(left||has_icon?DT_LEFT:DT_CENTER)|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_NOPREFIX);
        if((item.itemState&ODS_FOCUS) && !(item.itemState&ODS_NOFOCUSRECT)) {
            auto focus=item.rcItem;InflateRect(&focus,-px(3),-px(3));
            studio::outline(item.hDC,focus,purple,static_cast<float>(px(5)));
        }
        SelectObject(item.hDC,old);
    }

};
#include "translations.inl"
LRESULT CALLBACK procedure(HWND window, UINT message, WPARAM w, LPARAM l) {
    auto app = reinterpret_cast<App*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        app = static_cast<App*>(reinterpret_cast<CREATESTRUCTW*>(l)->lpCreateParams);
        app->window = window; SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
    }
    if (!app) return DefWindowProcW(window, message, w, l);
    switch (message) {
        case WM_APP+90: DestroyWindow(window);return 0;
        case WM_DPICHANGED: if(app->page_open){const auto route=app->page_route;page::close(window);PostMessageW(window,WM_COMMAND,MAKEWPARAM(route,BN_CLICKED),0);}app->system_dpi=HIWORD(w);if(app->body)try{app->reflow();}catch(...){app->record_ui_error("DPI reflow");}return 0;
        case WM_DISPLAYCHANGE: if(app->body)try{app->reflow();}catch(...){app->record_ui_error("display reflow");}return 0;
        case WM_CREATE:
            try { app->create(); SendMessageW(window,WM_CHANGEUISTATE,MAKEWPARAM(UIS_SET,UISF_HIDEFOCUS),0); } catch (const std::exception& e) { MessageBoxW(window, wide(e.what()).c_str(), L"Masiva Skin", MB_ICONERROR); return -1; }
            return 0;
        case menu_toggle_message: app->menu_host.toggle();app->refresh_key_label();return 0;
        case menu_key_message: app->bind_key(static_cast<unsigned>(w));return 0;
        case menu_tray_message:
            if(l==WM_LBUTTONUP || l==WM_RBUTTONUP)app->menu_host.show();
            return 0;
        case WM_LBUTTONDOWN:
            if(!app->gallery_click(GET_X_LPARAM(l),GET_Y_LPARAM(l)) && GET_Y_LPARAM(l)<app->px(68) && GET_X_LPARAM(l)<app->px(530)) {
                ReleaseCapture();SendMessageW(window,WM_NCLBUTTONDOWN,HTCAPTION,0);
            }
            return 0;
        case WM_MOUSEWHEEL: {
            POINT point{GET_X_LPARAM(l),GET_Y_LPARAM(l)};ScreenToClient(window,&point);
            if(!app->settings_open && !app->page_open && !app->customize_open && point.x>=app->px(168) && point.x<app->px(844) && point.y>=app->px(150) && point.y<app->px(684))app->turn_page(GET_WHEEL_DELTA_WPARAM(w)>0?-1:1);
            return 0;
        }
        case WM_ACTIVATE:
            if(!app->preview_only && LOWORD(w)==WA_INACTIVE && IsWindowEnabled(window) && l && GetAncestor(reinterpret_cast<HWND>(l),GA_ROOTOWNER)!=window) {
                app->menu_host.hide(false);app->refresh_key_label();
            }
            return 0;
        case WM_COMMAND: app->command(LOWORD(w), HIWORD(w)); return 0;
        case WM_SHOWWINDOW:
            if(w) {
                PostMessageW(window,WM_TIMER,1,0);
                app->animation_started=GetTickCount64();app->fading=true;
                SetLayeredWindowAttributes(window,0,static_cast<BYTE>(ui::alpha(app->opacity_percent)*3/4),LWA_ALPHA);
                SetTimer(window,2,16,nullptr);
            } else {KillTimer(window,2);app->fading=false;}
            return DefWindowProcW(window,message,w,l);
        case WM_TIMER: try { if(w==2)app->animate();else if(w==1)app->poll();else if(w==3){KillTimer(window,3);app->track_edit();} } catch(const std::exception& e) { app->record_ui_error(e.what()); } catch(...) { app->record_ui_error("timer"); } return 0;
        case WM_MEASUREITEM: {auto m=reinterpret_cast<MEASUREITEMSTRUCT*>(l);if(m->CtlType==ODT_MENU){m->itemWidth=app->px(300);m->itemHeight=app->px(36);return TRUE;}break;}
        case WM_DRAWITEM: try { app->draw_button(*reinterpret_cast<DRAWITEMSTRUCT*>(l)); } catch(...) { app->record_ui_error("draw button"); } return TRUE;
        case WM_CTLCOLOREDIT:
            SetTextColor(reinterpret_cast<HDC>(w),text);SetBkColor(reinterpret_cast<HDC>(w),app->palette.card);return reinterpret_cast<LRESULT>(app->input_surface);
        case WM_CTLCOLORBTN: case WM_CTLCOLORSTATIC: case WM_CTLCOLORLISTBOX:
            SetTextColor(reinterpret_cast<HDC>(w), text); SetBkColor(reinterpret_cast<HDC>(w), panel);
            return reinterpret_cast<LRESULT>(app->surface);
        case WM_PAINT: {
            PAINTSTRUCT paint{};
            const auto dc=BeginPaint(window,&paint);
            const auto& region=paint.rcPaint;
            const int width=region.right-region.left,height=region.bottom-region.top;
            if(width>0 && height>0) {
                RECT title_region{app->px(100),app->px(54),app->px(500),app->px(76)};
                const bool header_only=EqualRect(&region,&title_region)!=0;
                RECT client_rect{};GetClientRect(window,&client_rect);
                const auto buffer=CreateCompatibleDC(dc);
                const auto bitmap=buffer?CreateCompatibleBitmap(dc,header_only?width:client_rect.right,header_only?height:client_rect.bottom):nullptr;
                const auto previous=bitmap?SelectObject(buffer,bitmap):nullptr;
                const auto target=previous?buffer:dc;
                if(previous) {
                    if(header_only)SetViewportOrgEx(target,-region.left,-region.top,nullptr);
                    IntersectClipRect(target,region.left,region.top,region.right,region.bottom);
                }
                try {
                    if(header_only) {
                        FillRect(target,&region,app->background);
                        app->label(target,wide(release::subtitle()),100,54,400,22,app->release_font,app->animated_title());
                    } else app->draw(target);
                } catch(const std::exception& e){app->record_ui_error(e.what());}
                catch(...){app->record_ui_error("paint");}
                if(previous) {
                    BitBlt(dc,region.left,region.top,width,height,buffer,region.left,region.top,SRCCOPY);
                    SelectObject(buffer,previous);
                }
                if(bitmap)DeleteObject(bitmap);
                if(buffer)DeleteDC(buffer);
            }
            EndPaint(window,&paint);
            return 0;
        }
        case WM_ERASEBKGND: return 1;
        case WM_CLOSE: if(app->preview_only){DestroyWindow(window);return 0;}app->menu_host.hide();app->refresh_key_label();return 0;
        case WM_DESTROY: KillTimer(window, 1); KillTimer(window,2); KillTimer(window,3); PostQuitMessage(0); return 0;
        default: return DefWindowProcW(window, message, w, l);
    }
    return DefWindowProcW(window,message,w,l);
}
}
int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR command_line, int show) {
    using DpiFn=BOOL(WINAPI*)(HANDLE);
    auto dpi_fn=reinterpret_cast<DpiFn>(reinterpret_cast<void*>(GetProcAddress(GetModuleHandleW(L"user32.dll"),"SetProcessDpiAwarenessContext")));
    if(dpi_fn)dpi_fn(reinterpret_cast<HANDLE>(static_cast<INT_PTR>(-4)));else SetProcessDPIAware();
    bool preview_only=std::wstring(command_line)==L"--preview";
    try {
        if(!preview_only) {
            const int choice=LaunchScreen::wait_for_game(instance);
            if(choice==IDCANCEL || choice==0)return 0;
            preview_only=choice==IDC_LAUNCH_PREVIEW;
        }
    } catch(const std::exception& error) {
        MessageBoxA(nullptr,error.what(),"Masiva Skin",MB_OK|MB_ICONERROR);
        return 1;
    }
    struct PresenceLifetime {~PresenceLifetime(){rpc::stop();}} presence_lifetime;
    App app;
    app.preview_only=preview_only;
    WNDCLASSW cls{}; cls.hInstance = instance; cls.lpfnWndProc = procedure;
    cls.lpszClassName = L"MasivaSkinWindow"; cls.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    cls.hIcon = LoadIconW(instance,MAKEINTRESOURCEW(101));
    if (!RegisterClassW(&cls)) return 1;
    HWND window = CreateWindowExW(WS_EX_APPWINDOW | WS_EX_LAYERED, cls.lpszClassName,release::window_title,
        WS_POPUP | WS_MINIMIZEBOX | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT,
        740, 478, nullptr, nullptr, instance, &app);
    if (!window) return 1;
    ShowWindow(window, show); UpdateWindow(window);
    MSG message{};
    int result;
    while ((result = GetMessageW(&message, nullptr, 0, 0)) > 0) {
        if (!IsDialogMessageW(window, &message)) { TranslateMessage(&message); DispatchMessageW(&message); }
    }
    return result < 0 ? 1 : static_cast<int>(message.wParam);
}
