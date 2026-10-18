param([switch]$Capture,[switch]$WindowCapture,[switch]$FullHD,[switch]$Software,[int]$Seconds=32)
$ErrorActionPreference='Stop'
Set-Location (Split-Path $PSScriptRoot -Parent)
New-Item -ItemType Directory -Force artifacts | Out-Null
& ./dist/ClipForge/core_tests.exe -o artifacts/unit.txt,txt
if($LASTEXITCODE){throw 'Core tests failed.'}
if($Capture -or $WindowCapture){
 $name=if($WindowCapture){'window-test'}else{'capture-test'}
 $data=Join-Path $PWD ("artifacts/"+$name+'-'+(Get-Date -Format 'yyyyMMdd-HHmmss'))
 $arguments=@('--self-test','--headless','--test-seconds',"$Seconds",'--data-dir',('"'+$data+'"'))
 if($WindowCapture){$arguments+='--window-test'}
 if($FullHD){$arguments+='--test-1080'}
 if($Software){$arguments+='--software-test'}
 $p=Start-Process "$PWD/dist/ClipForge/ClipForge.exe" -ArgumentList $arguments -WindowStyle Hidden -PassThru
 if(!$p.WaitForExit(($Seconds+30)*1000)){ $p.Kill();throw 'Capture test timed out.' }
 if($p.ExitCode){throw "Capture application failed: $($p.ExitCode)"}
 $validation=@('scripts/validate_media.py',$data,'--seconds',$(if($Seconds -gt 60){60}else{15}))
 if($FullHD){$validation+=@('--width','1920','--height','1080','--fps','60')}
 python @validation
 if($LASTEXITCODE){throw 'Media validation failed.'}
 $env:CLIPFORGE_TEST_VIDEO=(Get-ChildItem "$data/clips" -Filter '*.mp4'|Select-Object -First 1).FullName
 $env:CLIPFORGE_TEST_OUTPUT=Join-Path $data 'export.mp4'
 if(!$FullHD){& ./dist/ClipForge/media_tests.exe -o "$data/media-tests.txt",txt}
 if($LASTEXITCODE){throw 'Media integration tests failed.'}
}
Write-Host 'Requested checks passed.'
