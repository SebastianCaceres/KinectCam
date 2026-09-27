#pragma once

#include <windows.h>
#include <string>

namespace KinectSettings
{
    constexpr const wchar_t* REG_KEY = L"Software\\KinectCam";

    constexpr int VIDEO_MODE_RGB = 0;
    constexpr int VIDEO_MODE_IR  = 1;

    constexpr int DEFAULT_TILT       = 0;
    constexpr int DEFAULT_VIDEO_MODE = VIDEO_MODE_RGB;
    constexpr int DEFAULT_LED_COLOR  = 1; // 1 = LED_GREEN
    constexpr int DEFAULT_STANDBY_OFF = 1;

    inline DWORD GetDword(const wchar_t* valueName, DWORD defaultValue)
    {
        HKEY hKey = nullptr;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, REG_KEY, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        {
            return defaultValue;
        }

        DWORD data = 0;
        DWORD dataSize = sizeof(data);
        DWORD type = REG_DWORD;
        LONG res = RegQueryValueExW(hKey, valueName, nullptr, &type, (LPBYTE)&data, &dataSize);
        RegCloseKey(hKey);

        if (res == ERROR_SUCCESS && type == REG_DWORD)
        {
            return data;
        }
        return defaultValue;
    }

    inline void SetDword(const wchar_t* valueName, DWORD value)
    {
        HKEY hKey = nullptr;
        if (RegCreateKeyExW(HKEY_CURRENT_USER, REG_KEY, 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) == ERROR_SUCCESS)
        {
            RegSetValueExW(hKey, valueName, 0, REG_DWORD, (const BYTE*)&value, sizeof(value));
            RegCloseKey(hKey);
        }
    }

    inline int GetTiltAngle()
    {
        int tilt = (int)GetDword(L"TiltAngle", (DWORD)DEFAULT_TILT);
        if (tilt < -27) tilt = -27;
        if (tilt > 27) tilt = 27;
        return tilt;
    }

    inline void SetTiltAngle(int angle)
    {
        if (angle < -27) angle = -27;
        if (angle > 27) angle = 27;
        SetDword(L"TiltAngle", (DWORD)angle);
    }

    inline int GetVideoMode()
    {
        DWORD mode = GetDword(L"VideoMode", (DWORD)DEFAULT_VIDEO_MODE);
        return (mode == VIDEO_MODE_IR) ? VIDEO_MODE_IR : VIDEO_MODE_RGB;
    }

    inline void SetVideoMode(int mode)
    {
        SetDword(L"VideoMode", (DWORD)mode);
    }

    inline int GetLedColor()
    {
        return (int)GetDword(L"LedColor", (DWORD)DEFAULT_LED_COLOR);
    }

    inline void SetLedColor(int color)
    {
        SetDword(L"LedColor", (DWORD)color);
    }

    inline bool GetStandbyKeepOff()
    {
        return GetDword(L"StandbyKeepOff", (DWORD)DEFAULT_STANDBY_OFF) != 0;
    }

    inline void SetStandbyKeepOff(bool enable)
    {
        SetDword(L"StandbyKeepOff", enable ? 1 : 0);
    }
}
