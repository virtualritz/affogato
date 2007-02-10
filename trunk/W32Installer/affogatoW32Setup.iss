;#include "liquidversion.iss"

[Setup]
AppPublisher=Rising Sun Pictures
AppPublisherURL=http://www.rsp.com.au/
AppSupportURL=http://affogato.sf.net/
AppUpdatesURL=http://sourceforge.net/projects/affogato
DefaultDirName={pf}\Affogato
DefaultGroupName=Affogato
AppName=Affogato
AppVersion=0.84.9
AppVerName=Affogato 0.84.9 for Softimage XSI 6
OutputBaseFileName=Affogato-0.84.9-Setup
AllowNoIcons=true
LicenseFile=LICENSE.rtf
WizardImageFile=.\affogato.bmp
WizardSmallImageFile=.\affogatosmall.bmp
WizardImageBackColor=$FFFFFF
AlwaysShowDirOnReadyPage=true
AlwaysShowGroupOnReadyPage=true
Compression=lzma/ultra
SolidCompression=yes
FlatComponentsList=true
WindowVisible=false
DisableStartupPrompt=true
PrivilegesRequired=admin
ShowTasksTreeLines=yes
;Encryption=yes
;Password=cibo4Caffe


[Dirs]
Name: {app}\bin;
;Name: {app}\python;
;Name: {app}\scripts;
;Name: {app}\renderers;
Name: {app}\shaders;
Name: {app}\shaders\src;
Name: {app}\shaders\include;

[Components]
Name: "main"; Description: "Plug-In"; Types: full minimal custom; Flags: fixed
Name: "shaders"; Description: "Shaders"; Types: full custom

[Files]
Source: ..\LICENSE.txt; DestDir: {app}; Components: main
;Source: ..\README.txt; DestDir: {app}; Components: main
Source: ..\CHANGES.txt; DestDir: {app}; Components: main
Source: ..\bin\affogato.dll; DestDir: {app}\bin; Components: main
;Source: ..\scripts\*.py; DestDir: {app}\scripts; Components: main
;Source: ..\python\*.py; DestDir: {app}\python; Components: main
Source: ..\shaders\compile.cmd; DestDir: {app}\shaders; Flags: deleteafterinstall; Components: shaders
Source: ..\shaders\src\*.sl; DestDir: {app}\shaders\src; Components: shaders
Source: ..\shaders\include\*.h; DestDir: {app}\shaders\include; Components: main
;Source: ..\artwork\*.png; DestDir: {app}\artwork; Components: main

[Tasks]
Name: registry; Description: Create AFFOGATOHOME &environment variable; MinVersion: 0,4.0.1381; Check: AdminPrivileges
Name: shaders; Description: &Compile Affogato shaders; MinVersion: 0,4.0.1381

[Icons]
Name: {group}\Affogato Wiki; Filename: {app}\AffogatoWiki.url
Name: {group}\Affogato Project Homepage; Filename: {app}\AffogatoHome.url
;Name: {group}\Affogato Website; Filename: {app}\Affogato.url
Name: {group}\Uninstall Affogato; Filename: {uninstallexe}
;Name: {group}\Artwork; Filename: {app}\artwork; Flags: foldershortcut

[Messages]
WelcomeLabel2=This will install [name/ver] on your computer.%n%nIt is recommended — but not strictly neccessary — that you close all other applications before continuing.%n%n
DiskSpaceMBLabel=The program requires at least [mb] MB of disk space.
ComponentsDiskSpaceMBLabel=The current selection requires at least [mb] MB of disk space.
FinishedLabel=Setup has finished installing [name] on your computer.%n%nThanks for using [name].%n
SelectTasksLabel2=Select the additional tasks you would like Setup to perform, then click Next.
NoProgramGroupCheck2=&Suppress creation of a Start Menu folder for Affogato

[Registry]
Root: HKLM; Subkey: SYSTEM\CurrentControlSet\Control\Session Manager\Environment; ValueType: string; ValueName: AFFOGATOHOME; ValueData: {app}; MinVersion: 0,4.00.1381; Flags: uninsdeletevalue; Tasks: registry

[Types]
Name: full; Description: Full Installation
Name: minimal; Description: Minimal Installation — Just the Plug-In
Name: custom; Description: Custom Installation; Flags: iscustom

[Run]
Filename: "{app}\shaders\compile.cmd"; WorkingDir: "{app}\shaders"; Flags: runhidden shellexec waituntilidle skipifdoesntexist; Tasks: shaders
Filename: "{app}\CHANGES.txt"; Description: "View CHANGES.txt"; Flags: postinstall nowait shellexec skipifdoesntexist skipifsilent

[UninstallDelete]
Type: files; Name: {app}\Affogato.url

[INI]
Filename: {app}\AffogatoWiki.url; Section: InternetShortcut; Key: URL; String: http://affogato.sf.net/
Filename: {app}\AffogatoHome.url; Section: InternetShortcut; Key: URL; String: http://www.sf.net/projects/affogato/

[LangOptions]
WelcomeFontName=Tahoma
DialogFontName=Tahoma
DialogFontSize=8

[_ISTool]
EnableISX=true

[Code]
function InsertPath_NT(PathToAdd: String; RootKey: Integer; Location: String; PathVar: String): Boolean;
var CurrentPath: String;
var Offset: Integer;
begin
  Offset := 0;
  if RegValueExists(RootKey, Location, PathVar) then begin
    RegQueryStringValue(RootKey, Location, PathVar, CurrentPath);
    if Pos(PathToAdd, CurrentPath) = 0 then begin
      if (StrGet(CurrentPath, Length(CurrentPath)) <> ';') then
        Offset := 1;
      Insert(';', CurrentPath, Length(CurrentPath)+Offset);
      Insert(PathToAdd, CurrentPath, Length(CurrentPath)+Offset);
      RegWriteStringValue(RootKey, Location, PathVar, CurrentPath);
    end;
  end
  else
    RegWriteStringValue(RootKey, Location, PathVar, PathToAdd);
  Result := True;
end;

function NextButtonClick(CurPage: Integer): Boolean;
var EnvName: String;
begin
  EnvName := 'Environment';
  if CurPage = wpFinished then begin
    InsertPath_NT(ExpandConstant('{app}\bin'), HKLM, 'SYSTEM\CurrentControlSet\Control\Session Manager\Environment', 'XSI_PLUGINS')
    SendBroadcastMessage(26, 0, CastStringToInteger(EnvName));
  end;
  Result := True;
end;

function PowerUserPrivileges(): Boolean;
begin
  Result := IsPowerUserLoggedOn() or IsAdminLoggedOn();
end;

function AdminPrivileges(): Boolean;
begin
  Result := IsAdminLoggedOn();
end;

function NoAdminPrivileges(): Boolean;
begin
  Result := not IsAdminLoggedOn();
end;

