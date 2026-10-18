# Third-party distribution inventory

ClipForge source is GPL-3.0-or-later. libobs is GPL-2.0-or-later, compatible with GPLv3. Qt is dynamically linked under its applicable open-source terms. No Medal code or artwork is used.

* OBS Studio 32.2.2 runtime and matching source: https://github.com/obsproject/obs-studio/releases/tag/32.2.2 . Source archive retained under `.deps/obs-source.tar.gz`; OBS license copied into the portable folder. Runtime contains additional FFmpeg, x264, GPU and other dependencies.
* Qt 6.8.3 MSVC2022 open-source binaries from the Qt online archive through aqtinstall 3.3.0. Corresponding sources: https://download.qt.io/archive/qt/6.8/6.8.3/single/ . Qt modules used: Core, Gui, Widgets, Qml, Quick, QuickControls2, Multimedia, Sql, Svg, ShaderTools and their runtime dependencies. SQLite is used through the Qt driver.
* FFmpeg 7.1.1 full build by Gyan.dev, GPLv3 enabled. Local executable version/configuration is captured in validation artifacts. Source: https://ffmpeg.org/releases/ffmpeg-7.1.1.tar.xz . Build provider: https://www.gyan.dev/ffmpeg/builds/ . The full build includes external libraries with their own notices and corresponding-source obligations.
* MSVC runtime is deployed by windeployqt, subject to Microsoft's redistribution terms.

The local portable output is a development deliverable. Before publishing binaries to third parties, collect and provide the complete corresponding sources/build scripts and license notices for the exact FFmpeg/OBS transitive binaries and Qt modules, or rebuild from a fully archived dependency manifest. Upstream links alone do not replace GPL corresponding-source obligations. A public distribution compliance audit has not been completed.
