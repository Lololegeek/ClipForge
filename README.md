# ClipForge

Native, offline Windows replay capture. C++20, Qt Quick, libobs, FFmpeg and SQLite. GPL-3.0-or-later.

Download the [Windows x64 installer](https://github.com/Lololegeek/ClipForge/releases/latest/download/ClipForge-Setup-x64.exe) or the [portable ZIP](https://github.com/Lololegeek/ClipForge/releases/latest/download/ClipForge-Windows-x64.zip). The installer runs for the current user without administrator rights. See the [website](https://lololegeek.github.io/ClipForge/) for real application screenshots and release details.

Run `dist/ClipForge/ClipForge.exe`. Open **Paramètres**, select the screen/window, apply settings, then activate Instant Replay. Press **F8** (the configurable default) to save a clip of the last seconds. Closing the window keeps capture running in the tray by default; use the tray's Quit action to exit.

ClipForge vérifie les nouvelles versions sur GitHub (désactivable dans **Paramètres**) et, pour une installation via l'installateur, télécharge puis installe la mise à jour à la fermeture.

Une notification en jeu de 3 secondes confirme chaque clip sauvegardé (fenêtre ou plein écran sans bordure ; désactivable dans **Paramètres**). Les raccourcis sont protégés contre le spam : un clip à la fois, 1,5 s minimum entre deux demandes.

Use **Mes clips** to play a video and open the editor. Choose start/end points and click **Garder ce passage** to trim; **Ajouter** appends another passage. Export a new MP4. Originals are preserved. Replay files and editor exports are MP4; existing MKV recordings can still be imported.

## Build

Install Visual Studio with MSVC x64 and a Windows SDK, CMake, Ninja, Python (dependency download only), 7-Zip and FFmpeg/FFprobe. PowerShell 7:

```powershell
cd 'D:\Code 2\LoloClips'
./scripts/bootstrap.ps1
./scripts/build.ps1
./scripts/package.ps1
./scripts/test.ps1
```

Dependencies are isolated under `.deps`. Python is never used by the application. See [building](docs/BUILDING.md), [capture](docs/CAPTURE.md), [validation](docs/VALIDATION.md) and [third-party inventory](THIRD_PARTY.md) for supported scope, evidence and source archives.
