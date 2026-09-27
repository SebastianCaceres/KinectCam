; Inno Setup Script for Kinect Virtual Camera
#define MyAppName "Kinect Virtual Camera"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "KinectCam Open Source Project"

[Setup]
AppId={{8E14549A-DB61-4309-AFA1-3578E927E933}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\KinectCam
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
OutputDir=out\installer
OutputBaseFilename=KinectCam-Setup-x64
Compression=lzma2/max
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
WizardStyle=modern
SetupIconFile=assets\kinect.ico
UninstallIconFile=assets\kinect.ico
InfoAfterFile=DRIVER_SETUP.txt

[Files]
; The 64-bit DirectShow Virtual Camera filter (registers automatically with regserver flag)
Source: "out\build\x64-release\KinectInfraredCam.ax"; DestDir: "{app}"; Flags: regserver restartreplace
; Kinect Control Panel utility
Source: "out\build\x64-release\KinectCamControl.exe"; DestDir: "{app}"; Flags: restartreplace
; Manual scripts for convenience
Source: "out\build\x64-release\Reg.cmd"; DestDir: "{app}"
Source: "out\build\x64-release\UnReg.cmd"; DestDir: "{app}"
; Documentation
Source: "README.md"; DestDir: "{app}"; Flags: isreadme
Source: "DRIVER_SETUP.txt"; DestDir: "{app}"
Source: "LICENSE.txt"; DestDir: "{app}"
; Icon Asset
Source: "assets\kinect.ico"; DestDir: "{app}\assets"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Icons]
Name: "{group}\Kinect Control Panel"; Filename: "{app}\KinectCamControl.exe"; IconFilename: "{app}\assets\kinect.ico"
Name: "{autodesktop}\Kinect Control Panel"; Filename: "{app}\KinectCamControl.exe"; IconFilename: "{app}\assets\kinect.ico"; Tasks: desktopicon
Name: "{group}\Driver Setup Instructions"; Filename: "{app}\DRIVER_SETUP.txt"
Name: "{group}\Download Zadig (Driver Tool)"; Filename: "https://zadig.akeo.ie"
Name: "{group}\Register Kinect Cam"; Filename: "{app}\Reg.cmd"
Name: "{group}\Unregister Kinect Cam"; Filename: "{app}\UnReg.cmd"
Name: "{group}\Uninstall Kinect Cam"; Filename: "{uninstallexe}"

[Run]
Filename: "https://zadig.akeo.ie"; Description: "Open Zadig website to install WinUSB driver (Required for first-time Kinect setup)"; Flags: shellexec postinstall skipifsilent
Filename: "{app}\KinectCamControl.exe"; Description: "Launch Kinect Control Panel"; Flags: nowait postinstall skipifsilent

[UninstallRun]
Filename: "taskkill.exe"; Parameters: "/f /im KinectCamControl.exe"; Flags: runhidden
Filename: "{sys}\regsvr32.exe"; Parameters: "/u /s ""{app}\KinectInfraredCam.ax"""; Flags: runhidden
