$ErrorActionPreference='Stop'
Set-Location (Split-Path $PSScriptRoot -Parent)
foreach($tool in @('cmake','ninja','python','curl.exe','tar.exe')) { if (!(Get-Command $tool -ErrorAction SilentlyContinue)) { throw "Missing tool: $tool" } }
New-Item -ItemType Directory -Force .deps | Out-Null
if (!(Test-Path .deps/Qt/6.8.3/msvc2022_64/bin/qmake.exe)) {
 python -m pip install aqtinstall==3.3.0
 python -m aqt install-qt windows desktop 6.8.3 win64_msvc2022_64 -O "$PWD/.deps/Qt" -m qtmultimedia qtshadertools
 if ($LASTEXITCODE) { throw 'Qt download failed.' }
}
if (!(Test-Path .deps/obs/bin/64bit/obs.dll)) {
 curl.exe -L --fail --retry 3 https://github.com/obsproject/obs-studio/releases/download/32.2.2/OBS-Studio-32.2.2-Windows-x64.zip -o .deps/obs.zip
 if ($LASTEXITCODE) { throw 'OBS download failed.' }
 if ((Get-FileHash .deps/obs.zip).Hash.ToLower() -ne '4d6e40e3ab155f56b30de517380566a206d74b63cdf5ad49aa596924768f97e1') { throw 'OBS SHA256 mismatch.' }
 Expand-Archive .deps/obs.zip .deps/obs -Force
}
if (!(Test-Path .deps/obs-source/obs-studio-32.2.2-sources/libobs/obs.h)) {
 curl.exe -L --fail --retry 3 https://github.com/obsproject/obs-studio/releases/download/32.2.2/OBS-Studio-32.2.2-Sources.tar.gz -o .deps/obs-source.tar.gz
 if ($LASTEXITCODE) { throw 'OBS source download failed.' }
 New-Item -ItemType Directory -Force .deps/obs-source | Out-Null
 tar.exe -xf .deps/obs-source.tar.gz -C .deps/obs-source
 if ($LASTEXITCODE) { throw 'OBS source extraction failed.' }
}
foreach($tool in @('ffmpeg','ffprobe')) { if (!(Get-Command $tool -ErrorAction SilentlyContinue)) { throw "Install a redistributable FFmpeg build containing $tool and run again." } }
Write-Host 'Dependencies ready. Build with scripts/build.ps1.'
