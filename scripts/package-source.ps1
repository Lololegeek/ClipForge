$ErrorActionPreference='Stop'
Set-Location (Split-Path $PSScriptRoot -Parent)
New-Item -ItemType Directory -Force dist | Out-Null
$zip=if(Test-Path 'C:\Program Files\7-Zip\7z.exe'){'C:\Program Files\7-Zip\7z.exe'}else{(Get-Command 7z.exe).Source}
# Explicit allowlist: captured media, databases, personal settings and dependency caches stay local.
& $zip a -tzip -mx=5 -mmt=2 "$PWD/dist/ClipForge-Source.zip" CMakeLists.txt CMakePresets.json README.md LICENSE THIRD_PARTY.md PRODUCT.md DESIGN.md .gitignore .clang-format .github src ui assets docs scripts tests packaging site '-xr!node_modules' '-xr!.next' '-xr!out' '-xr!*.tsbuildinfo' | Out-Null
if($LASTEXITCODE){throw 'Source archive creation failed.'}
