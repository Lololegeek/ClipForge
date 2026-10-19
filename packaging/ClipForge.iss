#ifndef SourceDir
 #define SourceDir "..\dist\ClipForge"
#endif
#ifndef AppVersion
 #define AppVersion "0.1.1"
#endif
[Setup]
AppId={{C38E01B3-9388-4B47-ACE2-04EB912F4160}
AppName=ClipForge
AppVersion={#AppVersion}
AppPublisher=ClipForge contributors
AppPublisherURL=https://github.com/Lololegeek/ClipForge
AppSupportURL=https://github.com/Lololegeek/ClipForge/issues
AppUpdatesURL=https://github.com/Lololegeek/ClipForge/releases/latest
DefaultDirName={localappdata}\Programs\ClipForge
DefaultGroupName=ClipForge
PrivilegesRequired=lowest
MinVersion=10.0.19045
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir=..\dist
OutputBaseFilename=ClipForge-Setup-x64
Compression=lzma2/fast
SolidCompression=yes
WizardStyle=modern
SetupIconFile=..\assets\ClipForge.ico
VersionInfoVersion={#AppVersion}
UninstallDisplayIcon={app}\ClipForge.exe
LicenseFile=..\LICENSE
CloseApplications=yes
SetupLogging=yes
[Languages]
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
[Tasks]
Name: "desktopicon"; Description: "Créer un raccourci sur le bureau"; Flags: unchecked
Name: "startup"; Description: "Démarrer ClipForge dans la zone de notification à l'ouverture de session"; Flags: unchecked
[Files]
Source: "{#SourceDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs; Excludes: "core_tests.exe,media_tests.exe,ClipForgeFixture.exe,*.pdb"
[Icons]
Name: "{group}\ClipForge"; Filename: "{app}\ClipForge.exe"; IconFilename: "{app}\ClipForge.ico"; AppUserModelID: "ClipForge.Desktop"
Name: "{autodesktop}\ClipForge"; Filename: "{app}\ClipForge.exe"; IconFilename: "{app}\ClipForge.ico"; AppUserModelID: "ClipForge.Desktop"; Tasks: desktopicon
[Registry]
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "ClipForge"; ValueData: """{app}\ClipForge.exe"" --headless"; Tasks: startup; Flags: uninsdeletevalue
[Run]
Filename: "{app}\ClipForge.exe"; Parameters: "--headless"; Flags: nowait; Check: Relaunch
Filename: "{app}\ClipForge.exe"; Description: "Lancer ClipForge"; Flags: nowait postinstall skipifsilent unchecked

[Code]
function Relaunch: Boolean;
begin
  Result := ExpandConstant('{param:relaunch|0}') = '1';
end;
