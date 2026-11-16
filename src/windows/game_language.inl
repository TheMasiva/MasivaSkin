// Use public Steamworks language accessors; never initialize or shut down Steam.
// Explicit -language launch options take precedence over Steam's configured language.
bool hud_english=true;
std::uint64_t language_checked_at{};
void update_hud_language(){
 const auto now=GetTickCount64();if(now<language_checked_at)return;language_checked_at=now+5000;
 static const std::wstring override_language=[](){
  int count{};auto args=CommandLineToArgvW(GetCommandLineW(),&count);std::wstring result;
  if(args){for(int i=1;i+1<count;++i)if(_wcsicmp(args[i],L"-language")==0){result=args[i+1];break;}LocalFree(args);}return result;
 }();
 std::string language="english";
 if(!override_language.empty())language=_wcsicmp(override_language.c_str(),L"turkish")==0?"turkish":"english";
 else if(auto module=GetModuleHandleW(L"steam_api64.dll")){
  using GetApps=void*(*)();using GetLanguage=const char*(*)(void*);
  GetApps apps{};auto apps_address=GetProcAddress(module,"SteamAPI_SteamApps_v008");static_assert(sizeof(apps)==sizeof(apps_address));std::memcpy(&apps,&apps_address,sizeof(apps));
  GetLanguage get_language{};auto language_address=GetProcAddress(module,"SteamAPI_ISteamApps_GetCurrentGameLanguage");std::memcpy(&get_language,&language_address,sizeof(get_language));
  if(apps&&get_language)if(auto context=apps())if(auto value=get_language(context)){language.assign(value,strnlen(value,32));}
 }
 hud_english=language!="turkish";
 if(box)std::snprintf(box->hud_language,sizeof(box->hud_language),"%s",language.c_str());
}
