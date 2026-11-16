// Offline image browser. Opening/closing this dialog never mutates the live selection.
struct DecorationEditor {
    HWND window{},owner{},slot{},query{},charm_query{},sticker{},charm{},seed{},xyz[3]{};
    HFONT font{};int dpi{96};bool english{},accepted{},updating{};Decorations draft;
    PreviewCache* previews{};ui::Palette palette{};HBRUSH background{};
    std::vector<int> visible,visible_charms;
    ~DecorationEditor(){if(font)DeleteObject(font);if(background)DeleteObject(background);}
    int px(int n)const{return MulDiv(n,dpi,96);}
    const wchar_t* tr(const wchar_t* a,const wchar_t* b)const{return english?b:a;}
    HWND add(const wchar_t* cls,const wchar_t* text,int id,int x,int y,int w,int h,DWORD style=0) {
        auto c=CreateWindowExW(0,cls,text,WS_CHILD|WS_VISIBLE|style,px(x),px(y),px(w),px(h),window,reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),GetModuleHandleW(nullptr),nullptr);
        if(!c)throw win_error("Sticker editor control");
        SendMessageW(c,WM_SETFONT,reinterpret_cast<WPARAM>(font),TRUE);return c;
    }
    std::wstring value(HWND c)const {int n=GetWindowTextLengthW(c);std::wstring s(n+1,L'\0');GetWindowTextW(c,s.data(),n+1);s.resize(n);return s;}
    int selected_slot()const{return static_cast<int>(SendMessageW(slot,CB_GETCURSEL,0,0));}
    static std::wstring lower(std::wstring s){return search_fold(std::move(s));}
    void reload(bool charms=false) {
        updating=true;auto list=charms?charm:sticker;auto& ids=charms?visible_charms:visible;
        SendMessageW(list,WM_SETREDRAW,FALSE,0);SendMessageW(list,LB_RESETCONTENT,0,0);ids.clear();
        const auto search=lower(value(charms?charm_query:query));int selected=-1;
        const auto current=charms?draft.charm:draft.stickers.at(selected_slot());
        auto add_entry=[&](const auto& e){
            auto name=wide(localized::decoration(charms,e.id,english));
            // Retain the selected row even when the search changes.
            if(e.id && e.id!=current && lower(name).find(search)==std::wstring::npos && lower(wide(e.name)).find(search)==std::wstring::npos)return;
            if(e.id==current)selected=static_cast<int>(ids.size());
            ids.push_back(e.id);SendMessageW(list,LB_ADDSTRING,0,reinterpret_cast<LPARAM>(name.c_str()));
        };
        if(charms){for(const auto& e:extras::charms)add_entry(e);}else{for(const auto& e:extras::stickers)add_entry(e);}
        SendMessageW(list,LB_SETCURSEL,selected,0);SendMessageW(list,WM_SETREDRAW,TRUE,0);InvalidateRect(list,nullptr,TRUE);updating=false;
    }
    void create() {
        background=CreateSolidBrush(palette.panel);
        add(L"STATIC",tr(L"Sticker / Charm · görsel düzenleyici",L"Sticker / Charm · visual editor"),0,20,14,680,24);
        add(L"STATIC",tr(L"Sticker yuvası",L"Sticker slot"),0,20,48,120,24);
        slot=add(L"COMBOBOX",L"",1,145,44,80,180,CBS_DROPDOWNLIST|WS_TABSTOP);
        for(int i=1;i<=5;++i){auto t=std::to_wstring(i);SendMessageW(slot,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(t.c_str()));}SendMessageW(slot,CB_SETCURSEL,0,0);
        query=add(L"EDIT",L"",2,20,80,400,28,WS_BORDER|ES_AUTOHSCROLL|WS_TABSTOP);
        charm_query=add(L"EDIT",L"",11,440,80,280,28,WS_BORDER|ES_AUTOHSCROLL|WS_TABSTOP);
        SendMessageW(query,EM_SETCUEBANNER,TRUE,reinterpret_cast<LPARAM>(tr(L"Sticker ara…",L"Search stickers…")));
        SendMessageW(charm_query,EM_SETCUEBANNER,TRUE,reinterpret_cast<LPARAM>(tr(L"Charm ara…",L"Search charms…")));
        for(auto c:{query,charm_query})SendMessageW(c,EM_SETLIMITTEXT,80,0);
        const DWORD style=LBS_OWNERDRAWFIXED|LBS_HASSTRINGS|LBS_NOTIFY|LBS_NOINTEGRALHEIGHT|WS_VSCROLL|WS_BORDER|WS_TABSTOP;
        sticker=add(L"LISTBOX",L"",3,20,118,400,320,style);charm=add(L"LISTBOX",L"",4,440,118,280,320,style);
        add(L"STATIC",tr(L"Charm deseni (0–100000)",L"Charm pattern (0–100000)"),0,20,454,240,22);
        seed=add(L"EDIT",std::to_wstring(draft.charm_seed).c_str(),5,20,480,180,28,WS_BORDER|ES_AUTOHSCROLL|WS_TABSTOP);SendMessageW(seed,EM_SETLIMITTEXT,6,0);
        const float values[]={draft.x,draft.y,draft.z};
        for(int i=0;i<3;++i){const wchar_t* names[]={L"X",L"Y",L"Z"};add(L"STATIC",names[i],0,270+i*150,454,130,22);xyz[i]=add(L"EDIT",std::to_wstring(values[i]).c_str(),6+i,270+i*150,480,130,28,WS_BORDER|ES_AUTOHSCROLL|WS_TABSTOP);SendMessageW(xyz[i],EM_SETLIMITTEXT,16,0);}
        add(L"STATIC",tr(L"Konum: -100…100. Beş sticker yuvası ayrı kaydedilir.\nVazgeç, bu penceredeki değişiklikleri iptal eder.",L"Position: -100…100. Five sticker slots are saved separately.\nCancel discards changes in this window."),0,20,530,420,50);
        add(L"BUTTON",tr(L"Kaydet",L"Save"),9,460,540,120,36,BS_DEFPUSHBUTTON|WS_TABSTOP);add(L"BUTTON",tr(L"Vazgeç",L"Cancel"),10,600,540,120,36,WS_TABSTOP);
        reload();reload(true);
    }
    void draw(const DRAWITEMSTRUCT& d){
        if(d.itemID==static_cast<UINT>(-1))return;
        const bool charms=d.CtlID==4;const auto& ids=charms?visible_charms:visible;if(d.itemID>=ids.size())return;
        const bool selected=(d.itemState&ODS_SELECTED)!=0;auto brush=CreateSolidBrush(selected?palette.selected:palette.card);FillRect(d.hDC,&d.rcItem,brush);DeleteObject(brush);
        RECT image{d.rcItem.left+px(5),d.rcItem.top+px(4),d.rcItem.left+px(61),d.rcItem.bottom-px(4)};
        auto old=SelectObject(d.hDC,font);SetBkMode(d.hDC,TRANSPARENT);SetTextColor(d.hDC,palette.text);
        if(ids[d.itemID] && !previews->draw(d.hDC,charms?-2:-1,ids[d.itemID],image))DrawTextW(d.hDC,L"—",-1,&image,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        auto name=wide(localized::decoration(charms,ids[d.itemID],english));RECT r=d.rcItem;r.left+=px(68);r.right-=px(6);r.top+=px(8);
        DrawTextW(d.hDC,name.c_str(),-1,&r,DT_LEFT|DT_WORDBREAK|DT_END_ELLIPSIS|DT_NOPREFIX);
        if(d.itemState&ODS_FOCUS)DrawFocusRect(d.hDC,&d.rcItem);
        SelectObject(d.hDC,old);
    }
    float number(HWND c)const {
        auto s=utf8(value(c));std::replace(s.begin(),s.end(),',','.');std::istringstream in(s);in.imbue(std::locale::classic());float f{};
        if(!(in>>f))throw std::runtime_error("Invalid charm position");
        in>>std::ws;
        if(!in.eof() || !std::isfinite(f) || std::abs(f)>100)throw std::runtime_error("Charm position must be -100..100");
        return f;
    }
    void command(int id,int code) {
        if(updating)return;
        if(id==1 && code==CBN_SELCHANGE){reload();return;}
        if((id==2 || id==11) && code==EN_CHANGE){reload(id==11);return;}
        if((id==3 || id==4) && code==LBN_SELCHANGE){auto& ids=id==3?visible:visible_charms;auto i=SendMessageW(id==3?sticker:charm,LB_GETCURSEL,0,0);if(i>=0 && static_cast<std::size_t>(i)<ids.size()){if(id==3)draft.stickers.at(selected_slot())=ids[i];else draft.charm=ids[i];}return;}
        if(code!=BN_CLICKED)return;
        if(id==10){DestroyWindow(window);return;}
        if(id==9){draft.charm_seed=parse_integer(utf8(value(seed)),"Charm seed",100000,true);draft.x=number(xyz[0]);draft.y=number(xyz[1]);draft.z=number(xyz[2]);if(!draft.charm){draft.charm_seed=0;draft.x=draft.y=draft.z=0;}accepted=true;DestroyWindow(window);}
    }
    static LRESULT CALLBACK proc(HWND w,UINT m,WPARAM a,LPARAM b) {
        auto self=reinterpret_cast<DecorationEditor*>(GetWindowLongPtrW(w,GWLP_USERDATA));
        if(m==WM_NCCREATE){self=static_cast<DecorationEditor*>(reinterpret_cast<CREATESTRUCTW*>(b)->lpCreateParams);self->window=w;SetWindowLongPtrW(w,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(self));}
        if(!self)return DefWindowProcW(w,m,a,b);
        try {
            if(m==WM_CREATE){self->create();return 0;}
            if(m==WM_MEASUREITEM){reinterpret_cast<MEASUREITEMSTRUCT*>(b)->itemHeight=self->px(64);return TRUE;}
            if(m==WM_DRAWITEM){self->draw(*reinterpret_cast<DRAWITEMSTRUCT*>(b));return TRUE;}
            if(m==WM_CTLCOLORSTATIC || m==WM_CTLCOLORLISTBOX){SetTextColor(reinterpret_cast<HDC>(a),self->palette.text);SetBkColor(reinterpret_cast<HDC>(a),self->palette.panel);return reinterpret_cast<LRESULT>(self->background);}
            if(m==WM_ERASEBKGND){RECT r;GetClientRect(w,&r);FillRect(reinterpret_cast<HDC>(a),&r,self->background);return 1;}
            if(m==WM_COMMAND){self->command(LOWORD(a),HIWORD(a));return 0;}
            if(m==WM_CLOSE){DestroyWindow(w);return 0;}
        }catch(const std::exception& e){MessageBoxW(w,wide(e.what()).c_str(),L"Masiva Skin",MB_ICONERROR);if(m==WM_CREATE)return -1;}
        return DefWindowProcW(w,m,a,b);
    }
    static bool edit(HWND owner,Decorations& result,bool english,int dpi,HFONT font,PreviewCache& previews,ui::Palette palette) {
        DecorationEditor e;e.owner=owner;e.draft=result;e.english=english;e.dpi=page::fit(owner,dpi,740,596);e.previews=&previews;e.palette=palette;
        LOGFONTW lf{};GetObjectW(font,sizeof(lf),&lf);lf.lfHeight=MulDiv(lf.lfHeight,e.dpi,dpi);e.font=CreateFontIndirectW(&lf);
        WNDCLASSW cls{};cls.hInstance=GetModuleHandleW(nullptr);cls.lpfnWndProc=proc;cls.lpszClassName=L"MasivaDecorations092";cls.hCursor=LoadCursorW(nullptr,IDC_ARROW);
        if(!RegisterClassW(&cls) && GetLastError()!=ERROR_CLASS_ALREADY_EXISTS)throw win_error("Sticker window registration");
        auto w=page::create(owner,cls.lpszClassName,e.dpi,740,596,&e);
        if(!w)throw win_error("Editor page");
        page::loop(owner,w);
        if(e.accepted)result=e.draft;
        return e.accepted;
    }
};
