// Local collection tools; these windows never access game memory.
struct CollectionDialog {
    HWND window{},owner{},list{},name{},detail{},team_filter{};
    HFONT font{};bool owns_font=false;int dpi{96};bool english{},loadout{},valid_draft=true,saved_current=false;ui::Palette palette{};HBRUSH background{};
    std::filesystem::path directory,chosen,active,saved_path;
    bool reapply_current=false,recent_mode=false;std::optional<Cosmetic> chosen_item;
    HWND profile_picker{},category_picker{},status{};
    std::vector<Cosmetic> preview_items;
    std::wstring preview_error;
    std::size_t page=0;
    int width()const{return loadout?1100:720;}
    int height()const{return loadout?674:650;}
    void text(HDC dc,const std::wstring& t,int x,int y,int w,int h,COLORREF color){RECT r{px(x),px(y),px(x+w),px(y+h)};SetTextColor(dc,color);SetBkMode(dc,TRANSPARENT);SelectObject(dc,font);DrawTextW(dc,t.c_str(),-1,&r,DT_LEFT|DT_WORDBREAK|DT_END_ELLIPSIS|DT_NOPREFIX);}
    std::vector<Cosmetic> side(const char* team)const {
        std::vector<Cosmetic> result;auto cat=SendMessageW(category_picker,CB_GETCURSEL,0,0);
        const char* categories[]={"","weapon","knife","glove","agent"};
        for(std::size_t i=0;i<preview_items.size();++i){const auto& c=preview_items[i];if((recent_mode?(i%2==(std::string(team)=="CT"?1U:0U)):c.team==team) && (cat<=0||cat>4||c.category==categories[cat]))result.push_back(c);}
        if(!recent_mode)std::stable_sort(result.begin(),result.end(),[](const auto& a,const auto& b){return std::tie(a.category,a.definition)<std::tie(b.category,b.definition);});
        return result;
    }
    struct Paint {
        HWND window;PAINTSTRUCT state{};HDC target{},dc{};HBITMAP bitmap{};HGDIOBJ old{};RECT rect{};
        explicit Paint(HWND w):window(w){target=BeginPaint(w,&state);GetClientRect(w,&rect);dc=CreateCompatibleDC(target);if(dc)bitmap=CreateCompatibleBitmap(target,rect.right,rect.bottom);if(dc&&bitmap)old=SelectObject(dc,bitmap);else{if(dc)DeleteDC(dc);dc=target;}}
        ~Paint(){if(dc!=target){BitBlt(target,0,0,rect.right,rect.bottom,dc,0,0,SRCCOPY);SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);}EndPaint(window,&state);}
    };
    void dashboard(HDC dc){
        text(dc,recent_mode?tr(L"SON KULLANILANLAR",L"RECENT SKINS"):tr(L"KOMBİNİM",L"MY LOADOUT"),28,20,400,32,palette.text);
        text(dc,recent_mode?tr(L"Seçmek için karta tıkla",L"Click a card to select"):tr(L"Profil seç · incele · yükle",L"Choose a profile · preview · load"),28,54,500,24,palette.muted);
        if(!recent_mode)text(dc,tr(L"Aktif profil: ",L"Active profile: ")+(active.empty()?std::wstring(tr(L"Kaydedilmemiş kombin",L"Unsaved loadout")):active.stem().wstring()),28,84,620,24,palette.muted);
        auto left=side("T"),right=side("CT");auto count=std::max(left.size(),right.size());const auto pages=std::max<std::size_t>(1,(count+1)/2);page=std::min(page,pages-1);
        for(int col=0;col<2;++col){const auto& items=col?right:left;int x=28+col*536;
            text(dc,recent_mode?L"":col?L"CT SIDE":L"T SIDE",x,112,480,24,palette.text);
            if(items.empty())text(dc,tr(L"Bu tarafta kayıtlı seçim yok.",L"No saved selections for this side."),x,174,480,50,palette.muted);
            for(std::size_t row=0;row<2;++row){auto i=page*2+row;if(i>=items.size())break;const auto& c=items[i];int y=150+static_cast<int>(row)*220;
                RECT rect{px(x),px(y),px(x+508),px(y+208)};studio::rounded(dc,rect,palette.card,palette.panel,ui::mix(palette.panel,palette.text,18),px(12));
                RECT image{px(x+14),px(y+6),px(x+494),px(y+144)};if(previews)previews->draw(dc,c.definition,c.paint,image);
                text(dc,wide(item_label(c.definition,c.paint,english).data()),x+18,y+148,472,30,palette.text);
                text(dc,wide(c.category)+L" · "+L"Float "+std::to_wstring(c.wear),x+18,y+180,290,22,palette.muted);
                if(!c.custom_name.empty())text(dc,wide(c.custom_name),x+318,y+180,172,22,palette.accent);
            }
        }
        text(dc,std::to_wstring(page+1)+L" / "+std::to_wstring(pages),470,610,160,24,palette.muted);
        text(dc,recent_mode?tr(L"Seçim taslağını açmak için karta tıkla.",L"Click a card to open it in the editor."):tr(L"Önizleme kayıtlı seçimleri gösterir; Yükle ile etkinleştir.",L"Preview shows saved selections; Load activates the profile."),28,642,1000,22,palette.muted);
    }

    std::vector<std::filesystem::path> files;
    std::vector<Cosmetic> snapshot,rows;
    PreviewCache* previews{};
    std::function<void(const std::filesystem::path&,const std::string&)> save;
    int px(int n)const{return MulDiv(n,dpi,96);}
    const wchar_t* tr(const wchar_t* a,const wchar_t* b)const{return english?b:a;}
    std::wstring value(HWND c)const{int n=GetWindowTextLengthW(c);std::wstring s(n+1,L'\0');GetWindowTextW(c,s.data(),n+1);s.resize(n);return s;}
    HWND add(const wchar_t* cls,const wchar_t* text,int id,int x,int y,int w,int h,DWORD style=0){
        auto c=CreateWindowExW(0,cls,text,WS_CHILD|WS_VISIBLE|style,px(x),px(y),px(w),px(h),window,reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),GetModuleHandleW(nullptr),nullptr);
        if(!c)throw win_error("Collection control");
        SendMessageW(c,WM_SETFONT,reinterpret_cast<WPARAM>(font),TRUE);return c;
    }
    void button(const wchar_t* text,int id,int x,int y,int w){add(L"BUTTON",text,id,x,y,w,34,BS_OWNERDRAW|WS_TABSTOP);}
    std::filesystem::path selected_file()const{auto i=SendMessageW(list,LB_GETCURSEL,0,0);if(i<0||static_cast<std::size_t>(i)>=files.size())throw std::runtime_error("Select a profile / Bir profil sec.");return files[i];}
    void populate(const std::filesystem::path& select={}){
        if(loadout){
            files=list_profiles(directory);SendMessageW(profile_picker,CB_RESETCONTENT,0,0);SendMessageW(profile_picker,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(tr(L"Mevcut kombin",L"Current loadout")));
            for(const auto& f:files)SendMessageW(profile_picker,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(f.stem().c_str()));
            SendMessageW(profile_picker,CB_SETCURSEL,0,0);preview_items=snapshot;page=0;InvalidateRect(window,nullptr,FALSE);return;
        }
        SendMessageW(list,LB_RESETCONTENT,0,0);int index=0;
        if(loadout){
            rows.clear();auto filter=SendMessageW(team_filter,CB_GETCURSEL,0,0);
            for(const auto& c:snapshot)if(filter==0 || c.team==(filter==1?"T":"CT"))rows.push_back(c);
            std::stable_sort(rows.begin(),rows.end(),[](const auto& a,const auto& b){return std::tie(a.team,a.category,a.definition)<std::tie(b.team,b.category,b.definition);});
            for(const auto& c:rows){auto text=wide(c.team+"  |  "+std::string(item_label(c.definition,c.paint,english).data()));SendMessageW(list,LB_ADDSTRING,0,reinterpret_cast<LPARAM>(text.c_str()));}
        }else{files=list_profiles(directory);for(std::size_t i=0;i<files.size();++i){auto text=files[i].stem().wstring();SendMessageW(list,LB_ADDSTRING,0,reinterpret_cast<LPARAM>(text.c_str()));if(files[i]==select)index=static_cast<int>(i);}}
        SendMessageW(list,LB_SETCURSEL,index,0);selection_changed();
    }
    std::string description(const Cosmetic& c)const{
        std::ostringstream out;out<<c.team<<" | "<<item_label(c.definition,c.paint,english).data()<<"\r\n\r\nFloat: "<<std::setprecision(8)<<c.wear<<"\r\nSeed: "<<c.seed;
        if(c.stattrak>=0)out<<"\r\nStatTrak: "<<c.stattrak;
        if(!c.custom_name.empty())out<<"\r\nNameTag: "<<c.custom_name;
        for(std::size_t i=0;i<c.decorations.stickers.size();++i)if(auto id=c.decorations.stickers[i])for(const auto& sticker:extras::stickers)if(sticker.id==id){out<<"\r\nSticker "<<i+1<<": "<<localized::decoration(false,sticker.id,english);break;}
        if(c.decorations.charm)for(const auto& charm:extras::charms)if(charm.id==c.decorations.charm){out<<"\r\nCharm: "<<localized::decoration(true,charm.id,english)<<"\r\nCharm seed: "<<c.decorations.charm_seed;break;}
        return out.str();
    }
    void selection_changed(){
        if(loadout){auto i=SendMessageW(list,LB_GETCURSEL,0,0);auto text=i>=0&&static_cast<std::size_t>(i)<rows.size()?wide(description(rows[i])):std::wstring(tr(L"Kayıtlı seçim yok. Skin seçip Uygula de.",L"No saved selections. Select a skin and Apply."));SetWindowTextW(detail,text.c_str());InvalidateRect(window,nullptr,FALSE);}
        else {auto i=SendMessageW(list,LB_GETCURSEL,0,0);preview_items.clear();if(i>=0&&static_cast<std::size_t>(i)<files.size()){SetWindowTextW(name,files[i].stem().c_str());try{preview_items=read_profile_file(files[i]);preview_error.clear();}catch(const std::exception& e){preview_error=wide(e.what());}}InvalidateRect(window,nullptr,FALSE);}
    }
    void create(){
        background=CreateSolidBrush(palette.panel);
        if(loadout){
            profile_picker=add(L"COMBOBOX",L"",21,680,24,390,260,CBS_DROPDOWNLIST|WS_VSCROLL|WS_TABSTOP);
            category_picker=add(L"COMBOBOX",L"",22,680,74,390,220,CBS_DROPDOWNLIST|WS_TABSTOP);
            for(auto name:{tr(L"Tüm eşyalar",L"All items"),tr(L"Silahlar",L"Weapons"),tr(L"Bıçaklar",L"Knives"),tr(L"Eldivenler",L"Gloves"),tr(L"Ajanlar",L"Agents")})SendMessageW(category_picker,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(name));
            SendMessageW(category_picker,CB_SETCURSEL,0,0);
            button(L"‹",23,28,602,60);button(L"›",24,100,602,60);
            button(tr(L"Uygula / Yükle",L"Apply / Load"),25,750,602,170);button(tr(L"Geri",L"Back"),10,932,602,138);
        }else{
            add(L"STATIC",tr(L"Profiller · isimli kombinlerini yönet",L"Profiles · manage your named loadouts"),0,20,16,660,24);
            list=add(L"LISTBOX",L"",1,20,50,300,410,LBS_NOTIFY|LBS_NOINTEGRALHEIGHT|WS_VSCROLL|WS_BORDER|WS_TABSTOP);
            add(L"STATIC",tr(L"Profil adı",L"Profile name"),0,342,50,350,22);
            name=add(L"EDIT",L"",2,342,78,358,28,ES_AUTOHSCROLL|WS_BORDER|WS_TABSTOP);SendMessageW(name,EM_SETLIMITTEXT,80,0);
            button(tr(L"Yeni kaydet",L"Save as new"),3,342,124,172);button(tr(L"Seçileni güncelle",L"Update selected"),4,528,124,172);
            button(tr(L"Yükle",L"Load"),5,342,174,358);
            button(tr(L"Çoğalt",L"Duplicate"),6,342,224,172);button(tr(L"Yeniden adlandır",L"Rename"),7,528,224,172);
            button(tr(L"İçe aktar",L"Import"),8,342,274,172);button(tr(L"Dışa aktar",L"Export"),9,528,274,172);
            add(L"STATIC",tr(L"PROFİL ÖNİZLEMESİ",L"PROFILE PREVIEW"),0,342,332,358,24);
            button(tr(L"Yedeği geri yükle",L"Restore backup"),26,342,586,184);
            button(tr(L"Geri",L"Back"),10,550,586,150);
            status=add(L"STATIC",L"",0,20,626,680,20);

        }
        populate();if(recent_mode){ShowWindow(profile_picker,SW_HIDE);ShowWindow(GetDlgItem(window,25),SW_HIDE);}
        if(!loadout&&!valid_draft){EnableWindow(GetDlgItem(window,3),FALSE);EnableWindow(GetDlgItem(window,4),FALSE);}
    }
    std::filesystem::path pick(bool exporting){
        wchar_t path[32768]{};OPENFILENAMEW d{};d.lStructSize=sizeof(d);d.hwndOwner=window;d.lpstrFile=path;d.nMaxFile=32768;d.lpstrFilter=L"Masiva profile\0*.masiva\0\0";d.lpstrDefExt=L"masiva";d.Flags=OFN_NOCHANGEDIR|OFN_PATHMUSTEXIST|(exporting?OFN_OVERWRITEPROMPT:OFN_FILEMUSTEXIST);
        if(!(exporting?GetSaveFileNameW(&d):GetOpenFileNameW(&d))){if(CommDlgExtendedError())throw std::runtime_error("File dialog failed");return {}; }return path;
    }
    void save_items(const std::filesystem::path& path,const std::vector<Cosmetic>& items){Selections check;check.set_all(items);std::ostringstream out;write(out,items);save(path,out.str());SetWindowTextW(status,tr(L"Profil kaydedildi. Önceki sürüm varsa yedeklendi.",L"Profile saved. Previous version backed up when present."));SetTimer(window,10,4000,nullptr);}
    void command(int id,int code){
        if(loadout){
            if(id==21&&code==CBN_SELCHANGE){auto i=SendMessageW(profile_picker,CB_GETCURSEL,0,0);preview_items.clear();InvalidateRect(window,nullptr,FALSE);preview_items=i>0&&static_cast<std::size_t>(i)<=files.size()?read_profile_file(files[i-1]):snapshot;page=0;InvalidateRect(window,nullptr,FALSE);return;}
            if(id==22&&code==CBN_SELCHANGE){page=0;InvalidateRect(window,nullptr,FALSE);return;}
            if(code==BN_CLICKED){if(id==10){DestroyWindow(window);return;}if(id==23&&page) --page;if(id==24)++page;
                if(id==25){auto i=SendMessageW(profile_picker,CB_GETCURSEL,0,0);if(i<=0){reapply_current=true;DestroyWindow(window);return;}chosen=files.at(i-1);DestroyWindow(window);return;}
                InvalidateRect(window,nullptr,FALSE);}return;
        }
        if(id==26&&code==BN_CLICKED){auto target=selected_file();auto backup=target;backup+=L".bak1";auto items=read_profile_file(backup);save_items(target,items);populate(target);return;}
        if(id==1&&code==LBN_SELCHANGE){selection_changed();return;}
        if(id==20&&code==CBN_SELCHANGE){populate();return;}
        if(code!=BN_CLICKED)return;
        if(id==10){DestroyWindow(window);return;}if(loadout)return;
        if(id==5){auto path=selected_file();(void)read_profile_file(path);chosen=path;DestroyWindow(window);return;}
        if(id==4){auto path=selected_file();if(MessageBoxW(window,tr(L"Seçili profil mevcut kombinle güncellensin mi?",L"Replace the selected profile with the current loadout?"),L"Masiva Skin",MB_YESNO|MB_ICONQUESTION)==IDYES){save_items(path,snapshot);saved_current=true;saved_path=path;populate(path);}return;}
        if(id==8){auto path=pick(false);if(path.empty())return;auto items=read_profile_file(path);auto target=named_profile(directory,path.stem().u8string());if(std::filesystem::exists(target))throw std::runtime_error("Profile already exists; rename the imported file first.");save_items(target,items);populate(target);return;}
        if(id==9){auto items=read_profile_file(selected_file());auto target=pick(true);if(!target.empty())save_items(target,items);return;}
        if(id==3||id==6||id==7){auto target=named_profile(directory,utf8(value(name)));if(std::filesystem::exists(target))throw std::runtime_error("Profile name already exists / Bu isim zaten var.");
            if(id==3){save_items(target,snapshot);saved_current=true;saved_path=target;}
            if(id==6)save_items(target,read_profile_file(selected_file()));
            if(id==7)std::filesystem::rename(selected_file(),target);
            populate(target);
        }
    }
    static LRESULT CALLBACK proc(HWND w,UINT m,WPARAM a,LPARAM b){
        auto self=reinterpret_cast<CollectionDialog*>(GetWindowLongPtrW(w,GWLP_USERDATA));
        if(m==WM_NCCREATE){self=static_cast<CollectionDialog*>(reinterpret_cast<CREATESTRUCTW*>(b)->lpCreateParams);self->window=w;SetWindowLongPtrW(w,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(self));}
        if(!self)return DefWindowProcW(w,m,a,b);
        try{
            if(m==WM_CREATE){self->create();return 0;}
            if(m==WM_LBUTTONDOWN&&self->recent_mode){int x=MulDiv(GET_X_LPARAM(b),96,self->dpi),y=MulDiv(GET_Y_LPARAM(b),96,self->dpi);for(int col=0;col<2;++col){auto rows=self->side(col?"CT":"T");for(std::size_t row=0;row<2;++row){auto i=self->page*2+row;int left=28+col*536,top=150+static_cast<int>(row)*220;if(i<rows.size()&&x>=left&&x<left+508&&y>=top&&y<top+208){self->chosen_item=rows[i];DestroyWindow(w);return 0;}}}return 0;}
            if(m==WM_COMMAND){self->command(LOWORD(a),HIWORD(a));return 0;}
            if(m==WM_TIMER&&a==10){SetWindowTextW(self->status,L"");KillTimer(w,10);return 0;}
            if(m==WM_CLOSE){DestroyWindow(w);return 0;}
            if(m==WM_CTLCOLORSTATIC||m==WM_CTLCOLOREDIT||m==WM_CTLCOLORLISTBOX){SetTextColor(reinterpret_cast<HDC>(a),self->palette.text);SetBkColor(reinterpret_cast<HDC>(a),self->palette.panel);return reinterpret_cast<LRESULT>(self->background);}
            if(m==WM_ERASEBKGND)return 1;
            if(m==WM_DRAWITEM){auto& d=*reinterpret_cast<DRAWITEMSTRUCT*>(b);const auto fill=(d.itemState&ODS_SELECTED)?self->palette.selected:self->palette.button;studio::rounded(d.hDC,d.rcItem,fill,self->palette.button,ui::mix(self->palette.panel,self->palette.text,20),self->px(8));auto old=SelectObject(d.hDC,self->font);SetBkMode(d.hDC,TRANSPARENT);SetTextColor(d.hDC,(d.itemState&ODS_DISABLED)?self->palette.muted:ui::ink(self->palette.button));auto text=self->value(d.hwndItem);DrawTextW(d.hDC,text.c_str(),-1,&d.rcItem,DT_CENTER|DT_VCENTER|DT_SINGLELINE);if(d.itemState&ODS_FOCUS)DrawFocusRect(d.hDC,&d.rcItem);SelectObject(d.hDC,old);return TRUE;}
            if(m==WM_PAINT){Paint paint(w);auto dc=paint.dc;FillRect(dc,&paint.rect,self->background);if(self->loadout)self->dashboard(dc);else if(self->previews){
                self->text(dc,self->preview_error.empty()?std::to_wstring(self->preview_items.size())+self->tr(L" kayıtlı eşya",L" saved items"):self->preview_error,342,360,358,24,self->palette.muted);
                for(std::size_t i=0;i<std::min<std::size_t>(2,self->preview_items.size());++i){const auto& c=self->preview_items[i];int x=342+static_cast<int>(i)*180;RECT image{self->px(x),self->px(394),self->px(x+170),self->px(496)};self->previews->draw(dc,c.definition,c.paint,image);self->text(dc,wide(c.team+" · "+std::string(item_label(c.definition,c.paint,self->english).data())),x,502,170,64,self->palette.text);}}
            return 0;}
        }catch(const std::exception& e){MessageBoxW(w,wide(e.what()).c_str(),L"Masiva Skin",MB_ICONERROR);if(m==WM_CREATE)return -1;}
        return DefWindowProcW(w,m,a,b);
    }
    std::filesystem::path run(){
        const auto original_dpi=dpi;dpi=page::fit(owner,dpi,width(),height());
        LOGFONTW description{};if(!GetObjectW(font,sizeof(description),&description))throw win_error("Collection font");
        description.lfHeight=MulDiv(description.lfHeight,dpi,original_dpi);font=CreateFontIndirectW(&description);if(!font)throw win_error("Collection font allocation");owns_font=true;
        WNDCLASSW cls{};cls.hInstance=GetModuleHandleW(nullptr);cls.lpfnWndProc=proc;cls.lpszClassName=L"MasivaCollection";cls.hCursor=LoadCursorW(nullptr,IDC_ARROW);
        if(!RegisterClassW(&cls)&&GetLastError()!=ERROR_CLASS_ALREADY_EXISTS)throw win_error("Collection registration");
        auto w=page::create(owner,cls.lpszClassName,dpi,width(),height(),this);
        if(!w)throw win_error("Collection page");
        page::loop(owner,w);return chosen;
    }
    ~CollectionDialog(){if(background)DeleteObject(background);if(owns_font)DeleteObject(font);}
};
