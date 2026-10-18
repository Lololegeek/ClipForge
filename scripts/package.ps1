param([switch]$SkipZip)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
Set-Location $root
$out=Join-Path $root 'dist/ClipForge'
New-Item -ItemType Directory -Force $out,"$out/obs/bin/64bit","$out/obs/obs-plugins/64bit","$out/obs/data","$out/tools","$out/licenses" | Out-Null
Copy-Item build/ClipForge.exe $out -Force
# Only load the multimedia modules used by ClipForge; never load the OBS frontend.
Get-ChildItem .deps/obs/bin/64bit -File | Where-Object { $_.Extension -eq '.dll' -or $_.Name -in @('obs-ffmpeg-mux.exe','obs-amf-test.exe','obs-nvenc-test.exe','obs-qsv-test.exe') } | Copy-Item -Destination "$out/obs/bin/64bit" -Force
Copy-Item .deps/obs/bin/64bit/obs.dll $out -Force
Copy-Item .deps/obs/bin/64bit/obs-ffmpeg-mux.exe,.deps/obs/bin/64bit/obs-amf-test.exe,.deps/obs/bin/64bit/obs-qsv-test.exe,.deps/obs/bin/64bit/obs-nvenc-test.exe $out -Force
Get-ChildItem .deps/obs/bin/64bit -Filter '*.dll' | Where-Object { $_.Name -notlike 'Qt*' } | Copy-Item -Destination $out -Force
foreach($name in @('obs-ffmpeg','obs-x264','win-capture','win-wasapi','obs-qsv11','obs-nvenc','image-source')) {
 $dll=".deps/obs/obs-plugins/64bit/$name.dll"
 if(Test-Path $dll){Copy-Item $dll "$out/obs/obs-plugins/64bit" -Force}
 if(Test-Path ".deps/obs/data/obs-plugins/$name"){New-Item -ItemType Directory -Force "$out/obs/data/obs-plugins" | Out-Null;Copy-Item ".deps/obs/data/obs-plugins/$name" "$out/obs/data/obs-plugins" -Recurse -Force}
}
Copy-Item .deps/obs/data/libobs "$out/obs/data" -Recurse -Force
foreach($tool in @('ffmpeg','ffprobe')) { $command=Get-Command "$tool.exe";Copy-Item $command.Source "$out/tools/$tool.exe" -Force }
& .deps/Qt/6.8.3/msvc2022_64/bin/windeployqt.exe --release --no-translations --qmldir "$root/ui" "$out/ClipForge.exe"
if($LASTEXITCODE){throw 'Qt deployment failed.'}
$vs=& "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products '*' -property installationPath
$crt=Get-ChildItem "$vs/VC/Redist/MSVC" -Filter msvcp140.dll -Recurse | Where-Object { $_.FullName -match '\\x64\\Microsoft\.VC\d+\.CRT\\' -and $_.FullName -notmatch '\\onecore\\' } | Sort-Object FullName -Descending | Select-Object -First 1
if(!$crt){throw 'MSVC app-local redistributable DLLs not found.'}
Copy-Item "$($crt.DirectoryName)/*.dll" $out -Force
# Developer tests use the same libraries as the portable build.
Copy-Item build/core_tests.exe $out -Force
Copy-Item build/media_tests.exe $out -Force
Copy-Item build/ClipForgeFixture.exe $out -Force
Copy-Item .deps/Qt/6.8.3/msvc2022_64/plugins/platforms/qoffscreen.dll "$out/platforms" -Force
Copy-Item .deps/Qt/6.8.3/msvc2022_64/bin/Qt6Test.dll $out -Force
Copy-Item README.md,LICENSE,THIRD_PARTY.md $out -Force
Copy-Item .deps/obs-source/obs-studio-32.2.2-sources/COPYING "$out/licenses/OBS-GPL.txt" -Force
Copy-Item docs "$out/docs" -Recurse -Force
@('[Paths]','Plugins=.','QmlImports=qml') | Set-Content "$out/qt.conf" -Encoding ascii
if(!$SkipZip){$zipTool=if(Test-Path 'C:\Program Files\7-Zip\7z.exe'){'C:\Program Files\7-Zip\7z.exe'}else{(Get-Command 7z.exe).Source}; & $zipTool a -tzip '-mx=5' '-mmt=2' "$root/dist/ClipForge-Windows-x64.zip" "$out/*" | Out-Null;if($LASTEXITCODE){throw 'ZIP failed (a full 7-Zip installation with ZIP support is required).'}}
Write-Host "Portable application: $out/ClipForge.exe"
$compiler='C:\Program Files (x86)\Inno Setup 6\ISCC.exe'
if((Test-Path $compiler) -and !$SkipZip){& $compiler "/DSourceDir=$out" "$root/packaging/ClipForge.iss";if($LASTEXITCODE){throw 'Installer compilation failed.'}}
