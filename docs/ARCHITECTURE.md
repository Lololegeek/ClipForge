# Architecture

`ObsEngine` owns libobs resources exclusively on the Capture OBS QThread. OBS supplies its own real-time graphics/audio/encoding and mux threads. QML only sends queued commands. Shared ownership is not used for OBS resources; the engine releases outputs before encoders, scenes and sources, then shuts down libobs. Startup and teardown never unload the engine while its worker runs.

Capture states: Idle → Initializing → Buffering; Buffering/Recording → Saving → Buffering/Recording; active states → Stopping → Idle. Errors stop or reject the operation. Saving is a presentation state over a continuously running replay output; it is not an interruption of capture. Continuous recording shares encoders with replay.

`MediaJobs` runs one background FFprobe/thumbnail process at a time with a 256-job queue and timeouts, and an independent FFmpeg export process. Process arguments are passed as arrays, never shell commands. Export uses a uniquely named temporary MP4 next to the destination and renames only on success. Existing files cannot be overwritten by export.

`ClipModel` uses SQLite WAL, indexed ordering and batches of 100 model rows. Video files remain ordinary disk files. Missing files are displayed as missing; they are not automatically deleted. Explicit refresh discovers files in the configured folder. Search and sorting execute in SQLite. Disk/SQL queries currently run on the GUI thread; very large or remote libraries need an asynchronous database worker.

`Settings` validates and atomically persists JSON. `EditProject` persists non-destructive edit decisions; undo/redo history is bounded to 100 states. Global hotkeys use RegisterHotKey; conflicts are reported. Profiles are opt-in per executable to avoid guessing that every process is a game.
