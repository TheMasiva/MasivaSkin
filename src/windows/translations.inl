std::wstring App::tr(std::wstring value) const {
    if(!english)return value;
    // UI text is kept separate from skin names and user-entered name tags.
    static const std::pair<const wchar_t*,const wchar_t*> messages[]={
        {L"Tema: Beyaz",L"Theme: White"},{L"Tema: Siyah",L"Theme: Black"},{L"Menü rengi",L"Menu color"},
        {L"Görsel yok",L"No preview"},{L"Koleksiyon",L"Collection"},{L"Gizle",L"Hide"},{L"Çıkış",L"Exit"},
        {L"Bir tuşa bas · Esc iptal",L"Press a key · Esc cancels"},{L"Menü tuşu: ",L"Menu key: "},
        {L"Ajan görünümü",L"Agent appearance"},{L"Ajanlar",L"Agents"},{L"Ajan: ",L"Agent: "},{L"HUD uyumluluk",L"HUD compatibility"},
        {L"Sticker / Charm seçimi hazır; Uygula de.",L"Sticker / Charm selection ready; press Apply."},
        {L"Bağlantı yok",L"Not connected"},
        {L"CS2 açıkken Bağlan düğmesini kullan.",L"Start CS2, then connect."},
        {L"Uygula bağlantıyı da kurar; CS2 açık olsun.",L"Apply also connects. Keep CS2 running."},
        {L"Seçim kaydedilebilir",L"Ready to apply"},
        {L"Bir eşya ve kaplama seç",L"Choose an item and finish"},
        {L"Elde olması gerekmez; eline alınca otomatik uygulanır.",L"No need to hold it. Applied when equipped."},
        {L" · Seçimler ayrı korunur.",L" · Each selection is remembered."},
        {L"Elde: ",L"Holding: "},{L"diğer eşya",L"other item"},
        {L"Uygulama hazır değil",L"Not ready"},
        {L"Seçim kaydedildi · uygun eşya elde olduğunda uygulanır.",L"Saved · applied when the matching item is equipped."},
        {L"Seçim geri alındı; bu eşyanın takibi kapalı.",L"Restored; automatic application disabled for this item."},
        {L"İşlem tamamlanamadı; aşağıdaki hata mesajını kontrol et.",L"Action failed. Check the diagnostic log below."},
        {L"Arayüz hatası kaydedildi; seçimini tekrar yapabilirsin.",L"UI error recorded. Please select the item again."},
        {L"Eşya → kaplama → Uygula. Elde olması gerekmez.",L"Item → finish → Apply. No need to hold the item."},
        {L"Görünümünü seç ve uygula",L"Choose your look and apply"},
        {L"Kategori",L"Category"},{L"Silah / model",L"Weapon / model"},{L"Takım",L"Team"},
        {L"Kaplama / phase",L"Finish / phase"},{L"Kaplama ara…",L"Search finishes…"},
        {L"Aramayla eşleşen kaplama yok.",L"No finishes match your search."},
        {L"Özel isim etiketi (isteğe bağlı)",L"Custom name tag (optional)"},
        {L"Desen / seed",L"Pattern / seed"},{L"Aşınma / float",L"Wear / float"},
        {L" seçenek  ·  Float ",L" finishes · Float "},
        {L"Günlüğü kapat",L"Close log"},{L"Günlük / tanı",L"Log / diagnostics"},
        {L"İşlem günlüğü",L"Activity log"},{L"Profil kaydet",L"Save profile"},{L"Profil yükle",L"Load profile"},
        {L"Tanı raporu",L"Diagnostic report"},{L"Bağlantıyı kes",L"Disconnect"},
        {L"Bağlantı kesildi.",L"Disconnected."},{L"Bağlan",L"Connect"},
        {L"Geri al",L"Restore"},{L"Uygula",L"Apply"},
        {L"Silahlar",L"Weapons"},{L"Bıçaklar",L"Knives"},{L"Eldivenler",L"Gloves"},
        {L"Profil kaydedildi. Diğer takım ve eşya kayıtları korundu.",L"Profile saved. Other team and item selections were preserved."},
        {L"Profil yüklendi; tüm kayıtlı seçimler etkinleştiriliyor.",L"Profile loaded; activating all saved selections."},
        {L"Profil etkin · tüm kayıtlı silah, bıçak ve eldiven seçimleri hazır.",L"Profile active · all saved weapon, knife and glove selections are ready."},
        {L"Profil hazır; CS2'de maça girdikten sonra Bağlan veya Uygula de.",L"Profile ready; enter a match in CS2, then press Connect or Apply."},
        {L"CS2 bağlantısı kuruldu. PID ",L"Connected to CS2. PID "},
        {L"Tanı raporu kaydedildi.",L"Diagnostic report saved."},
        {L"Görünüm geri alınamadı: ",L"Could not restore appearance: "},
        {L"Eldiven geri alınamadı: ",L"Could not restore gloves: "},
        {L"Bekleyen görünüm yedekleri var; tamamen temizlemek için CS2'yi yeniden başlat.",L"Appearance backups remain; restart CS2 to fully reset."},
        {L"İşlem tamamlanamadı: ",L"Action failed (technical detail): "},
        {L"Sticker / Charm seçimi oyuna gönderildi.",L"Sticker / Charm selection sent to the game."},
        {L"0.9.2 oyun içi köprü ve materyal isteği bekleniyor...",L"Waiting for the game to accept your selection..."},
        {L"Masiva Skin 0.9.2. Her silahın seçimi ayrı korunur; Uygula otomatik takibi açar.",L"Masiva Skin 0.9.2. Apply enables automatic appearance retention for each item."},
        {L"0.9.2 oyun içi sonuçları bu ortamda doğrulanmadı. Tanı raporu teknik ayrıntıları içerir.",L"0.9.2 changes have not been live-tested here. Diagnostic reports include technical details."},
        {L"İstek: paint=",L"Request: paint="},
        {L"Ara",L"Search"}
    };
    for(const auto& pair:messages)if(value==pair.first)return pair.second;
    for(const auto& pair:messages) {
        const std::wstring from=pair.first,to=pair.second;
        const bool fragment=from==L"Elde: " || from==L" · Seçimler ayrı korunur." ||
            from==L" seçenek  ·  Float " || from==L"CS2 bağlantısı kuruldu. PID " ||
            from==L"Görünüm geri alınamadı: " || from==L"Eldiven geri alınamadı: " ||
            from==L"İşlem tamamlanamadı: " || from==L"İstek: paint=";
        if(!fragment)continue;
        std::size_t pos=0;
        while((pos=value.find(from,pos))!=std::wstring::npos) {value.replace(pos,from.size(),to);pos+=to.size();}
    }
    return value;
}
