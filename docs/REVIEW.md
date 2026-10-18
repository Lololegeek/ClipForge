# APEX examination

Direct review; no subagents were used.

Resolved findings:

* Replay filename collisions within one second: per-save unique suffix, verified with overlapping actual saves.
* OBS shutdown resource race: outputs → encoders → sources, then `obs_wait_for_destroy_queue` before shutdown; no remaining-source/double-destroy diagnostics in validated runs.
* Stale asynchronous encoder/output callbacks: generation checks prevent old output events from affecting a replacement pipeline.
* Wrong capture module capabilities: D3D now exists before win-capture loads; runtime logs confirm DXGI and WGC.
* Settings edited while metrics refresh: configuration/catalog signals are separate; automated QML check preserves an edited field across refreshes.
* Recursive per-game configurations: embedded profiles are omitted from individual profile snapshots.
* Large-folder import pressure: directory discovery waits when the bounded worker queue is full.
* Destructive edits/export: originals preserved; export rejects existing/source destinations and commits a temporary file only after success; deletion requires a gallery confirmation and uses the recycle bin.
* SQL/process invocation: bound SQL values, whitelist-only sort columns, numeric edit validation, QProcess argument arrays without shell interpolation.
* Portable runtime gap: Microsoft redistributable DLLs are now deployed alongside the executable.
* MSVC header dependency tracking: corrected localized include-prefix detection and verified nonzero Ninja header dependencies after a clean build.

Known product/release limitations are listed in FEATURES.md and THIRD_PARTY.md. This examination is not a repository-wide security certification or an anti-cheat compatibility certification.
