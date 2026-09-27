# Kinect 360 Virtual Camera

A modernized 64-bit DirectShow virtual webcam filter and control panel utility for the Microsoft Xbox 360 Kinect (Kinect v1 / Model 1414).

This project allows the Xbox 360 Kinect camera to function as a regular webcam on modern 64-bit Windows 10 and Windows 11 systems without disabling Memory Integrity (HVCI) or installing obsolete kernel drivers.

---

> [!CAUTION]
> ### ⚠️ EXPERIMENTAL & AI-ASSISTED PROJECT DISCLAIMER
> 
> * **Personal Test Project:** This is a personal test project created strictly for experimentation and hobbyist use.
> * **AI-Assisted Development:** The modifications, driver abstraction layer, and scripts in this repository were largely developed and generated with the assistance of AI.
> * **Install at Your Own Risk:** This software is provided **"as-is"**, without warranty of any kind, express or implied. The author assumes no responsibility or liability for any issues, system instability, or damages that may arise from using or installing this software.
> * **No Support:** This project is not actively maintained, supported, or monitored for issues. No technical support or bug fixes are provided.

---

## Features

* **DirectShow Virtual Camera:** Streams Kinect video directly to OBS Studio, Discord, Google Meet, Zoom, and web browsers.
* **RGB & Infrared (Night Vision):** Switch between full-color RGB and infrared night-vision video modes on the fly.
* **Native Control Panel (`KinectCamControl.exe`):**
  * **Live Motor Tilt:** Smooth hardware angle adjustment (-27° to +27°).
  * **LED Control:** Solid Green, Solid Red, Solid Yellow, Blinking, or Off.
  * **Standby Blinking Fix:** Automatically silences the blinking green light when idle.
  * **System Tray:** Runs quietly in the notification area with quick right-click options.
* **Modern Windows Security:** Runs entirely in user mode via Microsoft's built-in `WinUSB` driver—fully compatible with Windows 11 Core Isolation / Memory Integrity (HVCI).

---

## Requirements

* **Operating System:** 64-bit Windows 10 or Windows 11.
* **Hardware:** Microsoft Xbox 360 Kinect Sensor (Model 1414) + 12V AC power supply adapter & USB breakout cable.
* **Driver Tool (Required for first-time setup):** [Zadig](https://zadig.akeo.ie) (Because the Kinect is not a standard UVC webcam, modern Windows requires associating it with Microsoft's built-in `WinUSB` driver once using Zadig).

---

## Installation Guide

Follow these steps to set up your Kinect as a webcam:

### Step 1: Connect Hardware
1. Plug the Kinect's 12V power supply into a wall outlet.
2. Plug the USB cable into your PC (USB 2.0 or USB 3.0 port).
3. The sensor's power LED will illuminate.

### Step 2: Configure Driver with Zadig (One-Time Setup)
1. Download and run **[Zadig](https://zadig.akeo.ie)** (free, portable, no installation required).
2. Click the **Options** menu at the top and select **List All Devices**.
3. In the device dropdown:
   * Select **`Xbox NUI Camera`** (USB ID `045E 02AE`).
   * Ensure the target driver (right side of the green arrow) is set to **`WinUSB`**.
   * Click **"Replace Driver"** (or *"Install Driver"*).
4. *(Optional - for motor tilt and LED controls)*:
   * Select **`Xbox NUI Motor`** (USB ID `045E 02B0`) from the dropdown.
   * Click **"Replace Driver"** to `WinUSB` as well.

### Step 3: Run the Installer Executable
1. Download the latest **`KinectCam-Setup-x64.exe`** from the **[Releases](https://github.com/SebastianCaceres/KinectCam/releases)** page.
2. Run the installer (requires Administrator privileges to register the DirectShow filter).
3. Follow the setup wizard to complete the installation.

### Step 4: Start Using Your Kinect
* **Webcam:** Open OBS Studio, Discord, or any webcam test tool and select **`Kinect Cam`** as your video capture device.
* **Control Panel:** Launch **Kinect Control Panel** from the Start Menu or desktop shortcut to adjust tilt angle, toggle night vision (IR), or configure LEDs.

---

## Manual Installation (Portable)

If you downloaded the portable zip archive (`KinectCam-Portable-x64.zip`) instead of running the installer:
1. Extract the zip to your desired permanent folder (e.g. `C:\Program Files\KinectCam`).
2. Right-click **`Reg.cmd`** and select **Run as administrator** to register the DirectShow filter.
3. Launch **`KinectCamControl.exe`**.
4. To uninstall later, right-click **`UnReg.cmd`** and select **Run as administrator**.

---

## Building from Source

Requires Visual Studio 2022 (with the **Desktop development with C++** workload) and CMake 3.20+:

```cmd
cmake --preset x64-release
cmake --build out/build/x64-release --config Release
```

Output binaries will be generated in `out/build/x64-release/`:
* `KinectInfraredCam.ax` (Virtual Camera DirectShow Filter)
* `KinectCamControl.exe` (Control Panel Utility)

To build the installer executable, install [Inno Setup 6](https://jrsoftware.org/isinfo.php) and run:
```cmd
iscc installer.iss
```

---

## Acknowledgments & Credits

* **[VisualError](https://github.com/VisualError/KinectCam):** Core credit to VisualError for the original KinectCam revival, CMake structure, and DirectShow filter foundation that made this project possible.
* **[wildbillcat](https://github.com/wildbillcat/KinectCam) & [roman380](https://github.com/roman380/tmhare.mvps.org-vcam):** Early KinectCam and DirectShow VCam sample implementations.
* **[OpenKinect](https://openkinect.org):** The `libfreenect` team for the open-source Kinect USB protocol reverse engineering and user-mode drivers.

---

## License

This project incorporates components licensed under the Apache 2.0 License, GPL v2 / Apache 2.0 (libfreenect), and LGPL v2.1 (libusb). See [LICENSE.txt](LICENSE.txt) for details.

---

> **Notice:** This project is an independent open-source tool and is not affiliated with, endorsed by, or sponsored by Microsoft Corporation. "Kinect" and "Xbox" are registered trademarks of Microsoft Corporation.
