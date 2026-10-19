# Third-party distribution inventory

ClipForge source is GPL-3.0-or-later. libobs is GPL-2.0-or-later, compatible with GPLv3. Qt is dynamically linked under its applicable open-source terms. No Medal code or artwork is used.

* OBS Studio 32.2.2 runtime and matching source: https://github.com/obsproject/obs-studio/releases/tag/32.2.2 . Source archive retained under `.deps/obs-source.tar.gz`; OBS license copied into the portable folder. Runtime contains additional FFmpeg, x264, GPU and other dependencies.
* Qt 6.8.3 MSVC2022 open-source binaries from the Qt online archive through aqtinstall 3.3.0. Corresponding sources: https://download.qt.io/archive/qt/6.8/6.8.3/single/ . Qt modules used: Core, Gui, Widgets, Qml, Quick, QuickControls2, Multimedia, Sql, Svg, ShaderTools and their runtime dependencies. SQLite is used through the Qt driver.
* FFmpeg 7.1.1 full build by Gyan.dev, GPLv3 enabled. Local executable version/configuration is captured in validation artifacts. Source: https://ffmpeg.org/releases/ffmpeg-7.1.1.tar.xz . Build provider: https://www.gyan.dev/ffmpeg/builds/ . The full build includes external libraries with their own notices and corresponding-source obligations.
* MSVC runtime is deployed by windeployqt, subject to Microsoft's redistribution terms.

Release source assets include the OBS 32.2.2 source archive, Qt 6.8.3 complete source archive, FFmpeg 7.1.1 source archive and OBS dependency build recipes (2026-07-15, as specified by OBS's CMake presets). Qt license texts and shipped-module SPDX inventories, OBS's license and FFmpeg license texts/configuration are included in `licenses/` in the installed and portable application. SHA256SUMS.txt identifies the release files.

These archives document the direct components; a full audit of every transitive library in the upstream prebuilt OBS and Gyan FFmpeg distributions has not been completed. No claim of a completed third-party compliance audit is made.
