# Performance

The UI reports libobs active FPS, total and lagged frames, mean render time, last replay save duration, process working set and process CPU divided across all logical processors. No GPU metric is fabricated. Buffer RAM usage is not directly exposed by the current adapter.

For repeatable impact measurements, record a fixed game benchmark without ClipForge, then repeat with identical driver/resolution/framecap and replay settings. Run at least three pairs and report mean/P95/P99 frame times, capture lag and resource use. Test a 60-second buffer after warmup and several overlapping F8 saves. A desktop capture check is not evidence of game FPS impact.

Capture subprocesses and FFmpeg exports run in the background. Export is limited to four encoder threads; decoder/filter threading still depends on FFmpeg. Avoid exporting during competitive play until impact is measured on your hardware.
