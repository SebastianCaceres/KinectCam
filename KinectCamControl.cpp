#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <libfreenect.h>
#include "KinectSettings.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")

#define WM_TRAYICON (WM_USER + 101)
#define ID_TRAY_RESTORE     2001
#define ID_TRAY_RESET_TILT  2002
#define ID_TRAY_MODE_RGB    2003
#define ID_TRAY_MODE_IR     2004
#define ID_TRAY_LED_OFF     2005
#define ID_TRAY_LED_GREEN   2006
#define ID_TRAY_EXIT        2007

#define IDC_SLIDER_TILT     1001
#define IDC_LABEL_TILT_VAL  1002
#define IDC_BTN_RESET_TILT  1003
#define IDC_RADIO_RGB       1004
#define IDC_RADIO_IR        1005
#define IDC_COMBO_LED       1006
#define IDC_CHK_STANDBY_OFF 1007
#define IDC_CHK_STARTUP     1008
#define IDC_BTN_MINIMIZE    1009
#define IDC_STATUS_LABEL    1010

#define TIMER_KEEP_ALIVE    1
#define TIMER_RECONNECT     2

static HINSTANCE g_hInstance = nullptr;
static HWND g_hMainWnd = nullptr;
static NOTIFYICONDATAW g_nid = { 0 };

static HWND g_hSliderTilt = nullptr;
static HWND g_hLblTiltVal = nullptr;
static HWND g_hRadioRgb = nullptr;
static HWND g_hRadioIr = nullptr;
static HWND g_hComboLed = nullptr;
static HWND g_hChkStandby = nullptr;
static HWND g_hChkStartup = nullptr;
static HWND g_hLblStatus = nullptr;

static freenect_context* g_fContext = nullptr;
static freenect_device* g_fDevice = nullptr;
static bool g_deviceConnected = false;
static int g_currentTilt = 0;
static int g_currentLed = 1; // Green

// Helper for Windows Startup Registry Key
static const wchar_t* RUN_KEY = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const wchar_t* APP_NAME = L"KinectCamControl";

static bool IsRunOnStartup()
{
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, RUN_KEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        wchar_t path[MAX_PATH];
        DWORD size = sizeof(path);
        DWORD type = REG_SZ;
        LONG res = RegQueryValueExW(hKey, APP_NAME, nullptr, &type, (LPBYTE)path, &size);
        RegCloseKey(hKey);
        return (res == ERROR_SUCCESS);
    }
    return false;
}

static void SetRunOnStartup(bool enable)
{
    HKEY hKey;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, RUN_KEY, 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) == ERROR_SUCCESS)
    {
        if (enable)
        {
            wchar_t exePath[MAX_PATH];
            GetModuleFileNameW(nullptr, exePath, MAX_PATH);
            std::wstring cmd = L"\"" + std::wstring(exePath) + L"\" --minimized";
            RegSetValueExW(hKey, APP_NAME, 0, REG_SZ, (const BYTE*)cmd.c_str(), (DWORD)((cmd.length() + 1) * sizeof(wchar_t)));
        }
        else
        {
            RegDeleteValueW(hKey, APP_NAME);
        }
        RegCloseKey(hKey);
    }
}

// Hardware motor/LED functions
static bool ConnectKinect()
{
    if (g_fDevice) return true;

    if (!g_fContext)
    {
        if (freenect_init(&g_fContext, nullptr) < 0)
        {
            return false;
        }
    }

    // Open only MOTOR subdevice for tilt & LED control
    freenect_select_subdevices(g_fContext, FREENECT_DEVICE_MOTOR);
    if (freenect_num_devices(g_fContext) <= 0)
    {
        return false;
    }

    if (freenect_open_device(g_fContext, &g_fDevice, 0) < 0)
    {
        g_fDevice = nullptr;
        return false;
    }

    g_deviceConnected = true;

    // Apply saved settings
    g_currentTilt = KinectSettings::GetTiltAngle();
    g_currentLed = KinectSettings::GetLedColor();
    freenect_set_tilt_degs(g_fDevice, (double)g_currentTilt);
    freenect_set_led(g_fDevice, (freenect_led_options)g_currentLed);

    return true;
}

static void DisconnectKinect()
{
    if (g_fDevice)
    {
        if (KinectSettings::GetStandbyKeepOff())
        {
            freenect_set_led(g_fDevice, LED_OFF);
        }
        freenect_close_device(g_fDevice);
        g_fDevice = nullptr;
    }
    if (g_fContext)
    {
        freenect_shutdown(g_fContext);
        g_fContext = nullptr;
    }
    g_deviceConnected = false;
}

static void ApplyTilt(int angle)
{
    if (angle < -27) angle = -27;
    if (angle > 27) angle = 27;
    g_currentTilt = angle;
    KinectSettings::SetTiltAngle(angle);

    if (g_fDevice)
    {
        freenect_set_tilt_degs(g_fDevice, (double)angle);
    }

    if (g_hLblTiltVal)
    {
        wchar_t buf[32];
        wsprintfW(buf, L"%+d\u00B0", angle);
        SetWindowTextW(g_hLblTiltVal, buf);
    }
}

static void ApplyLed(int led)
{
    g_currentLed = led;
    KinectSettings::SetLedColor(led);

    if (g_fDevice)
    {
        freenect_set_led(g_fDevice, (freenect_led_options)led);
    }
}

static void UpdateStatusText()
{
    if (!g_hLblStatus) return;
    if (g_deviceConnected)
    {
        SetWindowTextW(g_hLblStatus, L"Status: Kinect Connected (Motor & LED Active)");
    }
    else
    {
        SetWindowTextW(g_hLblStatus, L"Status: Searching for Kinect (USB Motor)...");
    }
}

static void ShowTrayMenu(HWND hWnd)
{
    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreatePopupMenu();
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_RESTORE, L"Open Control Panel");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_RESET_TILT, L"Reset Tilt to 0\u00B0");

    int mode = KinectSettings::GetVideoMode();
    AppendMenuW(hMenu, MF_STRING | (mode == KinectSettings::VIDEO_MODE_RGB ? MF_CHECKED : 0), ID_TRAY_MODE_RGB, L"Mode: RGB Color");
    AppendMenuW(hMenu, MF_STRING | (mode == KinectSettings::VIDEO_MODE_IR ? MF_CHECKED : 0), ID_TRAY_MODE_IR, L"Mode: Infrared (Night Vision)");

    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_LED_OFF, L"Set LED: Off");
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_LED_GREEN, L"Set LED: Solid Green");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, L"Exit");

    SetForegroundWindow(hWnd);
    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, nullptr);
    DestroyMenu(hMenu);
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        // Group 1: Motor Tilt
        HWND hGrpTilt = CreateWindowW(L"BUTTON", L"Motor Tilt Angle", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            15, 10, 340, 95, hWnd, nullptr, g_hInstance, nullptr);
        SendMessageW(hGrpTilt, WM_SETFONT, (WPARAM)hFont, TRUE);

        g_hSliderTilt = CreateWindowW(TRACKBAR_CLASSW, L"", WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
            30, 35, 230, 30, hWnd, (HMENU)IDC_SLIDER_TILT, g_hInstance, nullptr);
        SendMessageW(g_hSliderTilt, TBM_SETRANGE, TRUE, MAKELPARAM(0, 54)); // 0..54 maps to -27..+27
        SendMessageW(g_hSliderTilt, TBM_SETPOS, TRUE, (LPARAM)(g_currentTilt + 27));

        g_hLblTiltVal = CreateWindowW(L"STATIC", L"0\u00B0", WS_CHILD | WS_VISIBLE | SS_CENTER,
            270, 35, 55, 25, hWnd, (HMENU)IDC_LABEL_TILT_VAL, g_hInstance, nullptr);
        SendMessageW(g_hLblTiltVal, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND hBtnReset = CreateWindowW(L"BUTTON", L"Reset to 0\u00B0", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            30, 68, 100, 26, hWnd, (HMENU)IDC_BTN_RESET_TILT, g_hInstance, nullptr);
        SendMessageW(hBtnReset, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Group 2: Sensor Mode
        HWND hGrpMode = CreateWindowW(L"BUTTON", L"Camera Sensor Mode", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            15, 115, 340, 65, hWnd, nullptr, g_hInstance, nullptr);
        SendMessageW(hGrpMode, WM_SETFONT, (WPARAM)hFont, TRUE);

        g_hRadioRgb = CreateWindowW(L"BUTTON", L"RGB Color Camera", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
            30, 140, 140, 25, hWnd, (HMENU)IDC_RADIO_RGB, g_hInstance, nullptr);
        SendMessageW(g_hRadioRgb, WM_SETFONT, (WPARAM)hFont, TRUE);

        g_hRadioIr = CreateWindowW(L"BUTTON", L"Infrared (Night Vision)", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
            180, 140, 160, 25, hWnd, (HMENU)IDC_RADIO_IR, g_hInstance, nullptr);
        SendMessageW(g_hRadioIr, WM_SETFONT, (WPARAM)hFont, TRUE);

        int curMode = KinectSettings::GetVideoMode();
        SendMessageW(curMode == KinectSettings::VIDEO_MODE_IR ? g_hRadioIr : g_hRadioRgb, BM_SETCHECK, BST_CHECKED, 0);

        // Group 3: LED Status
        HWND hGrpLed = CreateWindowW(L"BUTTON", L"Kinect LED Status", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            15, 190, 340, 95, hWnd, nullptr, g_hInstance, nullptr);
        SendMessageW(hGrpLed, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND hLblLed = CreateWindowW(L"STATIC", L"Active Color:", WS_CHILD | WS_VISIBLE,
            30, 215, 80, 22, hWnd, nullptr, g_hInstance, nullptr);
        SendMessageW(hLblLed, WM_SETFONT, (WPARAM)hFont, TRUE);

        g_hComboLed = CreateWindowW(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
            120, 212, 210, 160, hWnd, (HMENU)IDC_COMBO_LED, g_hInstance, nullptr);
        SendMessageW(g_hComboLed, WM_SETFONT, (WPARAM)hFont, TRUE);

        SendMessageW(g_hComboLed, CB_ADDSTRING, 0, (LPARAM)L"Off (No Light)");            // 0
        SendMessageW(g_hComboLed, CB_ADDSTRING, 0, (LPARAM)L"Solid Green (Active)");       // 1
        SendMessageW(g_hComboLed, CB_ADDSTRING, 0, (LPARAM)L"Solid Red (Recording)");      // 2
        SendMessageW(g_hComboLed, CB_ADDSTRING, 0, (LPARAM)L"Solid Yellow");               // 3
        SendMessageW(g_hComboLed, CB_ADDSTRING, 0, (LPARAM)L"Blinking Green (Standby)");   // 4
        SendMessageW(g_hComboLed, CB_ADDSTRING, 0, (LPARAM)L"Blinking Red / Yellow");      // 5 -> maps to 6

        int ledIdx = g_currentLed;
        if (ledIdx >= 0 && ledIdx <= 4)
            SendMessageW(g_hComboLed, CB_SETCURSEL, ledIdx, 0);
        else if (ledIdx == 6)
            SendMessageW(g_hComboLed, CB_SETCURSEL, 5, 0);
        else
            SendMessageW(g_hComboLed, CB_SETCURSEL, 1, 0);

        g_hChkStandby = CreateWindowW(L"BUTTON", L"Keep LED completely Off when idle on standby",
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 30, 250, 310, 22, hWnd, (HMENU)IDC_CHK_STANDBY_OFF, g_hInstance, nullptr);
        SendMessageW(g_hChkStandby, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessageW(g_hChkStandby, BM_SETCHECK, KinectSettings::GetStandbyKeepOff() ? BST_CHECKED : BST_UNCHECKED, 0);

        // Preferences
        g_hChkStartup = CreateWindowW(L"BUTTON", L"Start automatically with Windows",
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 30, 295, 250, 22, hWnd, (HMENU)IDC_CHK_STARTUP, g_hInstance, nullptr);
        SendMessageW(g_hChkStartup, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessageW(g_hChkStartup, BM_SETCHECK, IsRunOnStartup() ? BST_CHECKED : BST_UNCHECKED, 0);

        HWND hBtnHide = CreateWindowW(L"BUTTON", L"Hide to Tray", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            245, 325, 110, 28, hWnd, (HMENU)IDC_BTN_MINIMIZE, g_hInstance, nullptr);
        SendMessageW(hBtnHide, WM_SETFONT, (WPARAM)hFont, TRUE);

        g_hLblStatus = CreateWindowW(L"STATIC", L"Status: Initializing...", WS_CHILD | WS_VISIBLE,
            15, 332, 220, 20, hWnd, (HMENU)IDC_STATUS_LABEL, g_hInstance, nullptr);
        SendMessageW(g_hLblStatus, WM_SETFONT, (WPARAM)hFont, TRUE);

        // System Tray Setup
        g_nid.cbSize = sizeof(NOTIFYICONDATAW);
        g_nid.hWnd = hWnd;
        g_nid.uID = 1;
        g_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
        g_nid.uCallbackMessage = WM_TRAYICON;
        g_nid.hIcon = LoadIconW(nullptr, (LPCWSTR)IDI_APPLICATION);
        lstrcpyW(g_nid.szTip, L"Kinect Virtual Camera Control");
        Shell_NotifyIconW(NIM_ADD, &g_nid);

        // Initialize connection
        ConnectKinect();
        UpdateStatusText();
        ApplyTilt(g_currentTilt);

        // Timer for connection maintenance and LED keep-alive
        SetTimer(hWnd, TIMER_KEEP_ALIVE, 2000, nullptr);

        return 0;
    }

    case WM_HSCROLL:
    {
        if ((HWND)lParam == g_hSliderTilt)
        {
            int pos = (int)SendMessageW(g_hSliderTilt, TBM_GETPOS, 0, 0);
            int angle = pos - 27;
            ApplyTilt(angle);
        }
        return 0;
    }

    case WM_COMMAND:
    {
        WORD id = LOWORD(wParam);
        WORD code = HIWORD(wParam);

        if (id == IDC_BTN_RESET_TILT)
        {
            SendMessageW(g_hSliderTilt, TBM_SETPOS, TRUE, 27);
            ApplyTilt(0);
        }
        else if (id == IDC_RADIO_RGB && code == BN_CLICKED)
        {
            KinectSettings::SetVideoMode(KinectSettings::VIDEO_MODE_RGB);
        }
        else if (id == IDC_RADIO_IR && code == BN_CLICKED)
        {
            KinectSettings::SetVideoMode(KinectSettings::VIDEO_MODE_IR);
        }
        else if (id == IDC_COMBO_LED && code == CBN_SELCHANGE)
        {
            int sel = (int)SendMessageW(g_hComboLed, CB_GETCURSEL, 0, 0);
            int ledVal = sel;
            if (sel == 5) ledVal = 6; // BLINK_RED_YELLOW
            ApplyLed(ledVal);
        }
        else if (id == IDC_CHK_STANDBY_OFF && code == BN_CLICKED)
        {
            bool chk = (SendMessageW(g_hChkStandby, BM_GETCHECK, 0, 0) == BST_CHECKED);
            KinectSettings::SetStandbyKeepOff(chk);
            if (chk) ApplyLed(LED_OFF);
        }
        else if (id == IDC_CHK_STARTUP && code == BN_CLICKED)
        {
            bool chk = (SendMessageW(g_hChkStartup, BM_GETCHECK, 0, 0) == BST_CHECKED);
            SetRunOnStartup(chk);
        }
        else if (id == IDC_BTN_MINIMIZE)
        {
            ShowWindow(hWnd, SW_HIDE);
        }
        // Tray Menu Commands
        else if (id == ID_TRAY_RESTORE)
        {
            ShowWindow(hWnd, SW_SHOW);
            SetForegroundWindow(hWnd);
        }
        else if (id == ID_TRAY_RESET_TILT)
        {
            SendMessageW(g_hSliderTilt, TBM_SETPOS, TRUE, 27);
            ApplyTilt(0);
        }
        else if (id == ID_TRAY_MODE_RGB)
        {
            KinectSettings::SetVideoMode(KinectSettings::VIDEO_MODE_RGB);
            SendMessageW(g_hRadioRgb, BM_SETCHECK, BST_CHECKED, 0);
            SendMessageW(g_hRadioIr, BM_SETCHECK, BST_UNCHECKED, 0);
        }
        else if (id == ID_TRAY_MODE_IR)
        {
            KinectSettings::SetVideoMode(KinectSettings::VIDEO_MODE_IR);
            SendMessageW(g_hRadioIr, BM_SETCHECK, BST_CHECKED, 0);
            SendMessageW(g_hRadioRgb, BM_SETCHECK, BST_UNCHECKED, 0);
        }
        else if (id == ID_TRAY_LED_OFF)
        {
            ApplyLed(LED_OFF);
            SendMessageW(g_hComboLed, CB_SETCURSEL, 0, 0);
        }
        else if (id == ID_TRAY_LED_GREEN)
        {
            ApplyLed(LED_GREEN);
            SendMessageW(g_hComboLed, CB_SETCURSEL, 1, 0);
        }
        else if (id == ID_TRAY_EXIT)
        {
            DestroyWindow(hWnd);
        }
        return 0;
    }

    case WM_TIMER:
    {
        if (wParam == TIMER_KEEP_ALIVE)
        {
            if (!g_fDevice)
            {
                ConnectKinect();
                UpdateStatusText();
            }
            else
            {
                // Send keep-alive to motor so it doesn't timeout to blinking green
                if (KinectSettings::GetStandbyKeepOff() && g_currentLed == LED_OFF)
                {
                    freenect_set_led(g_fDevice, LED_OFF);
                }
            }
        }
        return 0;
    }

    case WM_TRAYICON:
    {
        if (lParam == WM_RBUTTONUP)
        {
            ShowTrayMenu(hWnd);
        }
        else if (lParam == WM_LBUTTONDBLCLK || lParam == WM_LBUTTONUP)
        {
            ShowWindow(hWnd, SW_SHOW);
            SetForegroundWindow(hWnd);
        }
        return 0;
    }

    case WM_SYSCOMMAND:
    {
        if ((wParam & 0xFFF0) == SC_MINIMIZE)
        {
            ShowWindow(hWnd, SW_HIDE);
            return 0;
        }
        break;
    }

    case WM_CLOSE:
    {
        // Closing window hides it to tray instead of quitting
        ShowWindow(hWnd, SW_HIDE);
        return 0;
    }

    case WM_DESTROY:
    {
        KillTimer(hWnd, TIMER_KEEP_ALIVE);
        Shell_NotifyIconW(NIM_DELETE, &g_nid);
        DisconnectKinect();
        PostQuitMessage(0);
        return 0;
    }
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
    g_hInstance = hInstance;

    // Check if another instance is already running
    HANDLE hMutex = CreateMutexW(nullptr, TRUE, L"KinectCamControl_SingleInstance_Mutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        HWND hExisting = FindWindowW(L"KinectCamControlWnd", nullptr);
        if (hExisting)
        {
            ShowWindow(hExisting, SW_SHOW);
            SetForegroundWindow(hExisting);
        }
        CloseHandle(hMutex);
        return 0;
    }

    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_BAR_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIconW(nullptr, (LPCWSTR)IDI_APPLICATION);
    wc.hCursor = LoadCursorW(nullptr, (LPCWSTR)IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = L"KinectCamControlWnd";
    RegisterClassExW(&wc);

    int windowWidth = 385;
    int windowHeight = 405;
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenW - windowWidth) / 2;
    int y = (screenH - windowHeight) / 2;

    g_hMainWnd = CreateWindowExW(
        WS_EX_DLGMODALFRAME,
        L"KinectCamControlWnd",
        L"Kinect Control Panel",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        x, y, windowWidth, windowHeight,
        nullptr, nullptr, hInstance, nullptr
    );

    bool startMinimized = (wcsstr(pCmdLine, L"--minimized") != nullptr);
    if (!startMinimized)
    {
        ShowWindow(g_hMainWnd, nCmdShow);
        UpdateWindow(g_hMainWnd);
    }

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    CloseHandle(hMutex);
    return (int)msg.wParam;
}
