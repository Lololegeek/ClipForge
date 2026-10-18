# Capture behavior and limitations

Real libobs replay buffer, hardware encoder selection, compressed video/audio packet retention, keyframe-aware muxing. Defaults: H.264, 1080p60, 60 seconds, 16 Mbit/s, 512 MB maximum. The RAM limit may shorten the retained duration at high bitrates; libobs retains GOP boundaries and may exceed the nominal limit by GOP overhead. The displayed buffer duration is elapsed capture time capped at the requested duration, not a direct measurement of buffered packets.

Up to eight pending save requests are serialized around libobs's saved signal. Buffering continues and overlapping clips are supported. A queued request snapshots at its actual processing time, not exactly the keypress timestamp; no stronger guarantee is made. A 30-second timeout reports a stalled mux. Stopping is rejected while saving.

Screen uses DXGI; window uses Windows Graphics Capture through OBS, or explicitly reports the BitBlt fallback when WGC is unavailable. D3D is initialized before loading win-capture so it registers the actual DXGI/WGC sources. Game capture uses the standard OBS game capture source, with its compatibility option enabled. There is no attempt to bypass anti-cheat. Window/game failures do not silently record the desktop; choose another source explicitly. Runtime automatic fallback across source types and GPU-device-loss restart are not yet implemented. OBS's optional online compatibility-file updater is not started; the bundled compatibility data is used offline.

Audio offers system WASAPI, application-only WASAPI (when supported) and an optional selected microphone. One mixed AAC track. Process audio requires a selected window and OS support. Microphone is off by default. Separate audio tracks, live level meters and live device switching are not implemented. Capture source audio clocks and A/V mux timestamps are managed by libobs.

H.264 tries AMF, NVENC, QSV then x264. HEVC and AV1 require a compatible available hardware encoder; failure is shown and H.264 can be selected explicitly. 120/144 FPS, 4K and other GPUs require actual hardware validation.

Saved continuous recordings use MKV to improve recovery after unexpected interruption. Replays use MP4. F10 captures the primary desktop screen via Qt; it is not a game framebuffer screenshot. Temporary on-disk replay buffering, overlay, startup registration, automatic clip deletion and universal kill detection are unavailable in this version.
