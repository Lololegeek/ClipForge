param([switch]$ConfigureOnly,[switch]$Fresh)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$vswhere="${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (!(Test-Path $vswhere)) { throw 'Visual Studio with Desktop development with C++ is required.' }
$vs=& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (!$vs) { throw 'MSVC x64 tools are missing.' }
$dev=Join-Path $vs 'Common7\Tools\Launch-VsDevShell.ps1'
& $dev -Arch amd64 -HostArch amd64 -SkipAutomaticLocation | Out-Null
Set-Location $root
# Force a stable /showIncludes prefix so Ninja tracks header dependencies on French Windows.
$env:VSLANG='1033'
if (!(Test-Path .deps/Qt/6.8.3/msvc2022_64/lib/cmake/Qt6/Qt6Config.cmake)) { throw 'Run scripts/bootstrap.ps1 first (Qt MSVC missing).' }
if (!(Test-Path .deps/obs/obs.lib)) {
 $exports=& dumpbin /nologo /exports .deps/obs/bin/64bit/obs.dll
 $symbols=$exports | ForEach-Object { if ($_ -match '^\s+\d+\s+[0-9A-F]+\s+[0-9A-F]+\s+(\S+)') {$Matches[1]} }
 @('LIBRARY obs','EXPORTS')+$symbols | Set-Content .deps/obs/obs.def -Encoding ascii
 & lib /nologo /def:.deps/obs/obs.def /machine:x64 /out:.deps/obs/obs.lib
 if ($LASTEXITCODE) { throw 'OBS import library generation failed.' }
}
if($Fresh){& cmake --fresh --preset windows-release}else{& cmake --preset windows-release}
if ($LASTEXITCODE) { throw 'CMake configuration failed.' }
if (!$ConfigureOnly) { if($Fresh){& cmake --build build --target clean}; & cmake --build --preset release --parallel 6; if ($LASTEXITCODE) { throw 'Build failed.' } }
