<div align="center">

# 💜 MASIVA SKIN

### Counter-Strike 2 · Client-Side Cosmetic Customization

**Windows x64 · 6 Interface Languages · Windows Setup & Signed Updates · The Kurumi Era (1.X)**

[![Windows x64](https://img.shields.io/badge/Windows-x64-303040?style=flat-square&logo=windows)](https://github.com/TheMasiva/MasivaSkin/releases)
[![Languages](https://img.shields.io/badge/Languages-6%20Languages-6d4aff?style=flat-square)](#languages--diller)
[![GitHub Releases](https://img.shields.io/badge/Downloads-Windows%20Setup-8257e6?style=flat-square&logo=github)](https://github.com/TheMasiva/MasivaSkin/releases)
[![Auto Updates](https://img.shields.io/badge/Updates-Signed%20Auto%20Updates-39b87f?style=flat-square)](https://github.com/TheMasiva/MasivaSkin/releases)
[![Discord](https://img.shields.io/badge/Discord-Join%20Community-5865F2?style=flat-square&logo=discord&logoColor=white)](https://discord.gg/V8ZGrrVwym)
[![YouTube Teaser](https://img.shields.io/badge/YouTube-Watch%20Teaser-FF0033?style=flat-square&logo=youtube&logoColor=white)](https://www.youtube.com/watch?v=d6i8fmS4o8g)

**[📦 Windows Setup & Releases](https://github.com/TheMasiva/MasivaSkin/releases) · [🎬 Official Teaser](https://www.youtube.com/watch?v=d6i8fmS4o8g) · [💬 Discord Community](https://discord.gg/V8ZGrrVwym) · [🛠 Troubleshooting](#troubleshooting) · [🇹🇷 Türkçe](#turkce)**

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
**Languages / Diller:** English · Türkçe · Deutsch · Русский · Nederlands · Français · [Türkçe rehber](#turkce)

> **About updates:** Starting with the installed **v1.0.1** release, Masiva Skin supports signed update checks at launch. This README is the **permanent project guide**, not a changelog. For new features, compatibility updates, fixes, downloads and release-specific instructions, always read the **[GitHub Releases](https://github.com/TheMasiva/MasivaSkin/releases)** page.

# 🇬🇧 English

## What is Masiva Skin?

**Masiva Skin** is a Windows application for managing **local cosmetic selections in Counter-Strike 2**. It offers a visual interface for weapons, knives, gloves and agents, plus saved profiles and a menu that can be opened over the game in borderless/windowed display modes.

The project is designed around **client-side appearance customization**. It is **not** a Steam inventory editor, does not grant ownership of items and does not change Steam trading or marketplace records. Visual results depend on the supported game build and the selected item.

> **Please read before installing:** This is third-party game modification software. It is not approved by Valve; using it can violate game/platform rules or lead to account restrictions. **No VAC, Trusted Mode, Trust Factor or account-safety guarantee is provided.**

The **Kurumi Era** is the visual identity planned for the 1.X series: application icon, menu portrait, launcher and optional Discord Rich Presence artwork.

## Features

| Area | What it offers |
| --- | --- |
| **Weapons & finishes** | Browse supported weapon models and paint finishes using visual selection cards. |
| **Knives & gloves** | Select supported knife and glove appearances, with applicable finish options. |
| **Agents** | Configure supported agent appearances for T and CT individually. |
| **Team loadouts** | Maintain separate **T / CT** selections; use **Both** for items shared by both teams where available. |
| **Appearance details** | Customize applicable wear/float, paint seed, StatTrak and name tags; sticker and charm options depend on item/build support. |
| **Visual previews & Quick Inspect** | Browse offline preview images; enlarge an item image, zoom with the mouse wheel (50–250%), reset with `0`, and close with `Esc`. These are catalog images, not 3D inspection. |
| **Overlay menu** | Open or hide the external Windows menu with a configurable keyboard shortcut (**Insert** by default). |
| **Appearance settings** | Light/dark themes, custom accent color and six interface languages (Turkish, English, German, Russian, Dutch and French). |
| **Loadout / Quick Loadout** | Preview T/CT items side by side, choose and load profiles, or reapply your current selections from the integrated Loadout page. |
| **Profiles** | Save, preview and load `.masiva` files. Profile overwrites keep up to five rotating `.bak` backups with a restore action. |
| **Recent skins & filters** | Revisit the latest 20 distinct successfully submitted selections; remove individual active filters using their chips. |
| **Selected / Applied status** | See whether an item is merely selected in the editor or already part of the active loadout. “Applied” means loadout status, not independent confirmation of the in-game render. |
| **Live skin details** | See draft float/wear, paint seed, supported float range and finish/variant information. Agent details show model/arms instead of irrelevant float values. |
| **Search suggestions** | See up to six localized item/finish matches in the current category and team; use mouse or Down + Enter to select. |
| **Keyboard & notifications** | Arrow/Enter navigation, `Ctrl+F` to search, `Ctrl+I` to inspect, `Ctrl+Enter` to apply and contextual notifications for saves/applies/restores/errors. |
| **Compact pages & themes** | Loadout, Profiles, Recent Skins, Filters and customization tools open inside the main window. Choose preset or custom main/accent colours, opacity and menu size. |
| **Restore All** | Clear active cosmetic overrides across the current T/CT session; immediately restore available originals, while holstered/unavailable items may be restored when equipped again. Saved profile files and favorites are not deleted. |
| **Local team previews** | Where supported, the local player’s selected agent/gloves and the weapon held by the presentation scene may be reflected in team introductions or end-of-match presentations. Other players’ preview characters are not modified. |
| **Diagnostics** | Connection status, compatibility feedback, application logs and diagnostic report export, including available team-preview callback counters and timings. |
| **Windows installer** | Install with `MasivaSkinSetup.exe` into `%LOCALAPPDATA%\MasivaSkin`, with desktop/Start Menu shortcuts; no administrator privileges normally required. |
| **Signed automatic updates** | On launch, check the latest stable GitHub Release, verify the signed update metadata and the setup file hash, and offer a compatible update when CS2 is closed. |
| **Launcher settings** | Launch CS2 through Steam; configure General (hide on launch, optional Discord Rich Presence, language) and In Game Window (opacity, size, accent, theme). |

### Inspect, status & interface responsiveness

- Open **Quick Inspect** using the item card action, the main preview image, or `Ctrl+I`. Zoom with the wheel, press `0` to reset and `Esc` to close. It enlarges the **packaged 2D preview**; it does not model real-time wear, stickers or game lighting.
- **Selected** = currently edited choice. **Applied** = choice stored in the active team loadout (including loaded profiles); it may remain applied when another weapon is held. **This is not real-time CS2 renderer verification.**
- Search suggestions and keyboard controls are designed not to interrupt typing into input fields. Notifications indicate whether a request is queued/sent or saved, not that a VAC-secure match is guaranteed.
- Settings can optionally color the version text with the current accent color rather than the animated RGB effect. Short card hover transitions are retained while unnecessary full-window fades and flashes are removed.
- Connection/status feedback differentiates **waiting for CS2**, **connected**, and **unsupported game build**. If a game build is unsupported, do not force compatibility.

### Interface languages

Choose **Türkçe, English, Deutsch, Русский, Nederlands or Français** in **Settings → General → Language**. New translations include menu terms and a catalog mapping for **2,224 weapon/finish records** across the four additional languages. Some secondary UI text may still fall back to English. Language dropdown visibility on smaller screens has been improved. **In-game HUD auto-language detection continues to use its existing Turkish/English behavior**, separate from the menu language.

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

### Recommended: Windows Setup (v1.0.1 and later)

1. Visit **[GitHub Releases](https://github.com/TheMasiva/MasivaSkin/releases)**. Read the latest **stable** release notes, including its supported CS2 build and known limitations.
2. Under **Assets**, download **`MasivaSkinSetup.exe`**. Do **not** use GitHub's automatically generated *Source code* ZIP as an installer. Download only from this project's official release page.
3. **Close CS2 and any older Masiva Skin version**, then run the setup file normally. It installs for your Windows account; administrator elevation is not normally required.
4. Setup installs the application under `%LOCALAPPDATA%\MasivaSkin\App` and creates desktop and Start Menu shortcuts.
5. On the **first installation**, setup can offer to import settings and profiles from an older portable folder. Select that folder's `Masiva-Skin.ini` if you want to migrate; see the next section.
6. Open Masiva Skin using its shortcut. The launcher checks for a newer signed release at startup. You can use **Launch CS2** (through Steam, preserving your existing Steam launch options) or **Explore menu** to preview the interface without starting CS2.
7. For the in-game overlay, use **Fullscreen Windowed (Borderless)** or **Windowed** display mode. Select your team, item and finish; press **Apply** and save a profile if needed.

> **First-time migration:** Existing portable v1.0.0 and older installations do **not** acquire automatic updating on their own. Run the v1.0.1-or-newer Windows installer **once** to move to the installed update system. A separate legacy portable build, if distributed, must be updated manually according to its own release notes.

### Importing profiles and settings from a portable version

- When prompted on first install, select the old portable installation's **`Masiva-Skin.ini`**, not an individual `.masiva` profile.
- Setup copies available settings, favorites, recent selections and files in the old `profiles/` folder into the new `UserData` directory. Existing destination files are **not overwritten**; your old folder is **not deleted**.
- A `.masiva` profile stored elsewhere can be imported manually using **Profiles → Load profile**.
- Keep a separate backup of valuable profiles, settings and `.bak1`–`.bak5` profile history before migrating.

### Automatic updates: how they work

- On launch, the installed app checks the **latest stable GitHub Release** for `update.txt` and `update.sig`. The metadata is checked against the **public verification key built into the application** (RSA-3072 / SHA-256).
- If a newer version is available and **CS2 is closed**, it downloads the matching `MasivaSkinSetup.exe` and checks its **SHA256** against the signed update metadata before starting the installation.
- The old program closes; the installer waits for it to exit and updates the program files. **Personal settings and profiles remain in `UserData`**. CS2 must also remain closed while setup runs.
- If the update server is unavailable, verification fails or the download is interrupted, the current version remains in place. **Updates are checked on launch, not continuously in the background.** If CS2 is already running, the update is postponed until a later launch.
- This signing system validates the project's update metadata and downloaded installer. It is **not a Windows Authenticode certificate**, and it does **not** guarantee anti-cheat safety or eliminate SmartScreen warnings.

### Installation folders and uninstalling

| Location | Contents |
| --- | --- |
| `%LOCALAPPDATA%\MasivaSkin\App` | Installed application, matching Bridge DLL, packaged previews and resources. |
| `%LOCALAPPDATA%\MasivaSkin\UserData` | Your settings, profiles, favorites, recent selections and update log. |
| `%LOCALAPPDATA%\MasivaSkin\Updates` | Downloaded update installers. |
| `%LOCALAPPDATA%\MasivaSkin\Previous` | Previous installation backup used by the update process. |

To uninstall, use **Windows Settings → Apps → Installed apps → Masiva Skin → Uninstall** (wording varies by Windows version). The uninstaller deliberately **preserves `UserData`**. You may remove it manually only after backing up anything you want to keep. The maintenance executable may remain until manually cleaned up.

### Maintainers: publishing a signed update

For each **new stable** release (for example `v1.0.2`):

1. Update the version in the program, build files, installer resources and documentation; build and test the new Windows x64 application and **`MasivaSkinSetup.exe`**.
2. Sign the **final, unchanged installer** with the **same private signing key** used for v1.0.1. From the source directory, with Python and the `cryptography` package installed, for example:

   ```powershell
   python tools/sign_release.py 1.0.2 MasivaSkinSetup.exe update-signing-key.pem release-1.0.2
   ```

3. Push the corresponding source code, create the **`v1.0.2`** GitHub Release, mark it **Latest** (not draft/pre-release), and upload these **three separate assets with these exact names**: `MasivaSkinSetup.exe`, `update.txt`, `update.sig`. An additional `SHA256.txt` is optional for manual verification.
4. Do **not** alter the installer after signing it. Preserve the same signing key across updates; never publish the **private key**, its ZIP backup, or a PEM file. Only the public key belongs in the application/source.
5. Before announcing the release, test a real **installed v1.0.1 → v1.0.2** upgrade on Windows, including profile persistence and recovery from a failed update.

The updater expects GitHub assets at `releases/latest/download/update.txt`, `releases/latest/download/update.sig`, and `releases/download/vX.Y.Z/MasivaSkinSetup.exe`. Source code files under `src/`, `tools/` or `tests/` are useful for development but **are not downloaded or built by the updater**. The private signing key must never be committed, even temporarily.

## Quick usage reference

| Control / option | Action |
| --- | --- |
| **Insert** | Show/hide the menu (default hotkey). |
| **Ctrl+F** | Focus the skin search box. |
| **Ctrl+I** | Open Quick Inspect for the selected item. |
| **Ctrl+Enter** | Submit the active selection with Apply. |
| **Arrow keys / Enter** | Navigate eligible cards and open an item preview; input fields keep normal editing keys. |
| **0 / mouse wheel / Esc (Inspect)** | Reset zoom / zoom catalog preview / close inspection. |
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
| **Explore menu (launcher)** | Preview the menu without launching CS2. Legacy portable distributions may include `Explore-menu.cmd`. |
| **Launcher → General** | Hide on launch, optional Discord Rich Presence sharing, and application language. |
| **Launcher → In Game Window** | Set the in-game menu opacity, size, accent colour and theme. |
| **Language** | Choose Türkçe, English, Deutsch, Русский, Nederlands or Français in Settings → General → Language. |
| **Settings** | Theme, accent color, language, menu key, opacity, size and diagnostics. |
| **Log / diagnostics** | Inspect errors and save a diagnostic report. |
| **Hide / Exit** | Hide the overlay or close the application. |

**Inline navigation:** Loadout, Profiles, Recent Skins, Filters, Customize and Stickers/Charms open within the main window. Use the top navigation / Back control to return. The selected card border is separate from the successful submit checkmark; neither alone verifies the visible in-game result.

**Both mode:** Only supported items shared by both teams are eligible. Merely switching to Both does not apply changes; use **Apply** or save the profile. Changes to the other side may appear when its item is next equipped. Agents and team-exclusive weapons are configured separately.

In the installed edition, interface preferences (`Masiva-Skin.ini`) and `.masiva` profiles are stored under `%LOCALAPPDATA%\MasivaSkin\UserData`, separately from program files. Discord Rich Presence is optional and can be configured in Launcher → General. Windows native file/color pickers can still appear outside the main window.

## Known limitations

- **Quick Inspect uses offline catalog artwork** and is not a live 3D model viewer; selected float/stickers and game lighting might not match the image.
- **Applied denotes active loadout state**, not verified real-time rendering on every CS2 game build. No “Pending” indicator is promised.

> Static module validation and automated tests are not a substitute for live game testing. Consult each release's validation notes for what was actually tested.

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

### 7. The automatic update check fails or an update is postponed

- Make sure you are running the **installed v1.0.1-or-newer edition**, not an older portable build. Confirm that the latest stable release contains the exact assets `update.txt`, `update.sig` and `MasivaSkinSetup.exe`.
- Check that GitHub is reachable; an offline or failed update check should leave the installed version usable. Do not turn off Windows security protections to force an update.
- Fully close **CS2 and Masiva Skin**, then relaunch the app to retry. The updater does not install while CS2 is open.
- If a downloaded update fails signature or SHA256 checks, **do not bypass the verification**. Download the official setup manually from the release page and report the failure through Discord or GitHub Issues.
- If a setup fails, look at `%LOCALAPPDATA%\MasivaSkin\UserData\update.log` when available. The installer may keep a previous application copy, but a crash after the new app starts is **not** automatically detected by a health check.

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

The script configures an **x64 Release** build and runs the included tests; consult the release-specific build instructions for packaging the installed Setup executable. **Building is not the same as signing an update:** `tools/sign_release.py` requires the private signing key, which must remain outside the public repository. Passing automated tests does **not** guarantee live CS2 compatibility or an end-to-end installer/update test.

## Integrity, licenses & disclaimer

- For the installed edition, verify the **Setup EXE** against the release's published SHA256 (or its `SHA256.txt`) using PowerShell:

  ```powershell
  Get-FileHash .\MasivaSkinSetup.exe -Algorithm SHA256
  ```

  Compare the value with the checksum published for **that exact release**. A matching hash supports file integrity relative to that checksum; it is **not proof of malware safety, VAC compatibility or Valve approval**.
- **Source licensing:** The project's original source code is currently published **without a project license** (`No license`). Public repository visibility does not grant permission to copy, modify, redistribute or reuse it beyond rights provided by law or the platform's terms. A project license may be added later.
- Source attribution and notices for third-party dependencies remain in [`third_party/`](third_party/) or the release's `THIRD-PARTY-NOTICES/` folder. Those components retain their own licenses. Third-party metadata licenses do **not** transfer ownership of Valve or Workshop artwork.
- The application is independent of Valve; Valve game DLLs are **not included** in the distributed package.
- Use this software only where permitted by the game's terms and applicable rules. **No anti-cheat immunity, matchmaking access or account safety is guaranteed.**

**Kurumi artwork:** The Kurumi-themed image remains subject to its creators’ rights; do not assume the source repository or this README grants third parties permission to reuse it. Public distribution requires appropriate rights.

**Project:** [GitHub repository](https://github.com/TheMasiva/MasivaSkin) · **Updates:** [Releases](https://github.com/TheMasiva/MasivaSkin/releases) · **Community:** [Discord](https://discord.gg/V8ZGrrVwym) · **Videos:** [Official teaser](https://www.youtube.com/watch?v=d6i8fmS4o8g) / [Previous showcase](https://www.youtube.com/watch?v=JX4N9-Rs1TE)

---

<a id="turkce"></a>
# 🇹🇷 Türkçe

> **Güncellemeler hakkında:** Kurulu **v1.0.1** sürümünden itibaren Masiva Skin açılışta imzalı güncellemeleri kontrol edebilir. Bu README, her sürümde değiştirilmesi gerekmeyen **kalıcı kullanım kılavuzudur**. Yeni özellikler, hotfix'ler, desteklenen CS2 sürümleri ve indirmeler için **[GitHub Releases](https://github.com/TheMasiva/MasivaSkin/releases)** bölümüne bakın.

## Masiva Skin nedir?

**Masiva Skin**, Counter-Strike 2 için geliştirilen, **istemci tarafındaki kozmetik görünüm seçimlerini** yönetmeye odaklanan bir Windows uygulamasıdır. Silah, bıçak, eldiven ve ajan görünümleri için görsel seçim kartları, profil kaydetme/yükleme ve oyun üzerinde açılabilen harici bir menü sunar.

Amaç **yerel görünüm özelleştirmesidir**. Steam envanterine eşya eklemez, eşya sahipliği vermez; Steam Pazarı veya takas kayıtlarını değiştirmez. Görsel sonuçlar desteklenen oyun sürümüne ve eşyaya bağlıdır.

> **Kurulumdan önce:** Bu, Valve tarafından onaylanmayan üçüncü taraf bir oyun modifikasyonudur. Kullanımı oyun/platform kurallarını ihlal edebilir veya hesap kısıtlamalarına yol açabilir. **VAC, Trusted Mode, Trust Factor ya da hesap güvenliği garantisi yoktur.**

**Kurumi Dönemi**, 1.X serisinin planlanan ortak görsel kimliğidir: program simgesi, menü portresi, launcher ve isteğe bağlı Discord Rich Presence görseli.

## Özellikler

| Bölüm | Açıklama |
| --- | --- |
| **Silahlar ve kaplamalar** | Desteklenen silah ve skinleri görsel kartlarla seçme. |
| **Bıçaklar ve eldivenler** | Uyumlu bıçak, eldiven ve kaplama seçenekleri. |
| **Ajanlar** | T ve CT için ayrı ajan görünümü seçimleri. |
| **Takım ayarları** | Ayrı **T / CT** kayıtları; uygun ortak eşyalar için **Both**. |
| **Kozmetik ayrıntıları** | Desteklenen eşyalarda float/wear, seed, StatTrak, NameTag, sticker ve charm seçenekleri. |
| **Görsel önizleme / Quick Inspect** | ZIP içindeki katalog görsellerini büyütme, fareyle %50–250 yakınlaştırma, `0` ile sıfırlama ve `Esc` ile kapatma. Canlı 3D inceleme değildir. |
| **Harici menü** | Varsayılan **Insert** tuşuyla açılıp kapanan, tuşu değiştirilebilir Windows overlay. |
| **Görünüm ve dil** | Siyah/beyaz tema, özel vurgu rengi ve altı arayüz dili (Türkçe, İngilizce, Almanca, Rusça, Hollandaca, Fransızca). |
| **Loadout / Quick Loadout** | T/CT görsellerini yan yana inceleme; entegre Loadout sayfasından profil seçme/yükleme ve mevcut kombinasyonu yeniden uygulama. |
| **Profiller** | `.masiva` profillerini kaydetme, resimli önizleme ve yükleme. Profilin üzerine yazıldığında `.bak1`–`.bak5` yedekleri ve geri yükleme seçeneği. |
| **Son kullanılanlar / filtreler** | Başarıyla gönderilen son 20 farklı skin seçimi; aktif filtre etiketlerini ayrı ayrı kaldırma. |
| **Selected / Applied durumları** | Düzenleyicide yalnızca seçilen görünüm ile aktif Loadout’a kaydedilmiş seçim ayrı gösterilir. `Applied`, oyundaki görüntünün ayrıca doğrulandığı anlamına gelmez. |
| **Canlı skin ayrıntıları** | Seçilen float/wear, seed, desteklenen float aralığı, skin varyantı; ajanlarda gereksiz float yerine model/kol bilgisi. |
| **Arama önerileri** | Seçilen takım ve kategoriye göre en fazla altı yerelleştirilmiş silah/skin önerisi. |
| **Klavye ve bildirimler** | Ok/Enter gezinmesi, `Ctrl+F`, `Ctrl+I`, `Ctrl+Enter` kısayolları ve kaydetme/uygulama/geri alma/hata bildirimleri. |
| **Kompakt menü ve temalar** | Loadout, Profiles, Recent Skins, Filters ve özelleştirme sayfaları ana pencerenin içinde açılır. Hazır paletler veya ayrı ana zemin/vurgu renkleri, opaklık ve boyut ayarları. |
| **Restore All / Tümünü geri al** | Oturumdaki T/CT kozmetik değişikliklerini temizler; hazır özgün verileri geri yükler, elde olmayan eşyaların geri yüklenmesi yeniden kuşanmaya kalabilir. Kayıtlı profil ve favoriler silinmez. |
| **Yerel takım tanıtımı** | Desteklenen sahnelerde yalnızca yerel oyuncunun tanıtım karakteri için ajan, eldiven ve sahnenin tuttuğu silah görünümü kullanılabilir. Başka oyuncuların tanıtım modelleri değiştirilmez. |
| **Tanılama** | Bağlantı/uyumluluk durumları, loglar ve mevcutsa Team Preview sayaçları ile sürelerini içeren tanı raporu. |
| **Windows kurucusu** | `MasivaSkinSetup.exe` ile `%LOCALAPPDATA%\MasivaSkin` klasörüne kurulum, masaüstü/Başlat kısayolları; normalde yönetici yetkisi gerekmez. |
| **İmzalı otomatik güncelleme** | Açılışta GitHub’daki en son kararlı Release’i denetleme; güncelleme bilgisi imzasını ve kurucu SHA256 değerini doğrulama. CS2 açıksa kurulum ertelenir. |
| **Launcher ayarları** | Steam üzerinden CS2 başlatma; General (açılışta gizle, isteğe bağlı Discord Rich Presence, dil) ve In Game Window (opaklık, boyut, vurgu, tema) seçenekleri. |

### İnceleme, durumlar ve arayüz

- **Quick Inspect**, skin kartındaki inceleme düğmesi, büyük görsel veya `Ctrl+I` ile açılır. Fare tekerleğiyle yakınlaştır, `0` ile sıfırla, `Esc` ile kapat. Görsel **paket içindeki 2D katalog resmidir**; oyunun canlı 3D görüntüsü, float veya sticker simülasyonu değildir.
- **Selected** = şu an düzenlediğin seçim. **Applied** = profil dahil aktif takım Loadout’unda kayıtlı görünüm; elinde başka silah olsa da Applied kalabilir. **Oyundaki renderin doğrulandığı anlamına gelmez.**
- Arama önerileri mevcut takım/kategoriye uyum sağlar. Kısayollar, metin kutularında doğal yazma davranışını engellememelidir. Bildirimler işlemin gönderilmesini/kaydedilmesini gösterir; VAC güvenliği iddiası değildir.
- Sürüm yazısı isteğe bağlı olarak tema vurgu rengini kullanabilir; gereksiz tam pencere solması ve seçim flaşı kaldırılmış, kısa kart hover efekti korunmuştur.
- Durum çubuğu **CS2 bekleniyor**, **bağlandı** ve **desteklenmeyen oyun derlemesi** durumlarını ayırır. Desteklenmeyen sürümü zorlamayın.

### Arayüz dilleri

**Ayarlar → Genel → Dil** bölümünden **Türkçe, English, Deutsch, Русский, Nederlands veya Français** seçebilirsin. Eklenen dört dilde **2.224 silah/kaplama kaydı** için katalog adı eşleştirmeleri bulunur. Bazı ikincil menü metinleri İngilizceye dönebilir. Dil açılır listesinin küçük ekranlardaki görünürlüğü iyileştirildi. **Oyun içi HUD dil algılaması** ise arayüzden bağımsız olarak mevcut Türkçe/İngilizce davranışını kullanır.

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

### Önerilen yöntem: Windows Setup (v1.0.1 ve sonrası)

1. **[GitHub Releases](https://github.com/TheMasiva/MasivaSkin/releases)** sayfasını aç. En son **kararlı** sürümün notlarını, desteklediği CS2 sürümünü ve bilinen sınırlamaları oku.
2. **Assets** bölümünden **`MasivaSkinSetup.exe`** dosyasını indir. GitHub'ın otomatik sunduğu *Source code* ZIP'i kurulum dosyası değildir. Yalnızca projenin resmî Release sayfasındaki kurucuyu kullan.
3. **CS2'yi ve eski Masiva Skin'i tamamen kapat.** Kurucuyu normal şekilde aç; kendi Windows kullanıcı hesabın için kurulum yapar, normalde yönetici izni istemez.
4. Uygulama `%LOCALAPPDATA%\MasivaSkin\App` klasörüne kurulur. Masaüstü ve Başlat menüsü kısayolları oluşturulur.
5. **İlk kurulumda** eski taşınabilir sürümden ayar/profil aktarma seçeneği çıkabilir. İstersen eski klasörün `Masiva-Skin.ini` dosyasını seç; ayrıntılar aşağıda.
6. Kısayoldan uygulamayı aç. Launcher, açılışta imzalı yeni sürüm kontrolü yapar. **Launch CS2**, oyunu mevcut Steam başlatma seçeneklerini değiştirmeden açar; **Explore menu** ise CS2'yi başlatmadan arayüze bakmanı sağlar.
7. Oyun içi menü için **Pencereli Tam Ekran (Borderless)** veya **Pencereli** görüntü modunu kullan. Takım/eşya/skin seç, **Apply** ile uygula ve gerekiyorsa profilini kaydet.

> **Eski sürümlerden geçiş:** Taşınabilir v1.0.0 ve önceki sürümler kendiliğinden otomatik güncelleme kazanmaz. Kurulumlu sisteme geçmek için **v1.0.1 veya daha yeni `MasivaSkinSetup.exe` dosyasını bir kez çalıştırman gerekir**. Ayrı sunulan eski taşınabilir sürümler kendi Release notlarına göre elle güncellenir.

### Eski profilleri ve ayarları taşıma

- İlk kurulumdaki içe aktarma penceresinde tek tek `.masiva` dosyası yerine eski taşınabilir klasöründeki **`Masiva-Skin.ini`** dosyasını seç.
- Kurucu, mevcutsa ayarları, favorileri, son seçimleri ve eski `profiles/` klasöründeki dosyaları yeni `UserData` klasörüne **kopyalar**. Yeni klasörde zaten var olan dosyaların **üzerine yazmaz**, eski dosyaları **silmez**.
- Farklı yerde sakladığın `.masiva` profilini **Profiles → Load profile** üzerinden kendin yükleyebilirsin.
- Taşımadan önce önemli profilleri, ayarları ve `.bak1`–`.bak5` yedeklerini ayrı bir yere yedekle.

### Otomatik güncelleme nasıl çalışıyor?

- Program açıldığında GitHub'daki **en son kararlı Release** üzerinden `update.txt` ve `update.sig` okunur. Güncelleme bilgisinin imzası uygulamaya gömülü **açık doğrulama anahtarıyla** denetlenir (RSA-3072 / SHA-256).
- Daha yeni sürüm varsa ve **CS2 kapalıysa**, yeni `MasivaSkinSetup.exe` indirilir. Kurucu dosyasının **SHA256** değeri imzalı güncelleme bilgisindeki değerle karşılaştırılır.
- Eski program kapanır; kurucu kapanmasını bekleyip program dosyalarını günceller. **Ayar ve profiller `UserData` içinde korunur.** Kurulum sırasında CS2 de kapalı olmalıdır.
- GitHub'a ulaşılamazsa, imza geçersizse veya indirme yarıda kalırsa kurulu sürüm korunur. **Kontrol uygulama açılışında yapılır; arka planda sürekli tarama yapılmaz.** CS2 zaten açıksa yeni kurulum sonraki açılışa ertelenir.
- Bu dijital imza güncellemenin gerçekten proje anahtarıyla imzalandığını doğrular; **Windows kod imzalama sertifikası değildir**. SmartScreen uyarılarını ortadan kaldırmaz, VAC ya da hesap güvenliği garantisi vermez.

### Kurulum klasörleri ve kaldırma

| Konum | İçerik |
| --- | --- |
| `%LOCALAPPDATA%\MasivaSkin\App` | Program, aynı sürüme ait Bridge DLL, görseller ve diğer uygulama kaynakları. |
| `%LOCALAPPDATA%\MasivaSkin\UserData` | Kalıcı ayarlar, profiller, favoriler, son seçimler ve güncelleme günlüğü. |
| `%LOCALAPPDATA%\MasivaSkin\Updates` | İndirilen yeni kurulum dosyaları. |
| `%LOCALAPPDATA%\MasivaSkin\Previous` | Güncelleme sırasında saklanan önceki kurulum yedeği. |

Kaldırmak için **Windows Ayarları → Uygulamalar → Yüklü uygulamalar → Masiva Skin → Kaldır** yolunu kullan (Windows sürümüne göre isimler değişebilir). Kaldırıcı **`UserData` klasörünü bilerek silmez**. Önce verilerini yedekledikten sonra bu klasörü istersen elle silebilirsin. Küçük bakım programı da elle temizlenene kadar kalabilir.

### Geliştirici için: imzalı güncelleme yayımlamak

Her yeni **kararlı** sürümde (örneğin `v1.0.2`):

1. Program, kurulum dosyaları ve derleme ayarlarındaki sürüm numaralarını güncelle. Yeni Windows x64 programını ve **`MasivaSkinSetup.exe`** kurucusunu derle ve test et.
2. **Son hâline gelmiş, sonradan değiştirilmeyecek** kurucuyu v1.0.1'deki **aynı özel imzalama anahtarıyla** imzala. Kaynak klasöründe Python ve `cryptography` paketi kuruluysa örneğin:

   ```powershell
   python tools/sign_release.py 1.0.2 MasivaSkinSetup.exe update-signing-key.pem release-1.0.2
   ```

3. Güncel kaynak kodlarını GitHub'a gönder, **`v1.0.2`** Release'ini aç, **Latest** olarak yayımla (Draft/Pre-release değil). Release Assets kısmına isimleri değişmeden **üç ayrı dosya** yükle: `MasivaSkinSetup.exe`, `update.txt`, `update.sig`. Elle doğrulama için ayrıca `SHA256.txt` ekleyebilirsin.
4. İmzaladıktan sonra kurucuyu değiştirme! Her yeni sürümde **aynı özel anahtarı** koru; özel anahtarı, ZIP yedeğini veya PEM dosyasını **asla GitHub'a yükleme**. Açık doğrulama anahtarı programda bulunabilir.
5. Geniş kitleye duyurmadan önce gerçek Windows sisteminde kurulu **v1.0.1 → v1.0.2** güncellemesini, profil korunmasını ve hatalı güncellemeden dönüşü test et.

Updater `releases/latest/download/update.txt`, `releases/latest/download/update.sig` ve `releases/download/vX.Y.Z/MasivaSkinSetup.exe` adreslerini bekler. GitHub'daki `src/`, `tools/` ve `tests/` klasörleri geliştiriciler içindir; **otomatik güncelleme bunları indirip derlemez**. Özel anahtar hiçbir Git commit'ine girmemelidir.

## Hızlı kullanım tablosu

| Kontrol | Ne yapar? |
| --- | --- |
| **Insert** | Varsayılan menü açma/kapatma tuşu. |
| **Ctrl+F** | Skin arama alanına odaklanır. |
| **Ctrl+I** | Seçili eşyanın Quick Inspect önizlemesini açar. |
| **Ctrl+Enter** | Seçimi Apply ile gönderir. |
| **Yön tuşları / Enter** | Kartlarda gezinme ve önizleme; metin alanlarında doğal yazma davranışı korunur. |
| **0 / tekerlek / Esc (Inspect)** | Yakınlaştırmayı sıfırlama / yakınlaştırma / önizlemeyi kapatma. |
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
| **Explore menu (launcher)** | CS2 açmadan menüyü önizler. Eski taşınabilir dağıtımlarda `Explore-menu.cmd` bulunabilir. |
| **Launcher → General** | Açılışta gizleme, isteğe bağlı Discord Rich Presence paylaşımı ve uygulama dili. |
| **Launcher → In Game Window** | Oyun içi menü opaklığı, boyutu, vurgu rengi ve tema ayarları. |
| **Language** | Ayarlar → Genel → Dil: Türkçe, English, Deutsch, Русский, Nederlands veya Français. |
| **Ayarlar** | Tema, vurgu rengi, dil, menü tuşu, opaklık, boyut ve tanılama. |
| **Log / diagnostics** | Hata ayrıntıları ve tanı raporu. |
| **Hide / Exit** | Menüyü gizler / uygulamadan çıkar. |

**Menü içinde gezinme:** Loadout, Profiles, Recent Skins, Filters, Customize ve Stickers/Charms ana pencere içinde açılır. Geri / üst gezinme kontrolüyle dönülür. Seçili kartın çerçevesi ile başarılı gönderim işareti farklıdır; hiçbiri tek başına oyundaki görünümü doğrulamaz.

**Both hakkında:** Yalnızca iki takımın ortak kullanabildiği uyumlu eşyalar içindir. Both sekmesine geçmek tek başına ayar uygulamaz; **Apply** veya profil kaydı gerekir. Diğer taraftaki görünüm, eşya tekrar kuşanıldığında güncellenebilir. Ajanlar ve tek takıma özel silahlar ayrı ayarlanır.

Kurulumlu sürümde `Masiva-Skin.ini` ayarları ve `.masiva` profil dosyaları `%LOCALAPPDATA%\MasivaSkin\UserData` klasöründe, program dosyalarından ayrı tutulur. Discord Rich Presence isteğe bağlıdır ve Launcher → General üzerinden ayarlanabilir. Standart Windows dosya/renk seçicileri ana pencere dışında açılabilir.

## Bilinen sınırlamalar

- **Quick Inspect çevrimdışı katalog görseli kullanır**; oyun içi gerçek 3D görünüm, float/sticker ve aydınlatmayla birebir aynı olmayabilir.
- **Applied aktif Loadout durumudur**; her CS2 sürümünde görünümün gerçekten çizildiğine dair doğrulama değildir. `Pending` göstergesi vaat edilmez.

> Statik modül doğrulaması ve otomatik testler canlı oyun testinin yerine geçmez. Hangi kontrollerin gerçekten yapıldığını ilgili sürümün doğrulama notlarından öğren.

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

### 7. Otomatik güncelleme kontrolü hata veriyor veya erteleniyor

- **Kurulu v1.0.1 veya daha yeni sürümü** kullandığından emin ol. Eski taşınabilir sürümler otomatik güncellenmez. En son kararlı Release altında isimleri birebir aynı `update.txt`, `update.sig`, `MasivaSkinSetup.exe` dosyalarının olduğunu kontrol et.
- GitHub'a erişilebildiğinden emin ol. İnternet veya güncelleme kontrolü başarısız olursa mevcut program kullanılabilir kalmalıdır. Güncelleme zorlamak için Windows güvenlik korumalarını kapatma.
- **CS2'yi ve Masiva Skin'i tamamen kapatıp** programı yeniden aç. CS2 açıkken güncelleme kurulmaz.
- İmza veya SHA256 doğrulaması başarısızsa **kontrolü atlatmaya çalışma**. Resmî Release sayfasından yeni Setup'ı kendin indir ve hatayı Discord veya GitHub Issues üzerinden bildir.
- Kurulum hatasında varsa `%LOCALAPPDATA%\MasivaSkin\UserData\update.log` kaydına bak. Önceki program yedeği bulunabilir; ancak yeni program açıldıktan sonraki çökmeleri otomatik fark eden bir sağlık kontrolü henüz yoktur.

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

Betik **x64 Release** derlemesini hazırlar ve testleri çalıştırır; kurulum EXE'sinin paketlenmesi için ilgili sürümün derleme notlarına bak. **Derleme ile imzalı güncelleme yayımlamak aynı şey değildir:** `tools/sign_release.py`, genel depoda bulunmaması gereken özel imzalama anahtarıyla çalışır. Testlerin geçmesi gerçek CS2 uyumluluğunu veya uçtan uca kurulum/güncelleme testini garanti etmez.

## Dosya doğrulama, lisanslar ve sorumluluk reddi

- Kurulumlu sürümde **Setup EXE** dosyasının SHA256 değerini Release altında yayımlanan değerle (veya `SHA256.txt` ile) PowerShell üzerinden kontrol edebilirsin:

  ```powershell
  Get-FileHash .\MasivaSkinSetup.exe -Algorithm SHA256
  ```

  Sonucu **aynı sürümün** yayımlanmış SHA256 değeriyle karşılaştır. Hash eşleşmesi yalnızca dosya bütünlüğüne yardımcı olur; **zararlı yazılım bulunmadığını, VAC uyumluluğunu veya Valve onayını garanti etmez**.
- **Kaynak kod lisansı:** Projenin özgün kaynak kodu şu anda **proje lisansı olmadan** (`No license`) yayımlanmaktadır. Deponun herkese açık olması, kanunen veya platform şartları kapsamında zaten var olan haklar dışında kodu kopyalama, değiştirme, yeniden dağıtma ya da başka projelerde kullanma izni vermez. Daha sonra bir proje lisansı eklenebilir.
- Üçüncü taraf bağımlılıkların bildirimleri [`third_party/`](third_party/) veya Release içindeki `THIRD-PARTY-NOTICES/` klasöründedir. Bu bileşenler kendi lisanslarına tabidir. Metadata lisansları Valve/Workshop görsellerinin telif haklarını devretmez.
- Masiva Skin, Valve'dan bağımsızdır. Valve'ın oyun DLL dosyaları Release paketine **dahil edilmez**.
- Programı yalnızca oyun/platform kuralları izin veriyorsa kullan. **Anti-cheat bağışıklığı, eşleştirme erişimi veya hesap güvenliği garantisi verilmez.**

**Kurumi görseli:** Kurumi temalı görsel ilgili hak sahiplerinin haklarına tabidir; bu repo, üçüncü kişilere görseli yeniden kullanma izni vermez. Herkese açık dağıtım için gerekli kullanım hakları ayrıca değerlendirilmelidir.

**Proje:** [GitHub](https://github.com/TheMasiva/MasivaSkin) · **Güncellemeler:** [Releases](https://github.com/TheMasiva/MasivaSkin/releases) · **Topluluk:** [Discord](https://discord.gg/V8ZGrrVwym) · **Videolar:** [Yeni tanıtım](https://www.youtube.com/watch?v=d6i8fmS4o8g) / [Önceki tanıtım](https://www.youtube.com/watch?v=JX4N9-Rs1TE)

---

<div align="center">

**Made by Masiva 💜**

[💬 Discord Community](https://discord.gg/V8ZGrrVwym) · [🎬 Official Teaser](https://www.youtube.com/watch?v=d6i8fmS4o8g)

*Project information lives here. Version-specific changes live in [Releases](https://github.com/TheMasiva/MasivaSkin/releases).*

</div>