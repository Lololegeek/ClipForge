# Scope of 0.1

| Area | Operational implementation | Limits / remaining work |
|---|---|---|
| Replay | libobs compressed RAM buffer, configurable duration/memory, serialized overlapping saves, unique filenames | Saves queued during a mux snapshot later than the original keypress; no disk-backed replay |
| Capture | DXGI display, WGC window, standard OBS game source, manual MKV recording, MP4 replay | Actual game/anti-cheat checks, device-loss recovery, seamless source-type fallback remain |
| Encoding | AMF/NVENC/QSV discovery and selection, H.264 x264 fallback, hardware HEVC/AV1 selection | Only AMF H.264 and x264 were exercised on this machine; other codecs/GPUs untested |
| Audio capture | WASAPI system/process source, selected microphone, per-source gain/mute through zero volume | One mixed AAC track; no separate tracks/meters/live device changes. Process isolation and microphone enabled require additional runtime tests |
| Library | SQLite paging, thumbnails, search/sort, favorites/tags/rename/trash, import, missing-file handling | External changes need Refresh; no relocation assistant. Remote/very large SQL workloads are not asynchronous yet |
| Editor | Native video, decoded audio output, scrub/range trim, multiple passages from one source, removal, speed, gain, undo/redo, saved projects, H.264/HEVC MP4 exports | All exports reencode; no lossless keyframe copy, multi-file assembly, vertical crop, text, subtitles or transitions |
| Background | Tray menu, global configurable F1–F24 keys with conflict errors, notifications, optional close-to-tray | Modifier chords and overlay remain; F10 captures primary desktop, not the game framebuffer |
| Games | Enumerate windows/executables, opt-in profiles, auto-start for known foreground profiles, stop after their windows disappear | Profile/game behavior has no real-game runtime verification yet. Unknown processes never automatically become games |
| Settings | Validated atomic JSON, folder selection, config import/export, start minimized, installer startup shortcut | No auto clip deletion, storage cleanup rules, disk quota, temporary-buffer folder or auto-clipping plugins |

Playback audio uses FFmpeg PCM decoding in a background subprocess and an explicitly described QAudioSink format to support virtual-surround headsets. It prefers stereo float PCM when the output supports it, reducing needless multichannel decode work. Pause and seek recreate the audio output; rate changes are coalesced for 120 ms and restart only the decoder while reusing that output. Its pending PCM queue is bounded to one second. Video uses Qt Multimedia. File export A/V timestamps are independently checked; preview synchronization has not been assessed by human listening.
