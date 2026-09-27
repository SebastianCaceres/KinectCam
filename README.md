# Kinect 360 Virtual Camera (libfreenect + WinUSB Edition)

A modernized 64-bit DirectShow virtual webcam filter for the Microsoft Xbox 360 Kinect (Kinect v1 / Model 1414).

This project replaces the legacy Microsoft Kinect SDK 1.8 drivers with an open-source `libfreenect` + Microsoft `WinUSB` architecture. This allows the Kinect camera to function on modern 64-bit Windows 10 and Windows 11 systems without disabling Memory Integrity (HVCI) or installing obsolete 2013 kernel drivers.

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

## First-Time Driver Setup: Why WinUSB & Zadig are Needed

### Why is this step necessary?
The Xbox 360 Kinect (Model 1414) was built in 2010 exclusively for the Xbox 360 console and does not use standard UVC webcam protocols.
* The obsolete 2013 Microsoft Kinect SDK installed a proprietary kernel driver (`kinect10.sys`), which Windows 11 blocks as an unverified security risk under **Memory Integrity (HVCI)**.
* This project runs completely in user space over Microsoft's built-in, secure **`WinUSB` (`winusb.sys`)** driver.
* Because Windows does not automatically assign `winusb.sys` to the Xbox 360 Kinect by default, you use the free, open-source tool **Zadig** once to associate `WinUSB` with the sensor. Once completed, the Kinect is 100% plug-and-play on your PC.

### Step-by-Step Zadig Guide (1-Time Setup)

1. **Connect Hardware:** Plug the Kinect's 12V wall power supply into an outlet, and connect the USB cable to your PC.
2. **Download Zadig:** Download the official portable tool from **[https://zadig.akeo.ie](https://zadig.akeo.ie)** (standalone, no installer needed).
3. **Show All Devices:** Open Zadig, click the **Options** menu at the top, and select **List All Devices**.
4. **Select Kinect Camera:** In the dropdown list, choose **`Xbox NUI Camera`** (USB ID `045E 02AE`).
5. **Install WinUSB:** Verify the driver on the right side of the green arrow is set to **`WinUSB`**, then click **"Replace Driver"** (or *"Install Driver"*).
6. *(Optional - for Tilt & LED Control)*: In the dropdown, also select **`Xbox NUI Motor`** (USB ID `045E 02B0`) and click **"Replace Driver"** to WinUSB.
7. **Complete!** Launch OBS Studio or the Kinect Control Panel.

## Building from Source

Requires Visual Studio 2022 (with C++ Desktop workload) and CMake 3.20+:

```cmd
cmake --preset x64-release
cmake --build out/build/x64-release --config Release
```

The resulting 64-bit binaries will be located at:
* `out/build/x64-release/KinectInfraredCam.ax` (Virtual Camera DirectShow Filter)
* `out/build/x64-release/KinectCamControl.exe` (Control Panel & System Tray Utility)

## Kinect Control Panel Utility

`KinectCamControl.exe` provides a lightweight native Windows interface to manage the sensor:
* **Live Tilt Slider:** Adjust hardware angle (-27° to +27°) with instant response.
* **Sensor Mode Selector:** Seamlessly switch between **RGB Color Camera** and **Infrared (Night Vision)** without re-registering filters.
* **LED Control:** Choose between Off, Solid Green, Solid Red, Solid Yellow, or Blinking modes.
* **Standby Blinking Fix:** Automatically silences the blinking green light when your PC is idle.
* **System Tray:** Minimizes quietly to the system tray with a quick-access right-click menu.

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

## Acknowledgments & Credits

* **[VisualError](https://github.com/VisualError/KinectCam):** Core credit to VisualError for the original KinectCam revival, CMake structure, and DirectShow filter foundation that made this project possible.
* **[wildbillcat](https://github.com/wildbillcat/KinectCam) & [roman380](https://github.com/roman380/tmhare.mvps.org-vcam):** Early KinectCam and DirectShow VCam sample implementations.
* **[OpenKinect](https://openkinect.org):** The `libfreenect` team for the open-source Kinect USB protocol reverse engineering and user-mode drivers.

## License

This project incorporates components licensed under the Apache 2.0 License, GPL v2 / Apache 2.0 (libfreenect), and LGPL v2.1 (libusb). See [LICENSE.txt](LICENSE.txt) for details.

---

> **Notice:** This project is an independent open-source tool and is not affiliated with, endorsed by, or sponsored by Microsoft Corporation. "Kinect" and "Xbox" are registered trademarks of Microsoft Corporation.
