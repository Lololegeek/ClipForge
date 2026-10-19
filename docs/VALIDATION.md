# Validation

Tests executed on 18 October 2026, Windows 11 build 26300.9711 (26H2), Ryzen 7 8700F (8 cores / 16 logical processors), 32 GB RAM, Radeon RX 6600, driver 32.0.21045.5002. MSVC 19.51, Qt 6.8.3, OBS 32.2.2, FFmpeg CLI 7.1.1. These are desktop/window capture results, not game FPS impact measurements.

| Executed run | Settings | Results |
|---|---|---|
| DXGI endurance, 125 s | 1280×720, 30 FPS, AMF H.264, 5 Mbit/s, 60 s replay | 58.933 / 59.333 s overlapping replays; 0 rendering-lag frames; mean CPU 0.378% across all cores; peak working set 192.1 MB; render 0.320 ms mean; observed save times 309 / 322 ms |
| DXGI default-quality test, 90 s | 1920×1080, 60 FPS, AMF H.264, 16 Mbit/s, 60 s replay | 58.317 / 59.883 s replays; 0 rendering-lag frames; mean CPU 0.655%; peak working set 275.8 MB; render 0.352 ms; observed save times 325 / 334 ms |
| Live native window fixture, 32 s | WGC, 1280×720 output, 30 FPS, AMF, audio disabled | 13.967 / 14.433 s replays; real changing native window captured outside the visible desktop; 1 rendering-lag frame during concurrent tests; peak RAM 159.4 MB |
| Software encoding, 32 s | DXGI, 1280×720, 30 FPS, x264 | Two valid replays and MKV recording; mean CPU 4.269%; peak RAM 420.8 MB; 0 rendering-lag frames; save times 1341 / 1362 ms |
| Final capture smoke, 32 s | DXGI, 1280×720, 30 FPS, AMF | Two distinct valid overlapping files plus manual recording; 0 rendering-lag frames; clean OBS shutdown |

All four capture configurations were checked using FFprobe packet/stream metadata and full FFmpeg decoding. Checks include container, duration, dimensions, FPS, AAC track, finite timestamps, monotonic per-stream DTS, and track-end timestamp skew below 250 ms. The 1080p60 replay track-end skews were 12.7 and 55.3 ms. This measures timestamps, not human-perceived lip sync. Normal stop left no OBS source leak/double-destroy diagnostic or orphan export temporary file.

Core QtTest: **7 passed, 0 failed**. Settings rejection/atomic persistence, project roundtrip and invalid edits, export graph speed/audio handling, SQLite favorites/search/tags, restart persistence and missing-file deletion were executed.

Media QtTest: **7 passed, 0 failed**. Captured-file validation/full decode, QMediaPlayer video frames and seeking, actual audio sink sample processing (muted during the test), AAC sample decoding, real registered global F22 input delivery, hotkey conflict detection, and a multi-segment speed-adjusted MP4 export were executed. The playback check also switches 2× then 4×, verifies advancing video and continued audio samples, and finishes without a playback error. On the Logitech PRO X headset, stereo float PCM is supported; only playback start and explicit seek opened the sink, while speed changes reused it. Expected export duration 3 s, checked within 200 ms; existing-file overwrite rejected. F22 is used for the automated check to avoid consuming the user's normal F8 workflow.

Qt's default audio renderer encountered the headset's virtual surround channel map. The final playback path explicitly decodes to a supported output format and feeds QAudioSink from a paced FFmpeg process; it prefers stereo when supported, and its test checks actual consumed samples. The optional WMF backend was evaluated and failed video delivery on this host; it is not selected or advertised.

Six QML pages were rendered and visually inspected in background software/offscreen mode. The library was populated with actual recorded-file metadata. An automated edit-persistence check verifies that metrics refresh does not reset a typed setting. A separate native Windows/D3D editor check ran in a window positioned outside the visible desktop: the clip advanced to 5 seconds, QVideoSink had a valid frame, and `editor-native/editor-playing.png` visibly shows the actual recorded video inside the editor. Human listening, prolonged editing A/V sync and accessibility automation remain to be checked.

Evidence remains local under `artifacts/`: `dxgi-endurance/validation.json`, `1080p60/validation.json`, `wgc-verified/validation.json`, `software/validation.json`, `release-smoke/validation.json`, QtTest text reports, capture logs and sampled metrics. Captured desktop content is intentionally excluded from distributable/source archives.

Reproduce:

```powershell
./scripts/bootstrap.ps1
./scripts/build.ps1 -Fresh
./scripts/package.ps1
./scripts/test.ps1
./scripts/test.ps1 -Capture -Seconds 125
./scripts/test.ps1 -Capture -FullHD -Seconds 90
./scripts/test.ps1 -WindowCapture
./scripts/test.ps1 -Capture -Software
```

Not executed: real-game hooking or FPS impact, anti-cheat game matrix, enabled microphone capture, process-isolated audio capture, Windows 10, clean VM, NVIDIA/Intel hardware, HEVC/AV1 capture, 120/144 FPS, 4K, driver reset/device loss, disk-full fault injection, or multi-hour endurance. These remain release validation work; no result is inferred from source code alone.

Packaging check: portable ZIP and Inno Setup x64 installer generated. Silent per-user installation to an isolated path containing an accented character completed with exit 0. The installed executable initialized its bundled OBS/Qt runtime and played an actual clip in the QML editor with a valid video frame and advancing position (exit 0). Silent uninstall completed with exit 0, no restart, and the isolated installation directory disappeared. Logs: `artifacts/setup-install.log`, `artifacts/installed-runtime/clipforge.log`, `artifacts/setup-uninstall.log`. No normal user clip directory was removed. CTest also passes with DLL/plugin paths supplied by the project configuration. Source/archive hashes and archive-integrity results are emitted alongside the deliverables.

## Release 0.1.0 checks, 19 October 2026

MSVC release build succeeded. Core QtTest: 7 passed; media QtTest: 7 passed. A 32-second WGC fixture run produced two replays and an MKV recording; all files passed full decode, timing, dimensions, FPS and AAC checks. This validates files and playback, not visual fidelity of an off-desktop GDI capture fixture or real-game behavior.

The regenerated Inno Setup installer installed silently to an isolated path containing an accented character, without administrator rights. The installed app initialized its bundled dependencies and passed the editor test (advancing playback and a valid QVideoSink frame). Silent uninstall returned 0, required no reboot and removed the test installation directory. Test data used an isolated directory. Installer version: 0.1.0.

The website passed TypeScript, ESLint and the production static build. Browser checks verified the screenshot tabs, a loaded real editor image, the GitHub installer link and no horizontal overflow at desktop and 390-pixel mobile widths. Screenshot provenance and media attribution are in SCREENSHOTS.md. Windows 10, a clean VM and real-game compatibility remain untested.

## Windows icon fix, 0.1.1

Fixed the Qt resource filename used by QIcon (`:/assets/ClipForge.ico`) and explicitly assigned it to the QQuickWindow. The process and installed shortcuts share the ClipForge.Desktop AppUserModelID. Start menu, desktop and startup shortcuts use the installed ClipForge.ico file. CMake now tracks the ICO file as a dependency of the Windows resource so branding changes rebuild the executable's icon.

Verified the installed executable contains seven native icon sizes. Both ExtractAssociatedIcon (Windows shell) and WM_GETICON on the actual running installed QQuickWindow return the website logo. Captured evidence remains local in artifacts/live-window-icon.png and artifacts/fixed-shell-icon.png. The installed Start menu shortcut was updated and Windows received a shell-change notification.

## Release 0.2.0 checks, 19 October 2026

MSVC release build and version resources: 0.2.0. Core QtTest: 7 passed, 0 failed. Media QtTest: 7 passed, 0 failed, including playback frames, audio processing, seeking/rate changes, hotkeys and an actual multi-segment export.

A 32-second isolated WGC fixture run produced two H.264/AAC MP4 replays and one MKV recording. Three save requests include a rapid 200 ms repeat: that repeat is suppressed, while a later request after the 1.5-second cooldown succeeds. Full decoding, timestamps, dimensions and FPS passed; rendering lag: 0 frames in this run. This is file/pipeline validation, not proof of real-game capture fidelity or exclusive-fullscreen overlay behavior.

The six-page QML smoke test passed with edited settings surviving refresh. The native editor check passed with an advancing playback position and a valid QVideoSink frame. Five public website screenshots were refreshed from the running 0.2.0 application, using the same credited imported sample video and isolated test data. The games-page capture stays private.

The capture harness now waits for the hotkey cooldown before its second accepted save; its validator requires exactly two replays. The PowerShell media-report argument was corrected to pass one output filename/format argument to QtTest. No changes were made to the installed user's application, settings or clips for these checks. Runtime update installation, startup-registry toggling, real games, exclusive fullscreen, a clean VM and Windows 10 were not exercised for this release.

Release artifacts built: installer 185114161 bytes, portable ZIP 252874214 bytes. Installer version resources are 0.2.0. The portable ZIP passed 7-Zip integrity checking and excludes development executables and the obsolete ClipForge-Fixed.exe. The regenerated installer was not installed over the user's active app during publication.
