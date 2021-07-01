[Setup]
AppName= MetroTools
AppVerName= MetroTools 11
DefaultDirName= {pf}\MetroTools
DefaultGroupName= MetroTools
UninstallDisplayIcon= {app}\Uninstall.exe
AllowNoIcons=Yes
OutputDir="."
OutputBaseFilename=setup
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: ru; MessagesFile: "compiler:Languages\Russian.isl"

[Tasks]
Name: "desktopicon"; Description: "Создать ярлыки на рабочем столе"; GroupDescription: "Создание ярлыков";

[Files]
Source: "bin\*.*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs;

[Icons]
Name: "{group}\MetroEX"; Filename: "{app}\MetroEX.exe"; WorkingDir: "{app}";
Name: "{group}\MetroTEX"; Filename: "{app}\MetroTEX.exe"; WorkingDir: "{app}";
Name: "{group}\MetroME"; Filename: "{app}\MetroME.exe"; WorkingDir: "{app}";
Name: "{group}\Удалить Metro Tools"; Filename: "{uninstallexe}"

; Shortcuts
Name: "{userdesktop}\MetroEX"; Filename: "{app}\MetroEX.exe"; Tasks: desktopicon
Name: "{userdesktop}\MetroTEX"; Filename: "{app}\MetroTEX.exe"; Tasks: desktopicon
Name: "{userdesktop}\MetroME"; Filename: "{app}\MetroME.exe"; Tasks: desktopicon