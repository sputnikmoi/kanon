; -- Kanon.iss, Innosetup Script --
; 3.1.1: Built with D:\Qt\5.14.1\mingw73_64, dynamically linked.
; See also D:\0\GitHub\kanon
[Setup]
AppName=Kanon
AppVersion=3.1.1
WizardStyle=modern
DefaultDirName={autopf}\Kanon
DefaultGroupName=Kanon
UninstallDisplayIcon={app}\Bin\Kanon.exe
Compression=lzma2
SolidCompression=yes
OutputBaseFilename=Kanon_3.1.1_Setup
LicenseFile=license.txt

OutputDir=InnoOutput
; "ArchitecturesAllowed=x64" specifies that Setup cannot run on
; anything but x64.
ArchitecturesAllowed=x64
; "ArchitecturesInstallIn64BitMode=x64" requests that the install be
; done in "64-bit mode" on x64, meaning it should use the native
; 64-bit Program Files directory and the 64-bit view of the registry.
ArchitecturesInstallIn64BitMode=x64

SourceDir=H:\_Share\pkt\Kanon

[Dirs] 
Name: "{app}\Data"
Name: "{app}\Bin"
;Name: "{app}\Bin\plugins\generic"
;Name: "{app}\Bin\plugins\iconengines"
;Name: "{app}\Bin\plugins\imageformats"
;Name: "{app}\Bin\plugins\mediaservice"
;Name: "{app}\Bin\plugins\platforminputcontexts"
;Name: "{app}\Bin\plugins\platforms"
;Name: "{app}\Bin\plugins\platformthemes"
;Name: "{app}\Bin\plugins\printsupport"
;Name: "{app}\Bin\plugins\renderplugins"
;Name: "{app}\Bin\plugins\sqldrivers"
;Name: "{app}\Bin\plugins\styles"

;Bin
[Files]
Source: "Bin\*.exe"; DestDir: "{app}\Bin"
Source: "Bin\*.dll"; DestDir: "{app}\Bin"
;Plugins
Source: "Bin\plugins\generic\*";        DestDir: "{app}\Bin\plugins\generic"
Source: "Bin\plugins\iconengines\*";    DestDir: "{app}\Bin\plugins\iconengines"
Source: "Bin\plugins\imageformats\*";   DestDir: "{app}\Bin\plugins\imageformats"
Source: "Bin\plugins\mediaservice\*";   DestDir: "{app}\Bin\plugins\mediaservice"
Source: "Bin\plugins\platforminputcontexts\*"; DestDir: "{app}\Bin\plugins\platforminputcontexts"
Source: "Bin\plugins\platforms\*";      DestDir: "{app}\Bin\plugins\platforms"
Source: "Bin\plugins\platformthemes\*"; DestDir: "{app}\Bin\plugins\platformthemes"
Source: "Bin\plugins\printsupport\*";   DestDir: "{app}\Bin\plugins\printsupport"
Source: "Bin\plugins\renderplugins\*";  DestDir: "{app}\Bin\plugins\renderplugins"
Source: "Bin\plugins\sqldrivers\*";     DestDir: "{app}\Bin\plugins\sqldrivers"
Source: "Bin\plugins\styles\*";         DestDir: "{app}\Bin\plugins\styles"

;Help
Source: "Help\Kanon.qch"; DestDir: "{app}\Bin"
Source: "Help\Kanon.qhc"; DestDir: "{app}\Bin"

; -- Data files
Source: "Data\*.kxm"; DestDir: "{app}\Data"

[Icons]
Name: "{group}\Kanon"; Filename: "{app}\Bin\Kanon.exe"; WorkingDir: "{app}\Bin"

[Run]
Filename: "{app}\Bin\Kanon.exe"; Description: "Launch application"; Flags: postinstall shellexec skipifsilent


