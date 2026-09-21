# Kinect 360 Virtual Camera (libfreenect + WinUSB Edition)

A modernized 64-bit DirectShow virtual webcam filter for the Microsoft Xbox 360 Kinect (Kinect v1 / Model 1414).

This branch replaces the legacy Microsoft Kinect SDK 1.8 drivers with an open-source `libfreenect` + Microsoft `WinUSB` architecture. This allows the Kinect camera to function on modern 64-bit Windows 10 and Windows 11 systems without disabling Memory Integrity (HVCI) or installing obsolete 2013 kernel drivers.

---

> [!CAUTION]
> ### ⚠️ EXPERIMENTAL & AI-ASSISTED PROJECT DISCLAIMER
> 
> * **Personal Test Project:** This is a personal test project created strictly for experimentation and hobbyist use.
> * **AI-Assisted Development:** The modifications, driver abstraction layer, and scripts in this repository were largely developed and generated with the assistance of AI.
> * **Install at Your Own Risk:** This software is provided **"as-is"**, without warranty of any kind, express or implied. The author assumes no responsibility or liability for any issues, system instability, or damages that may arise from using or installing this software.
> * **No Support:** This project is not actively maintained, supported, or monitored for issues. No technical support or bug fixes are provided.

---

## What This Version Does

* **Driver Model:** Uses Microsoft's built-in `WinUSB` (`winusb.sys`) via `libfreenect` in user space.
* **Modern Windows Security:** Fully compatible with Windows 11 Core Isolation / Memory Integrity (HVCI).
* **DirectShow Output:** Exposes the Kinect RGB camera stream as a standard DirectShow video capture source (accessible in OBS Studio, browser webcams, Discord, etc.).
* **Zero External SDKs Required:** No Microsoft Kinect for Windows SDK or developer toolkits are required.

## Building from Source

Requires Visual Studio 2022 (with C++ Desktop workload) and CMake 3.20+:

```cmd
cmake --preset x64-release
cmake --build out/build/x64-release --config Release
```

The resulting 64-bit DirectShow filter will be located at:
`out/build/x64-release/KinectInfraredCam.ax`

## Building the Installer (.iss)

If you have [Inno Setup 6](https://jrsoftware.org/isinfo.php) installed, you can compile the standalone setup installer:

```cmd
iscc installer.iss
```
*(Or open `installer.iss` in the Inno Setup Compiler GUI and click **Build -> Compile**).*

The compiled installer will be generated at:
`out/installer/KinectCam-Setup-x64.exe`

## Manual Registration

If running without the installer, you can register or unregister the filter manually from an elevated (Administrator) command prompt:

* **Register:** Run `Reg.cmd`
* **Unregister:** Run `UnReg.cmd`

## License

This project incorporates components licensed under the Apache 2.0 License, GPL v2 / Apache 2.0 (libfreenect), and LGPL v2.1 (libusb). See [LICENSE.txt](LICENSE.txt) for details.
