"""Test tooling only: inspect actual OBS files, never generate capture substitutes."""
import argparse
import json
import math
import pathlib
import subprocess
import statistics

parser = argparse.ArgumentParser()
parser.add_argument("folder", type=pathlib.Path)
parser.add_argument("--seconds", type=float, default=15)
parser.add_argument("--width", type=int, default=1280)
parser.add_argument("--height", type=int, default=720)
parser.add_argument("--fps", type=int, default=30)
parser.add_argument("--tools", type=pathlib.Path, default=pathlib.Path("dist/ClipForge/tools"))
args = parser.parse_args()
report = {"files": [], "checks": []}
files = sorted((args.folder / "clips").glob("*"))
assert len(files) >= 3, f"Expected two replays and one recording, got {len(files)}"
assert sum(p.name.startswith("Replay") for p in files) == 2, "Expected two replays after three requests (one rapid repeat suppressed)"
assert len({p.name for p in files}) == len(files)
for path in files:
    data = json.loads(subprocess.check_output([
        str(args.tools / "ffprobe.exe"), "-v", "error", "-show_format", "-show_streams",
        "-show_packets", "-of", "json", str(path)], timeout=30))
    duration = float(data["format"]["duration"])
    video = next(s for s in data["streams"] if s["codec_type"] == "video")
    audio = [s for s in data["streams"] if s["codec_type"] == "audio"]
    assert (video["width"], video["height"]) == (args.width, args.height)
    assert video["r_frame_rate"] == f"{args.fps}/1"
    assert audio, "AAC track absent"
    if path.name.startswith("Replay"):
        assert args.seconds - 3 <= duration <= args.seconds + 1, (path, duration)
        assert "mp4" in data["format"]["format_name"]
    else:
        assert 3 <= duration <= 7, (path, duration)
    last_dts = {}
    ends = {}
    for packet in data["packets"]:
        index = packet["stream_index"]
        if "dts_time" in packet:
            dts = float(packet["dts_time"])
            assert math.isfinite(dts)
            assert dts >= last_dts.get(index, -math.inf), "Non-monotonic DTS"
            last_dts[index] = dts
        if "pts_time" in packet:
            pts = float(packet["pts_time"])
            assert math.isfinite(pts)
            ends[index] = max(ends.get(index, -math.inf), pts + float(packet.get("duration_time", 0)))
    drift = max(ends.values()) - min(ends.values())
    assert drift < 0.25, f"Track duration skew {drift}"
    decode = subprocess.run([str(args.tools / "ffmpeg.exe"), "-v", "error", "-nostdin",
                             "-threads", "2", "-i", str(path), "-f", "null", "-"],
                            capture_output=True, timeout=45)
    assert decode.returncode == 0 and not decode.stderr, decode.stderr.decode(errors="replace")
    report["files"].append({"name": path.name, "bytes": path.stat().st_size,
                            "duration": duration, "video": video["codec_name"],
                            "audio": audio[0]["codec_name"], "track_end_skew_seconds": drift,
                            "decode": "passed", "timestamps": "passed"})
samples_path = args.folder / "samples.json"
if samples_path.exists():
    samples = json.loads(samples_path.read_text())
    active = [s for s in samples if s.get("sourceWidth", 0) > 0 and s.get("bufferSeconds", 0) > 0]
    assert active, "No active source dimensions were measured"
    report["performance"] = {
        "active_samples": len(active), "mean_cpu_percent_all_cores": statistics.mean(s.get("cpu", 0) for s in active),
        "peak_working_set_mb": max(s["ramMB"] for s in samples),
        "mean_fps": statistics.mean(s["fps"] for s in active),
        "lagged_frames": max(s["lagged"] for s in samples),
        "peak_buffer_coverage_seconds": max(s.get("bufferSeconds", 0) for s in samples),
        "mean_render_ms": statistics.mean(s["frameMs"] for s in active)}
log = (args.folder / "clipforge.log").read_text(encoding="utf-8")
assert "Double destroy" not in log and "source(s) were remaining" not in log
assert "Freeing OBS context data" in log
assert "La source n'envoie pas" not in log, "Capture source had no frames"
assert not list(args.folder.rglob(".clipforge-*.mp4")), "Orphan export temporary file"
report["checks"] = ["two distinct overlapping replays", "manual recording", "full decode",
                    "resolution and FPS", "AAC track", "monotonic DTS", "track-end timestamp skew",
                    "clean OBS shutdown", "no orphan export temporary files"]
(args.folder / "validation.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
print(json.dumps(report, indent=2))
