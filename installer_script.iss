; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Prehistoric Engine"
#define MyAppVersion "1.0"
#define MyAppPublisher "Prehistoric, Inc."
#define MyAppURL "https://www.google.com/"
#define MyAppExeName "Prehistoric.exe"
#define MyAppAssocName MyAppName + " Setup File"
#define MyAppAssocExt ".msi"
#define MyAppAssocKey StringChange(MyAppAssocName, " ", "") + MyAppAssocExt

[Setup]
; NOTE: The value of AppId uniquely identifies this application. Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{6EFEDB72-5CAC-4824-AB05-CA5E4932F38A}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
ChangesAssociations=yes
DisableProgramGroupPage=yes
LicenseFile=F:\Prehistoric - C++\Visual Studio Projects\PrehistoricEngine\LICENSE
; Uncomment the following line to run in non administrative install mode (install for current user only.)
;PrivilegesRequired=lowest
OutputBaseFilename=PrehistoricEngineSetup
Compression=lzma
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "F:\Prehistoric - C++\Visual Studio Projects\PrehistoricEngine\bin\Distribution-windows-x86_64\Prehistoric\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "F:\Prehistoric - C++\Visual Studio Projects\PrehistoricEngine\PrehistoricEngine\Dependencies\redist\*"; DestDir: "{app}\redist\"; Flags: ignoreversion
Source: "F:\Prehistoric - C++\Visual Studio Projects\PrehistoricEngine\PrehistoricEngine\Dependencies\shared_libraries\Windows_x64\*"; DestDir: "{app}";
Source: "F:\Prehistoric - C++\Visual Studio Projects\PrehistoricEngine\Prehistoric\res\*"; DestDir: "{app}\res\"; Flags: ignoreversion recursesubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Registry]
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocExt}\OpenWithProgids"; ValueType: string; ValueName: "{#MyAppAssocKey}"; ValueData: ""; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}"; ValueType: string; ValueName: ""; ValueData: "{#MyAppAssocName}"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""
Root: HKA; Subkey: "Software\Classes\Applications\{#MyAppExeName}\SupportedTypes"; ValueType: string; ValueName: ".myp"; ValueData: ""

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]         
Filename: "{app}\oalinst.exe"; Flags: postinstall 
Filename: "{app}\VC_redist.x64.exe"; Flags: postinstall
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

