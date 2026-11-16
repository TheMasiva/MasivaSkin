<div align="center">

# 💜 MASIVA SKIN

### Counter-Strike 2 · Client-Side Cosmetic Customization

**Windows x64 · English & Türkçe · External Overlay Menu**

[![Windows x64](https://img.shields.io/badge/Windows-x64-303040?style=flat-square&logo=windows)](https://github.com/TheMasiva/MasivaSkin/releases)
[![Languages](https://img.shields.io/badge/Languages-EN%20%7C%20TR-6d4aff?style=flat-square)](#languages--diller)
[![GitHub Releases](https://img.shields.io/badge/Downloads-GitHub%20Releases-8257e6?style=flat-square&logo=github)](https://github.com/TheMasiva/MasivaSkin/releases)
[![Discord](https://img.shields.io/badge/Discord-Join%20Community-5865F2?style=flat-square&logo=discord&logoColor=white)](https://discord.gg/V8ZGrrVwym)
[![YouTube Teaser](https://img.shields.io/badge/YouTube-Watch%20Teaser-FF0033?style=flat-square&logo=youtube&logoColor=white)](https://www.youtube.com/watch?v=d6i8fmS4o8g)

**[📦 Downloads](https://github.com/TheMasiva/MasivaSkin/releases) · [🎬 Official Teaser](https://www.youtube.com/watch?v=d6i8fmS4o8g) · [💬 Discord Community](https://discord.gg/V8ZGrrVwym) · [🛠 Troubleshooting](#troubleshooting) · [🇹🇷 Türkçe](#turkce)**

*An independent, unofficial project. Not affiliated with or endorsed by Valve Corporation.*

</div>

<div align="center">

### 🎬 Official Teaser · Resmî Tanıtım

[![Masiva Skin — Official Teaser](https://img.youtube.com/vi/d6i8fmS4o8g/hqdefault.jpg)](https://www.youtube.com/watch?v=d6i8fmS4o8g)

**[▶ Watch the Official Teaser / Yeni tanıtımı izle](https://www.youtube.com/watch?v=d6i8fmS4o8g)**

**[🎥 Previous Showcase / Önceki tanıtım](https://www.youtube.com/watch?v=JX4N9-Rs1TE) · [💜 Join our Discord / Discord'a katıl](https://discord.gg/V8ZGrrVwym)**

</div>

---

<a id="languages--diller"></a>
**Languages / Diller:** English (first) · [Türkçe](#turkce)

> **About updates:** This README is the **permanent project guide**, not a changelog. For new features, compatibility updates, fixes, downloads and release-specific instructions, always read the **[GitHub Releases](https://github.com/TheMasiva/MasivaSkin/releases)** page.

# 🇬🇧 English

## What is Masiva Skin?

**Masiva Skin** is a Windows application for managing **local cosmetic selections in Counter-Strike 2**. It offers a visual interface for weapons, knives, gloves and agents, plus saved profiles and a menu that can be opened over the game in borderless/windowed display modes.

The project is designed around **client-side appearance customization**. It is **not** a Steam inventory editor, does not grant ownership of items and does not change Steam trading or marketplace records. Visual results depend on the supported game build and the selected item.

> **Please read before installing:** This is third-party game modification software. It is not approved by Valve; using it can violate game/platform rules or lead to account restrictions. **No VAC, Trusted Mode, Trust Factor or account-safety guarantee is provided.**

## Features

| Area | What it offers |
| --- | --- |
| **Weapons & finishes** | Browse supported weapon models and paint finishes using visual selection cards. |
| **Knives & gloves** | Select supported knife and glove appearances, with applicable finish options. |
| **Agents** | Configure supported agent appearances for T and CT individually. |
| **Team loadouts** | Maintain separate **T / CT** selections; use **Both** for items shared by both teams where available. |
| **Appearance details** | Customize applicable wear/float, paint seed, StatTrak and name tags; sticker and charm options depend on item/build support. |
| **Visual previews** | Browse offline preview images included in the release archive. |
| **Overlay menu** | Open or hide the external Windows menu with a configurable keyboard shortcut (**Insert** by default). |
| **Appearance settings** | Light/dark themes, custom accent color, English/Turkish interface. |
| **Loadout / Quick Loadout** | Preview T/CT items side by side, choose and load profiles, or reapply your current selections from the integrated Loadout page. |
| **Profiles** | Save, preview and load `.masiva` files. Profile overwrites keep up to five rotating `.bak` backups with a restore action. |
| **Recent skins & filters** | Revisit the latest 20 distinct successfully submitted selections; remove individual active filters using their chips. |
| **Compact pages & themes** | Loadout, Profiles, Recent Skins, Filters and customization tools open inside the main window. Choose preset or custom main/accent colours, opacity and menu size. |
| **Restore All** | Clear active cosmetic overrides across the current T/CT session; immediately restore available originals, while holstered/unavailable items may be restored when equipped again. Saved profile files and favorites are not deleted. |
| **Local team previews** | Where supported, the local player’s selected agent/gloves and the weapon held by the presentation scene may be reflected in team introductions or end-of-match presentations. Other players’ preview characters are not modified. |
| **Diagnostics** | Connection status, compatibility feedback, application logs and diagnostic report export, including available team-preview callback counters and timings. |

Additional HUD, killfeed and agent-audio functionality may be available in supported builds. These effects are **not guaranteed** for every game update or item; refer to the relevant release notes.

### How the menu works

Masiva Skin uses an **external Windows overlay**, **not a native game-engine menu**. It is intended for **Fullscreen Windowed (Borderless)** or windowed mode. **Exclusive Fullscreen is not supported by the overlay.**

### Watch the videos

- **[🎬 Official teaser — Masiva Skin](https://www.youtube.com/watch?v=d6i8fmS4o8g)** — our latest promotional video.
- **[🎥 Original project showcase](https://www.youtube.com/watch?v=JX4N9-Rs1TE)** — an earlier look at the project; its interface may differ from newer builds.

Videos demonstrate the project; the **[release notes](https://github.com/TheMasiva/MasivaSkin/releases)** remain the source for current supported features and CS2 compatibility.

### Community & support

**[💬 Join the Masiva Skin Discord Community](https://discord.gg/V8ZGrrVwym)** to discuss the project, get installation help, report bugs and share feature ideas. You can find the FAQ, known issues, announcements and support tickets there. Only download software from release sources announced by the project team.

## Download & installation

1. Open **[Releases](https://github.com/TheMasiva/MasivaSkin/releases)** and select the build you want to install. **Read its release notes** to check which CS2 version it supports.
2. Download the **Windows x64 ZIP** from **Assets** (not GitHub's automatic *Source code* ZIP).
3. **Close CS2 and Masiva Skin completely** before installing or replacing files.
4. Extract the **entire ZIP** into a new local folder. **Never mix an EXE and bridge DLL from different releases.**
5. Keep the files and `previews/` directory in their original relative locations.
6. Run `MasivaSkin.exe`. If CS2 is not running, the app offers **Launch CS2** through Steam; if it is running, the app proceeds to the menu and attempts to connect. Use **Fullscreen Windowed / Borderless** or Windowed mode for the overlay.
7. Select a team, category, item and finish, then choose **Apply**. Use **Save profile** to keep your selections.

A typical **release ZIP** contains:

```text
MasivaSkin/
├── MasivaSkin.exe
├── MasivaSkinBridge.dll
├── Masiva-Skin.ini
├── Explore-menu.cmd             # Optional UI preview, no game required
├── previews/
├── README.md
├── SHA256.txt                 # If supplied with the release
└── THIRD-PARTY-NOTICES/       # If supplied with the release
```

> Extract first: running directly inside a ZIP or copying only `MasivaSkin.exe` can cause missing images, DLL errors or missing configuration.

### Updating to a new release

- Back up any `.masiva` profiles and existing settings you want to keep. Overwrites of saved profiles can produce rotating `.bak1`–`.bak5` files; keep an external copy before upgrading.
- Quit the game and app; extract the **new release into a fresh folder** instead of overwriting a mix of old/new files.
- Import your saved profile with **Load profile** after starting the new build.
- Read the new release notes for compatibility or profile-format notices.

## Quick usage reference

| Control / option | Action |
| --- | --- |
| **Insert** | Show/hide the menu (default hotkey). |
| **Menu key** | Change the menu shortcut; **Esc** cancels reassignment. |
| **Automatic connection** | Waits for CS2, connects when ready, and exits after the watched game closes. |
| **T / CT** | Configure each team's cosmetic selections and agents. |
| **Both** | Apply a compatible shared item to both team loadouts; agents remain team-specific. |
| **Apply** | Submit the selected appearance for the applicable item/team. |
| **Restore** | Clear the applicable cosmetic override. |
| **Restore All** | Clear active session overrides for the applicable T/CT state and restore accessible original items; unequipped items can finish restoration when re-equipped. Does **not** delete `.masiva` profiles or favorites. |
| **Loadout** | Open the two-column T/CT collection; Quick Loadout actions are inside this page, not a separate top-bar button. |
| **Recent skins** | Choose from the last 20 distinct successfully submitted selections; selecting one does not automatically apply it. |
| **Save profile / Load profile** | Save/load a `.masiva` cosmetic profile. |
| **Menu & Theme Colour** | Choose independent main background and accent colours, theme preset, opacity and menu size. |
| **Explore-menu.cmd** | Open `--preview` mode to inspect the menu without attaching to CS2. |
| **Language** | Switch the interface between English and Turkish. |
| **Settings** | Theme, accent color, language, menu key, opacity, size and diagnostics. |
| **Log / diagnostics** | Inspect errors and save a diagnostic report. |
| **Hide / Exit** | Hide the overlay or close the application. |

**Inline navigation:** Loadout, Profiles, Recent Skins, Filters, Customize and Stickers/Charms open within the main window. Use the top navigation / Back control to return. The selected card border is separate from the successful submit checkmark; neither alone verifies the visible in-game result.

**Both mode:** Only supported items shared by both teams are eligible. Merely switching to Both does not apply changes; use **Apply** or save the profile. Changes to the other side may appear when its item is next equipped. Agents and team-exclusive weapons are configured separately.

Interface preferences are generally stored in `Masiva-Skin.ini`, while cosmetic selections are saved in `.masiva` profiles. Discord Rich Presence is optional and disabled by default in this distribution. Windows native file/color pickers can still appear outside the main window.

## Known limitations

- **CS2 updates can break compatibility.** A matching build/version check does not prove every cosmetic operation is working. Unsupported module identities or signatures should be treated as a reason to **stop**, not force an older build.
- **The overlay requires borderless/windowed display.** Exclusive fullscreen is not supported.
- **The bridge may be blocked or fail to initialize** because of Windows protection, mismatched files, unsupported CS2 builds or third-party software restrictions.
- **Performance:** decoded preview images and scaled copies have separate cache budgets (32 MiB and 24 MiB in the source validation record). These are not total application RAM limits, and measured game FPS improvements are not guaranteed.
- **Visual previews are catalog images**, not necessarily exact in-game renders for every wear, sticker, charm or lighting combination.
- **Some changes may not appear immediately.** Equipping/re-equipping an item or switching teams can affect when its visible state updates.
- **Team presentation scenes are game-controlled.** Only the local player’s identified scene character and the weapon held by that scene are eligible; the game chooses that weapon. Presentation rendering, Restore All visual effects and FPS impact can require separate live verification. Restore All cannot instantly rewrite every unequipped weapon.
- **Item-specific features may vary** by the active CS2 build; not every cosmetic, glove, agent, HUD effect, voice effect or decoration is guaranteed.
- **Changes are intended to be local.** They do not confer real ownership and may not be visible to other players.

## Troubleshooting

### 1. Bridge DLL does not load / "bridge not ready" / Apply fails

First, close CS2 and the app. Extract a fresh release, checking that `MasivaSkin.exe` and `MasivaSkinBridge.dll` are **from the same build**, and that Windows Security **Protection history** has not flagged a file. Do **not** disable security software to force a file to run.

Some players reported resolving a bridge-load problem by adding Valve's documented launch option to CS2:

```text
-allow_third_party_software
```

**Steam → Library → Counter-Strike 2 → Properties → General → Launch Options.** Add the parameter, separated by a space from any existing options, and fully restart CS2 if you choose to use it.

**Important safety notice:** This setting allows certain third-party software but **does not approve Masiva Skin, bypass all restrictions, or guarantee VAC/account safety**. Valve notes that running outside Trusted Mode **may negatively affect Trust Factor** (not an automatic penalty); some injections may still be blocked, and **unsigned DLL injections can result in insecure mode, preventing access to VAC-secure servers**. The option will not fix every DLL issue. See [Valve's official CS2 Trusted Mode FAQ](https://help.steampowered.com/en/faqs/view/09A0-4879-4353-EF95).

If it still fails, save **Log / diagnostics → Diagnostic report** and check the error details rather than retrying blindly.

### 2. The menu is missing or does not respond

- Set CS2 to **Fullscreen Windowed (Borderless)** or **Windowed**, not Exclusive Fullscreen.
- Press **Insert** or your configured **Menu key** and check if the app is hidden in the system tray.
- Close conflicting overlay windows; check the application's logs if the game window is not detected.

### 3. Skin or preview does not appear

- Verify `previews/` is intact and beside the EXE as provided in the ZIP.
- Verify the game is supported by your installed Masiva Skin release.
- Check **T / CT / Both**, the currently equipped item and whether **Apply** succeeded.
- Re-equip the item if needed. A visual mismatch alone does not mean an item was granted to your Steam inventory.

### Team intro / end-of-match preview or Restore All not behaving as expected

- Check the matching release notes; preview support can depend on the exact game build and scene type.
- Ensure the local player is recognized, the correct T/CT selection is active, and the scene is displaying the relevant item.
- **Restore All may defer holstered items until equipped**; this is different from an immediate restore of every inventory world model.
- Export **Log / diagnostics → Diagnostic report**, including any available **Team preview** counters (`ready`, `local_calls`, `updates`, `errors`, skipped callbacks and callback time). Omit private identifiers before sharing logs.
- If the current CS2 build is unsupported or the scene is repeatedly failing, stop using the build rather than forcing compatibility.

### 4. Problems after a CS2 update

Check [Releases](https://github.com/TheMasiva/MasivaSkin/releases) for a compatibility update. If you see **unsupported build**, **signature mismatch** or similar errors, **do not force an older version or reuse old addresses**. Wait until the affected release is verified for the new game build.

### 5. Saved profile or settings do not persist

- Use **Save profile** after making cosmetic changes; use **Load profile** when importing a previous `.masiva` file.
- Keep the application in a folder where Windows permits writing configuration files.
- Back up your profile before upgrading or switching release folders.

### 6. Crash, FPS drop or unexpected behavior

Close Masiva Skin and verify that CS2 works normally without it. Confirm files belong to one release, avoid combining multiple game-modifying tools, and review Windows Event Viewer plus the application's diagnostic report. If the game crashes repeatedly, **stop testing that build** until the cause is understood.

### Reporting a bug

Use [GitHub Issues](https://github.com/TheMasiva/MasivaSkin/issues) (if available), or the **bug-report/support channels on our [Discord server](https://discord.gg/V8ZGrrVwym)**. Include:

- Masiva Skin **release name**, CS2 **build/version**, Windows version, and relevant display mode.
- Expected vs. actual behavior and steps to reproduce.
- **Log / diagnostics → Diagnostic report**, plus an error screenshot if useful.

Please **remove personal information, tokens, account identifiers and private file paths** before posting public logs.

## Building from source

The repository includes C++ sources, CMake configuration, a Windows build script and development tests (in the source archive/repository; not in the portable binary ZIP). A typical Windows development setup uses **Visual Studio 2022**, **Desktop development with C++** and **CMake**.

From the source root:

```bat
build-windows.cmd
```

The script configures an **x64 Release** build and runs the included tests. The current build script and release-specific documentation are authoritative if the process changes. Passing automated tests does **not** guarantee live CS2 compatibility.

## Integrity, licenses & disclaimer

- When a release provides a `.sha256` file, verify the downloaded ZIP with PowerShell:

  ```powershell
  Get-FileHash .\YOUR_DOWNLOADED_RELEASE.zip -Algorithm SHA256
  ```

  Compare the value with the matching published checksum. It confirms file integrity relative to that checksum, **not that the software is safe or Valve-approved**.
- **Source licensing:** The project's original source code is currently published **without a project license** (`No license`). Public repository visibility does not grant permission to copy, modify, redistribute or reuse it beyond rights provided by law or the platform's terms. A project license may be added later.
- Source attribution and notices for third-party dependencies remain in [`third_party/`](third_party/) or the release's `THIRD-PARTY-NOTICES/` folder. Those components retain their own licenses. Third-party metadata licenses do **not** transfer ownership of Valve or Workshop artwork.
- The application is independent of Valve; Valve game DLLs are **not included** in the distributed package.
- Use this software only where permitted by the game's terms and applicable rules. **No anti-cheat immunity, matchmaking access or account safety is guaranteed.**

**Project:** [GitHub repository](https://github.com/TheMasiva/MasivaSkin) · **Updates:** [Releases](https://github.com/TheMasiva/MasivaSkin/releases) · **Community:** [Discord](https://discord.gg/V8ZGrrVwym) · **Videos:** [Official teaser](https://www.youtube.com/watch?v=d6i8fmS4o8g) / [Previous showcase](https://www.youtube.com/watch?v=JX4N9-Rs1TE)

---

<a id="turkce"></a>
# 🇹🇷 Türkçe

> **Güncellemeler hakkında:** Bu README, her sürümde değiştirilmesi gerekmeyen **kalıcı kullanım kılavuzudur**. Yeni özellikler, hotfix'ler, desteklenen CS2 sürümleri ve indirmeler için **[GitHub Releases](https://github.com/TheMasiva/MasivaSkin/releases)** bölümüne bakın.

## Masiva Skin nedir?

**Masiva Skin**, Counter-Strike 2 için geliştirilen, **istemci tarafındaki kozmetik görünüm seçimlerini** yönetmeye odaklanan bir Windows uygulamasıdır. Silah, bıçak, eldiven ve ajan görünümleri için görsel seçim kartları, profil kaydetme/yükleme ve oyun üzerinde açılabilen harici bir menü sunar.

Amaç **yerel görünüm özelleştirmesidir**. Steam envanterine eşya eklemez, eşya sahipliği vermez; Steam Pazarı veya takas kayıtlarını değiştirmez. Görsel sonuçlar desteklenen oyun sürümüne ve eşyaya bağlıdır.

> **Kurulumdan önce:** Bu, Valve tarafından onaylanmayan üçüncü taraf bir oyun modifikasyonudur. Kullanımı oyun/platform kurallarını ihlal edebilir veya hesap kısıtlamalarına yol açabilir. **VAC, Trusted Mode, Trust Factor ya da hesap güvenliği garantisi yoktur.**

## Özellikler

| Bölüm | Açıklama |
| --- | --- |
| **Silahlar ve kaplamalar** | Desteklenen silah ve skinleri görsel kartlarla seçme. |
| **Bıçaklar ve eldivenler** | Uyumlu bıçak, eldiven ve kaplama seçenekleri. |
| **Ajanlar** | T ve CT için ayrı ajan görünümü seçimleri. |
| **Takım ayarları** | Ayrı **T / CT** kayıtları; uygun ortak eşyalar için **Both**. |
| **Kozmetik ayrıntıları** | Desteklenen eşyalarda float/wear, seed, StatTrak, NameTag, sticker ve charm seçenekleri. |
| **Görsel önizlemeler** | ZIP paketine dahil çevrimdışı skin görselleri. |
| **Harici menü** | Varsayılan **Insert** tuşuyla açılıp kapanan, tuşu değiştirilebilir Windows overlay. |
| **Görünüm ve dil** | Siyah/beyaz tema, özel vurgu rengi, Türkçe/İngilizce. |
| **Loadout / Quick Loadout** | T/CT görsellerini yan yana inceleme; entegre Loadout sayfasından profil seçme/yükleme ve mevcut kombinasyonu yeniden uygulama. |
| **Profiller** | `.masiva` profillerini kaydetme, resimli önizleme ve yükleme. Profilin üzerine yazıldığında `.bak1`–`.bak5` yedekleri ve geri yükleme seçeneği. |
| **Son kullanılanlar / filtreler** | Başarıyla gönderilen son 20 farklı skin seçimi; aktif filtre etiketlerini ayrı ayrı kaldırma. |
| **Kompakt menü ve temalar** | Loadout, Profiles, Recent Skins, Filters ve özelleştirme sayfaları ana pencerenin içinde açılır. Hazır paletler veya ayrı ana zemin/vurgu renkleri, opaklık ve boyut ayarları. |
| **Restore All / Tümünü geri al** | Oturumdaki T/CT kozmetik değişikliklerini temizler; hazır özgün verileri geri yükler, elde olmayan eşyaların geri yüklenmesi yeniden kuşanmaya kalabilir. Kayıtlı profil ve favoriler silinmez. |
| **Yerel takım tanıtımı** | Desteklenen sahnelerde yalnızca yerel oyuncunun tanıtım karakteri için ajan, eldiven ve sahnenin tuttuğu silah görünümü kullanılabilir. Başka oyuncuların tanıtım modelleri değiştirilmez. |
| **Tanılama** | Bağlantı/uyumluluk durumları, loglar ve mevcutsa Team Preview sayaçları ile sürelerini içeren tanı raporu. |

Uyumlu derlemelerde HUD, killfeed ve ajan sesleriyle ilgili ek özellikler bulunabilir. Bunların hepsi her oyun güncellemesinde veya her eşya için **garanti edilmez**; sürüme özel bilgiler Releases bölümündedir.

### Menü nasıl çalışıyor?

Masiva Skin'in menüsü oyun motorunda çizilen yerel bir menü değil, **harici Windows overlay** sistemidir. **Pencereli Tam Ekran (Borderless)** veya **Pencereli** mod için tasarlanmıştır. **Özel Tam Ekran (Exclusive Fullscreen) overlay tarafından desteklenmez.**

### Tanıtım videoları

- **[🎬 Yeni resmî tanıtım — Masiva Skin](https://www.youtube.com/watch?v=d6i8fmS4o8g)** — projenin yeni teaser videosu.
- **[🎥 Önceki proje tanıtımı](https://www.youtube.com/watch?v=JX4N9-Rs1TE)** — eski sürümlerden görüntüler içerebilir.

Videolar projeyi tanıtır. **Güncel özellikler ve CS2 uyumluluğu** için her zaman [Release notlarını](https://github.com/TheMasiva/MasivaSkin/releases) esas al.

### Topluluk ve destek

**[💬 Masiva Skin Discord topluluğuna katıl](https://discord.gg/V8ZGrrVwym)** — kurulum yardımı, FAQ, bilinen sorunlar, hata bildirimleri, duyurular ve yeni özellik önerileri için topluluk sunucumuzu kullanabilirsin. Programı yalnızca geliştirici ekibin duyurduğu resmî kaynaklardan indir.

## İndirme ve kurulum

1. **[Releases](https://github.com/TheMasiva/MasivaSkin/releases)** sayfasından kurmak istediğin sürümü seç. Önce sürüm açıklamasındaki **CS2 uyumluluk** notlarını oku.
2. **Assets** altındaki **Windows x64 ZIP** dosyasını indir. GitHub'ın otomatik oluşturduğu *Source code* ZIP'ini uygulama paketi sanma.
3. Dosyaları kurmadan/değiştirmeden önce **CS2 ve Masiva Skin'i tamamen kapat**.
4. ZIP'in **tamamını yeni bir yerel klasöre çıkar**. **Farklı sürümlerin EXE ve DLL dosyalarını karıştırma.**
5. EXE, köprü DLL'i, ayar dosyası ve `previews/` klasörünün paket içindeki dizilimini koru.
6. `MasivaSkin.exe` uygulamasını çalıştır. CS2 kapalıysa **Launch CS2** düğmesiyle Steam üzerinden açabilirsin; açıksa uygulama menüye geçip bağlanmayı dener. Overlay için **Pencereli Tam Ekran (Borderless)** veya **Pencereli** kullan.
7. Takım → kategori → eşya → skin seçimini yapıp **Apply** kullan. Seçimleri saklamak için **Save profile** kullan.

**Örnek Release klasörü:**

```text
MasivaSkin/
├── MasivaSkin.exe
├── MasivaSkinBridge.dll
├── Masiva-Skin.ini
├── Explore-menu.cmd             # Oyun olmadan menü önizlemesi
├── previews/
├── README.md
├── SHA256.txt                 # Varsa
└── THIRD-PARTY-NOTICES/       # Varsa
```

> ZIP içinden doğrudan EXE açma. Yalnızca EXE'yi başka klasöre kopyalamak, görsel veya DLL bulunamaması gibi hatalar oluşturabilir.

### Yeni sürüme geçiş

- Önemli `.masiva` profillerini ve eski ayarlarını yedekle. Profillerin üzerine kayıtta `.bak1`–`.bak5` geçmişi oluşturulabilir; güncellemeden önce ayrıca harici bir kopya sakla.
- CS2 ve uygulamayı kapat; yeni sürümü **temiz bir klasöre** çıkar.
- **Load profile** ile kaydettiğin profili geri yükle.
- Yeni sürümün Release notlarındaki uyumluluk ve profil bilgilerini kontrol et.

## Hızlı kullanım tablosu

| Kontrol | Ne yapar? |
| --- | --- |
| **Insert** | Varsayılan menü açma/kapatma tuşu. |
| **Menu key** | Menü tuşunu değiştir; **Esc** işlemi iptal eder. |
| **Otomatik bağlantı** | CS2 açılmasını bekler, hazır olduğunda bağlanır ve takip edilen oyun kapandığında çıkar. |
| **T / CT** | Takımlara özel skin ve ajan seçimleri. |
| **Both** | Uyumlu ortak eşyayı T ve CT'ye birlikte uygular; ajanlar ayrı kalır. |
| **Apply** | Seçilen görünümü uygular. |
| **Restore** | İlgili görünüm değişikliğini temizler. |
| **Restore All / Tümünü geri al** | Oturumdaki aktif T/CT seçimlerini temizler ve erişilebilir özgün verileri geri yükler; elde olmayan silahlar tekrar kuşanılınca tamamlanabilir. `.masiva` profil ve favorilerini silmez. |
| **Loadout** | İki sütunlu T/CT koleksiyonunu açar; Quick Loadout işlemleri artık burada, ayrı üst menü düğmesinde değil. |
| **Recent skins** | Başarıyla gönderilmiş son 20 farklı görünümden seçim yapar; tek başına uygulamaz. |
| **Save profile / Load profile** | `.masiva` profili kaydeder/yükler. |
| **Menu & Theme Colour** | Bağımsız ana zemin ve vurgu renkleri, tema paleti, opaklık ve menü boyutu. |
| **Explore-menu.cmd** | Oyun bağlantısı olmadan `--preview` modunda menüyü açar. |
| **Language** | İngilizce / Türkçe dili değiştirir. |
| **Ayarlar** | Tema, vurgu rengi, dil, menü tuşu, opaklık, boyut ve tanılama. |
| **Log / diagnostics** | Hata ayrıntıları ve tanı raporu. |
| **Hide / Exit** | Menüyü gizler / uygulamadan çıkar. |

**Menü içinde gezinme:** Loadout, Profiles, Recent Skins, Filters, Customize ve Stickers/Charms ana pencere içinde açılır. Geri / üst gezinme kontrolüyle dönülür. Seçili kartın çerçevesi ile başarılı gönderim işareti farklıdır; hiçbiri tek başına oyundaki görünümü doğrulamaz.

**Both hakkında:** Yalnızca iki takımın ortak kullanabildiği uyumlu eşyalar içindir. Both sekmesine geçmek tek başına ayar uygulamaz; **Apply** veya profil kaydı gerekir. Diğer taraftaki görünüm, eşya tekrar kuşanıldığında güncellenebilir. Ajanlar ve tek takıma özel silahlar ayrı ayarlanır.

Menü teması ve tuş gibi ayarlar genellikle `Masiva-Skin.ini` dosyasında, kozmetik seçimler `.masiva` profilinde tutulur. Bu dağıtımda Discord Rich Presence varsayılan kapalıdır. Standart Windows dosya/renk seçicileri ana pencere dışında açılabilir.

## Bilinen sınırlamalar

- **CS2 güncellemeleri uyumluluğu bozabilir.** Build numarasının eşleşmesi bütün görsel işlemlerin çalıştığını kanıtlamaz. Desteklenmeyen modül kimliği/imza durumunda **uygulamayı zorlamayın**.
- **Overlay yalnızca pencereli modlarda** desteklenir; Exclusive Fullscreen'de görünmeyebilir.
- **Köprü DLL'i yüklenmeyebilir**: Windows koruması, sürüm uyuşmazlığı, desteklenmeyen oyun sürümü veya üçüncü taraf yazılım kısıtlamaları neden olabilir.
- **Performans:** çözülmüş ve ölçeklenmiş görseller için ayrı önbellek bütçeleri kullanılır (kaynak doğrulama kaydında 32 MiB + 24 MiB). Bunlar uygulamanın toplam RAM sınırları değildir; FPS artışı garanti edilmez.
- **Skin görselleri katalog önizlemesidir**; float, sticker, charm ve ışık gibi ayrıntılar oyundaki görüntüden farklı olabilir.
- **Bazı değişiklikler anında görünmeyebilir.** Eşyayı yeniden kuşanmak veya takım değiştirmek gerekebilir.
- **Takım tanıtımı oyun tarafından yönetilir.** Sadece kimliği eşleşen yerel oyuncunun sahne karakteri ve oyun tarafından sahnede tutturulan silah desteklenir. Gerçek görsel sonuç ve FPS etkisi ayrı canlı test ister. Restore All elde olmayan bütün silahları tek karede geri yüklemez.
- **Eşyaya ve sürüme göre özellikler değişebilir.** Eldiven, ajan, HUD, killfeed, ses ve dekorasyon özelliklerinin her durumda çalışacağı garanti edilmez.
- **Değişiklikler yerel kullanım içindir**; envantere gerçek eşya eklemez ve diğer oyuncuların bunları görmesini garanti etmez.

## Bilinen sorunlar ve çözümler

### 1. Köprü DLL'i yüklenmiyor / "bridge not ready" / Apply başarısız

Önce CS2 ve Masiva Skin'i tamamen kapat. Güncel Release'i yeni klasöre çıkar. `MasivaSkin.exe` ile `MasivaSkinBridge.dll` dosyalarının **aynı sürüme ait olduğunu** kontrol et. Windows Güvenliği → **Koruma geçmişi** bölümünde dosyanın engellenip engellenmediğine bak. Güvenlik yazılımını zorla çalıştırmak için kapatma.

Bazı kullanıcılarda köprü yükleme hatası, Valve'ın belgelediği aşağıdaki CS2 başlatma seçeneğiyle çözülmüştür:

```text
-allow_third_party_software
```

**Steam → Kütüphane → Counter-Strike 2 → Özellikler → Genel → Başlatma Seçenekleri.** Kullanmayı tercih edersen varsa diğer seçenekleri silmeden araya boşluk koyarak ekle; ardından CS2'yi tamamen yeniden başlat.

**Önemli güvenlik uyarısı:** Bu parametre **Masiva Skin'in Valve tarafından onaylandığı, VAC açısından güvenli olduğu veya tüm kısıtlamaları kaldırdığı anlamına gelmez.** Valve'a göre Trusted Mode dışında oynama **Trust Factor'ı olumsuz etkileyebilir** (otomatik ceza değildir); bazı DLL yüklemeleri yine engellenebilir ve **imzasız DLL yüklemeleri güvensiz moda yol açarak VAC korumalı sunuculara girişi engelleyebilir**. Her DLL hatası bu komutla çözülmez. Resmî bilgi: [Valve — CS2 Trusted Mode](https://help.steampowered.com/en/faqs/view/09A0-4879-4353-EF95).

Sorun sürerse **Log / diagnostics → Diagnostic report** ile rapor al ve hata nedenini incele.

### 2. Menü açılmıyor veya tıklanamıyor

- Oyunu **Pencereli Tam Ekran (Borderless)** veya **Pencereli** yap; Exclusive Fullscreen kullanma.
- **Insert** veya ayarladığın **Menu key** tuşuna bas; uygulamanın sistem tepsisine gizlenip gizlenmediğine bak.
- Çakışan diğer overlay pencerelerini kapat; oyun penceresi tespit edilmiyorsa loglara bak.

### 3. Skin veya önizleme görünmüyor

- `previews/` klasörünün eksiksiz ve EXE'nin yanında olduğundan emin ol.
- Kullandığın Masiva Skin sürümünün güncel CS2 sürümünü desteklediğini doğrula.
- **T / CT / Both**, eldeki eşya ve **Apply** sonucunu kontrol et.
- Gerekirse eşyayı yeniden kuşan. Görsel değişiklik Steam envanterine eşya eklemez.

### Takım tanıtımı veya Restore All beklenen sonucu vermiyor

- Sürüm notlarında ilgili CS2 derlemesi ve sahne türünün desteklenip desteklenmediğini kontrol et.
- T/CT seçimini ve sahnede gerçekten gösterilen eşyayı doğrula; yalnızca eşleşen yerel oyuncu karakteri değiştirilir.
- **Restore All elde olmayan silahların geri yüklenmesini tekrar kuşanılana kadar erteleyebilir.** Bu bütün envanterin anında sıfırlanması değildir.
- **Log / diagnostics → Diagnostic report** ile Team preview alanlarını (`ready`, `local_calls`, `updates`, `errors`, atlanan callback'ler ve süreler) kontrol et. Raporu paylaşmadan kişisel bilgileri temizle.
- Oyun sürümü uyumsuzsa zorlamadan kullanmayı durdur.

### 4. CS2 güncellemesinden sonra çalışmıyor

**[Releases](https://github.com/TheMasiva/MasivaSkin/releases)** sayfasında uyumluluk güncellemesi olup olmadığına bak. **Unsupported build**, **signature mismatch** veya benzeri hata varsa eski sürümü zorlamadan kullanmayı durdur. Yeni oyun derlemesi doğrulanana kadar bekle.

### 5. Profil veya ayarlar kaydedilmiyor

- Skin seçimlerini **Save profile** ile kaydet ve `.masiva` dosyasını **Load profile** ile yükle.
- Windows'un dosya yazmaya izin verdiği bir klasör kullan.
- Yeni sürüme geçmeden önce profili yedekle.

### 6. Çökme, FPS düşüşü veya beklenmeyen davranış

Masiva Skin'i kapatıp CS2'nin tek başına düzgün çalıştığını kontrol et. Aynı sürümden dosyalar kullandığından emin ol; birden fazla oyun modifikasyon aracını aynı anda çalıştırmaktan kaçın. **Windows Olay Görüntüleyicisi** ve uygulama tanı raporunu incele. Çökmeler tekrarlanıyorsa o derlemeyi kullanmayı bırak.

### Hata nasıl bildirilir?

[GitHub Issues](https://github.com/TheMasiva/MasivaSkin/issues) (açıksa) veya **[Discord topluluğumuzdaki](https://discord.gg/V8ZGrrVwym) hata bildirimi/destek kanallarında** şu bilgileri paylaş:

- Masiva Skin **sürümü**, CS2 **build numarası**, Windows sürümü ve ekran modu.
- Beklenen/gerçekleşen davranış ve hatayı yeniden üretme adımları.
- **Log / diagnostics → Diagnostic report** ve gerekirse ekran görüntüsü.

Herkese açık log paylaşmadan önce **kişisel bilgilerini, token'ları, hesap tanımlayıcılarını ve özel dosya yollarını** gizle.

## Kaynak koddan derleme

Depoda C++ kodları, CMake yapılandırması, Windows derleme betiği ve testler bulunur. Genellikle **Visual Studio 2022**, **Desktop development with C++** ve **CMake** yeterlidir.

Kaynak kodun ana klasöründe:

```bat
build-windows.cmd
```

Betik **x64 Release** derlemesini hazırlar ve testleri çalıştırır. Derleme işlemi değişirse depodaki güncel betik ve ilgili Release notları esas alınır. Otomatik testlerin geçmesi, canlı CS2 uyumluluğunu garanti etmez.

## Dosya doğrulama, lisanslar ve sorumluluk reddi

- Release yanında `.sha256` dosyası verilmişse, Windows PowerShell ile ZIP'in hash değerini kontrol et:

  ```powershell
  Get-FileHash .\YOUR_DOWNLOADED_RELEASE.zip -Algorithm SHA256
  ```

  Çıktıyı yayımlanan checksum ile karşılaştır. Bu, dosyanın belirtilen hash ile eşleştiğini gösterir; **yazılımın güvenli veya Valve onaylı olduğunu kanıtlamaz**.
- **Kaynak kod lisansı:** Projenin özgün kaynak kodu şu anda **proje lisansı olmadan** (`No license`) yayımlanmaktadır. Deponun herkese açık olması, kanunen veya platform şartları kapsamında zaten var olan haklar dışında kodu kopyalama, değiştirme, yeniden dağıtma ya da başka projelerde kullanma izni vermez. Daha sonra bir proje lisansı eklenebilir.
- Üçüncü taraf bağımlılıkların bildirimleri [`third_party/`](third_party/) veya Release içindeki `THIRD-PARTY-NOTICES/` klasöründedir. Bu bileşenler kendi lisanslarına tabidir. Metadata lisansları Valve/Workshop görsellerinin telif haklarını devretmez.
- Masiva Skin, Valve'dan bağımsızdır. Valve'ın oyun DLL dosyaları Release paketine **dahil edilmez**.
- Programı yalnızca oyun/platform kuralları izin veriyorsa kullan. **Anti-cheat bağışıklığı, eşleştirme erişimi veya hesap güvenliği garantisi verilmez.**

**Proje:** [GitHub](https://github.com/TheMasiva/MasivaSkin) · **Güncellemeler:** [Releases](https://github.com/TheMasiva/MasivaSkin/releases) · **Topluluk:** [Discord](https://discord.gg/V8ZGrrVwym) · **Videolar:** [Yeni tanıtım](https://www.youtube.com/watch?v=d6i8fmS4o8g) / [Önceki tanıtım](https://www.youtube.com/watch?v=JX4N9-Rs1TE)

---

<div align="center">

**Made by Masiva 💜**

[💬 Discord Community](https://discord.gg/V8ZGrrVwym) · [🎬 Official Teaser](https://www.youtube.com/watch?v=d6i8fmS4o8g)

*Project information lives here. Version-specific changes live in [Releases](https://github.com/TheMasiva/MasivaSkin/releases).*

</div>