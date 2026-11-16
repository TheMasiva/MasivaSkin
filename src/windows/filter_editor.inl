struct FilterEditor {
 HWND window{},owner{},rarity{},collection{},phase{},stat{};int dpi{96};HFONT font{};bool english{},accepted{};localized::Filter draft;ui::Palette palette{};HBRUSH background{};
 ~FilterEditor(){if(background)DeleteObject(background);if(font)DeleteObject(font);}
 int px(int n)const{return MulDiv(n,dpi,96);}
 HWND add(const wchar_t* cls,const wchar_t* text,int id,int y,DWORD style,int height=28){
  auto c=CreateWindowExW(0,cls,text,WS_CHILD|WS_VISIBLE|style,px(20),px(y),px(420),px(height),window,reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),GetModuleHandleW(nullptr),nullptr);if(!c)throw win_error("Filter control");SendMessageW(c,WM_SETFONT,reinterpret_cast<WPARAM>(font),TRUE);return c;
 }
 template<std::size_t N> HWND combo(const wchar_t* caption,int id,int y,const localized::Choice (&choices)[N],int selected){
  add(L"STATIC",caption,0,y,0);auto c=add(L"COMBOBOX",L"",id,y+26,CBS_DROPDOWNLIST|WS_VSCROLL|WS_TABSTOP,240);
  SendMessageW(c,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(english?L"All":L"Tümü"));SendMessageW(c,CB_SETITEMDATA,0,0);int chosen=0;
  for(const auto& x:choices){auto name=wide(english?x.en:x.tr);auto i=SendMessageW(c,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(name.c_str()));SendMessageW(c,CB_SETITEMDATA,i,x.id);if(x.id==selected)chosen=static_cast<int>(i);}SendMessageW(c,CB_SETCURSEL,chosen,0);return c;
 }
 void create(){
  background=CreateSolidBrush(palette.panel);
  rarity=combo(english?L"Rarity":L"Nadirlik",1,20,localized::rarities,draft.rarity);
  collection=combo(english?L"Collection":L"Koleksiyon",2,94,localized::collections,draft.collection);
  phase=combo(english?L"Doppler phase / gem":L"Doppler fazı / değerli taş",3,168,localized::phases,draft.phase);
  stat=add(L"BUTTON",english?L"StatTrak available":L"StatTrak uygulanabilenler",4,246,BS_AUTOCHECKBOX|WS_TABSTOP);SendMessageW(stat,BM_SETCHECK,draft.stattrak?BST_CHECKED:BST_UNCHECKED,0);
  add(L"STATIC",english?L"Filters combine with search and favorites.":L"Filtreler arama ve favorilerle birlikte çalışır.",0,286,0);
  add(L"BUTTON",english?L"Apply filters":L"Filtreleri uygula",5,328,BS_DEFPUSHBUTTON|WS_TABSTOP,34);
  add(L"BUTTON",english?L"Clear all filters":L"Tüm filtreleri temizle",6,372,WS_TABSTOP,34);
  add(L"BUTTON",english?L"Cancel":L"Vazgeç",7,416,WS_TABSTOP,34);
 }
 int selected(HWND c)const{auto i=SendMessageW(c,CB_GETCURSEL,0,0);return i<0?0:static_cast<int>(SendMessageW(c,CB_GETITEMDATA,i,0));}
 static LRESULT CALLBACK proc(HWND w,UINT m,WPARAM a,LPARAM b){
  auto self=reinterpret_cast<FilterEditor*>(GetWindowLongPtrW(w,GWLP_USERDATA));if(m==WM_NCCREATE){self=static_cast<FilterEditor*>(reinterpret_cast<CREATESTRUCTW*>(b)->lpCreateParams);self->window=w;SetWindowLongPtrW(w,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(self));}if(!self)return DefWindowProcW(w,m,a,b);
  try{if(m==WM_CTLCOLORSTATIC||m==WM_CTLCOLOREDIT||m==WM_CTLCOLORLISTBOX||m==WM_CTLCOLORBTN){SetTextColor(reinterpret_cast<HDC>(a),self->palette.text);SetBkColor(reinterpret_cast<HDC>(a),self->palette.panel);return reinterpret_cast<LRESULT>(self->background);}
 if(m==WM_ERASEBKGND){RECT r{};GetClientRect(w,&r);FillRect(reinterpret_cast<HDC>(a),&r,self->background);return 1;}
 if(m==WM_CREATE){self->create();return 0;}if(m==WM_COMMAND&&HIWORD(a)==BN_CLICKED){auto id=LOWORD(a);if(id==5){self->draft={self->selected(self->rarity),self->selected(self->collection),self->selected(self->phase),SendMessageW(self->stat,BM_GETCHECK,0,0)==BST_CHECKED};self->accepted=true;DestroyWindow(w);}if(id==6){self->draft={};self->accepted=true;DestroyWindow(w);}if(id==7)DestroyWindow(w);return 0;}if(m==WM_CLOSE){DestroyWindow(w);return 0;}}catch(const std::exception& e){MessageBoxW(w,wide(e.what()).c_str(),L"Masiva Skin",MB_ICONERROR);if(m==WM_CREATE)return -1;}return DefWindowProcW(w,m,a,b);
 }
 static bool edit(HWND owner,localized::Filter& result,bool english,int dpi,HFONT font,ui::Palette palette){
  FilterEditor e;e.owner=owner;e.draft=result;e.english=english;e.palette=palette;e.dpi=page::fit(owner,dpi,460,470);LOGFONTW lf{};GetObjectW(font,sizeof(lf),&lf);lf.lfHeight=MulDiv(lf.lfHeight,e.dpi,dpi);e.font=CreateFontIndirectW(&lf);
  WNDCLASSW c{};c.hInstance=GetModuleHandleW(nullptr);c.lpfnWndProc=proc;c.lpszClassName=L"MasivaFilters092";c.hCursor=LoadCursorW(nullptr,IDC_ARROW);if(!RegisterClassW(&c)&&GetLastError()!=ERROR_CLASS_ALREADY_EXISTS)throw win_error("Filter class");
  auto w=page::create(owner,c.lpszClassName,e.dpi,460,470,&e);
        if(!w)throw win_error("Editor page");
        page::loop(owner,w);
        if(e.accepted)result=e.draft;
        return e.accepted;
 }
};
