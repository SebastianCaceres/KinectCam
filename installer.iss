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

[Files]
; The 64-bit DirectShow Virtual Camera filter (registers automatically with regserver flag)
Source: "out\build\x64-release\KinectInfraredCam.ax"; DestDir: "{app}"; Flags: regserver restartreplace
; Manual scripts for convenience
Source: "out\build\x64-release\Reg.cmd"; DestDir: "{app}"
Source: "out\build\x64-release\UnReg.cmd"; DestDir: "{app}"
; Documentation
Source: "README.md"; DestDir: "{app}"; Flags: isreadme
Source: "LICENSE.txt"; DestDir: "{app}"

[Icons]
Name: "{group}\Register Kinect Cam"; Filename: "{app}\Reg.cmd"
Name: "{group}\Unregister Kinect Cam"; Filename: "{app}\UnReg.cmd"
Name: "{group}\Uninstall Kinect Cam"; Filename: "{uninstallexe}"

[UninstallRun]
Filename: "{sys}\regsvr32.exe"; Parameters: "/u /s ""{app}\KinectInfraredCam.ax"""; Flags: runhidden
