#!/usr/bin/env python3
"""Builds ClipForge-manifest.json for a release, from the release's own ZIP.

The manifest lists every file of the ZIP with its sha256 and where its compressed bytes sit inside the ZIP.
A client hashes its installed files and downloads only the byte ranges of the files that differ
(HTTP Range requests on the ZIP asset), so a release needs no extra upload besides this small JSON.
"""
import argparse, hashlib, json, os, struct, sys, zipfile


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--zip", required=True, help="the release ZIP (ClipForge-Windows-x64.zip)")
    ap.add_argument("--out", required=True, help="output manifest path")
    ap.add_argument("--tag", required=True)
    ap.add_argument("--installer", help="full installer, used by the client to decide delta vs full")
    a = ap.parse_args()

    files = []
    with zipfile.ZipFile(a.zip) as z, open(a.zip, "rb") as raw:
        for info in z.infolist():
            if info.is_dir():
                continue
            if info.compress_type not in (zipfile.ZIP_STORED, zipfile.ZIP_DEFLATED) or info.flag_bits & 1:
                sys.exit(f"{info.filename}: unsupported compression {info.compress_type} (need Deflate or Store)")
            # The data starts after the local header, whose name/extra lengths may differ from the central directory.
            raw.seek(info.header_offset)
            head = raw.read(30)
            if head[:4] != b"PK\x03\x04":
                sys.exit(f"{info.filename}: bad local header")
            name_len, extra_len = struct.unpack("<HH", head[26:30])
            data = z.read(info)
            files.append({
                "path": info.filename.replace("\\", "/"),
                "size": info.file_size,
                "sha256": hashlib.sha256(data).hexdigest(),
                "offset": info.header_offset + 30 + name_len + extra_len,
                "csize": info.compress_size,
                "method": info.compress_type,
            })
    manifest = {
        "version": a.tag,
        "archive": os.path.basename(a.zip),
        "archiveSize": os.path.getsize(a.zip),
        "installerSize": os.path.getsize(a.installer) if a.installer and os.path.exists(a.installer) else 0,
        "files": sorted(files, key=lambda f: f["offset"]),
    }
    with open(a.out, "w", encoding="utf-8") as f:
        json.dump(manifest, f, separators=(",", ":"))
    print(f"{len(files)} files, manifest {os.path.getsize(a.out) / 1024:.0f} KB -> {a.out}")


if __name__ == "__main__":
    sys.exit(main())
