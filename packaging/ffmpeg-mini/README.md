# FFmpeg minimal build (ClipForge)

ClipForge ships a trimmed FFmpeg 7.1.1 (shared build, GPLv3) compiled with MSYS2 MinGW-w64.
It only contains the encoders/filters ClipForge uses (see `configure.sh`).

* Source: https://ffmpeg.org/releases/ffmpeg-7.1.1.tar.xz (unmodified)
* Linked libraries: x264 (GPLv2+), x265 (GPLv2+), libwebp (BSD-3-Clause), zlib, plus the MinGW-w64 runtime (libgcc, libstdc++, winpthread; GCC runtime exception / MIT-like).
  Packages used: mingw-w64-x86_64-x264 0.165, x265 4.3, libwebp 1.6.0 (sources via MSYS2 PKGBUILDs: https://github.com/msys2/MINGW-packages).
* Rebuild: in an MSYS2 MINGW64 shell run `configure.sh` then `/usr/bin/make -j8 && /usr/bin/make install`, copy `out/bin` and its DLL dependencies to `.deps/ffmpeg-mini/bin`.
* If ClipForge starts using a new encoder/filter, add it to the `--enable-encoder` / `--enable-filter` lists and rebuild.
