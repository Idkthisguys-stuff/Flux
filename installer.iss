#ifndef MySourcePath
  #define MySourcePath "out\install\x64-debug"
#endif

[Setup]
AppName=Flux
AppVersion=1.0.0
DefaultDirName={autopf}\FluxEngine
DefaultGroupName=FluxEngine
OutputBaseFilename=FluxEngineInstaller
Compression=lzma
SolidCompression=yes

[Files]
Source: "{#MySourcePath}\bin\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs

[Icons]
Name: "{group}\FluxEngine"; Filename: "{app}\Flux.exe"
Name: "{userdesktop}\FluxEngine"; Filename: "{app}\Flux.exe"

[Registry]
Root: HKCR; Subkey: ".fscn"; ValueType: string; ValueName: ""; ValueData: "FluxEngineSceneFile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "FluxEngineSceneFile"; ValueType: string; ValueName: ""; ValueData: "Flux Engine Scene"; Flags: uninsdeletekey
Root: HKCR; Subkey: "FluxEngineSceneFile\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\Flux.exe,0"
Root: HKCR; Subkey: "FluxEngineSceneFile\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\Flux.exe"" ""%1"""

Root: HKCR; Subkey: ".flux"; ValueType: string; ValueName: ""; ValueData: "FluxEngineFile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "FluxEngineFile"; ValueType: string; ValueName: ""; ValueData: "Flux Engine Project"; Flags: uninsdeletekey
Root: HKCR; Subkey: "FluxEngineFile\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\Flux.exe,0"
Root: HKCR; Subkey: "FluxEngineFile\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\Flux.exe"" ""%1"""