# Building and deployment

Windows x64, MSVC 2022-compatible tools or newer, Windows SDK 10.0.22621 or newer. The tested host uses Visual Studio 2026 MSVC 19.51. Qt 6.8.3 is deliberately used instead of the host's incompatible MinGW Qt installation.

`bootstrap.ps1` downloads the official Qt MSVC modules and official OBS 32.2.2 portable distribution/sources. The OBS binary SHA256 is verified. `build.ps1` imports the VS developer environment, generates an import library from the exported symbols of the official obs.dll, configures CMake and builds with Ninja. The public OBS headers are from the exact matching source release; no invented declarations are used.

`package.ps1` deploys Qt QML/plugins, OBS multimedia modules, capture helpers, D3D runtime files and FFmpeg tools into `dist/ClipForge`. `qt.conf` resolves bundled plugins. OBS paths are resolved relative to the application executable, not the launch working directory. The replay mux helper runs from the bundled OBS binary directory.

The package includes app-local MSVC runtime DLLs. `package.ps1` also builds `dist/ClipForge-Setup-x64.exe` when Inno Setup 6 is available. It installs per-user and offers optional desktop/startup shortcuts, both unchecked by default. No account, admin rights or online activation is required.

The portable folder is self-contained for the tested host; a fresh Windows VM and Windows 10 remain required release checks. `build.ps1 -Fresh` performs a clean rebuild. French-only MSVC language kits need the corrected UTF-8 showIncludes prefix in CMakeLists.txt so Ninja records header dependencies; this is verified with `ninja -C build -t deps`.
