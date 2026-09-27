#include "KinectRGBCam.h"
#include "KinectSettings.h"
#include <chrono>

freenect_context* KinectRGBCam::m_fContext = nullptr;
freenect_device* KinectRGBCam::m_fDevice = nullptr;
std::thread KinectRGBCam::m_workerThread;
std::atomic<bool> KinectRGBCam::m_running(false);
std::mutex KinectRGBCam::m_frameMutex;
BYTE KinectRGBCam::m_frontBuffer[640 * 480 * 4] = { 0 };
bool KinectRGBCam::m_hasNewFrame = false;
int KinectRGBCam::m_currentVideoMode = KinectSettings::VIDEO_MODE_RGB;

KinectRGBCam::KinectRGBCam()
{
}

KinectRGBCam::~KinectRGBCam()
{
    Nui_UnInit();
}

void KinectRGBCam::VideoCallback(freenect_device* dev, void* video, uint32_t timestamp)
{
    std::lock_guard<std::mutex> lock(m_frameMutex);

    if (m_currentVideoMode == KinectSettings::VIDEO_MODE_IR)
    {
        // 8-bit IR mode: 640x488 grayscale. Crop top 4 lines to center in 640x480.
        uint8_t* ir_src = ((uint8_t*)video) + (4 * 640);
        for (int i = 0; i < 640 * 480; ++i)
        {
            uint8_t val = ir_src[i];
            m_frontBuffer[i * 4 + 0] = val; // B
            m_frontBuffer[i * 4 + 1] = val; // G
            m_frontBuffer[i * 4 + 2] = val; // R
            m_frontBuffer[i * 4 + 3] = 255; // A
        }
    }
    else
    {
        // Standard RGB mode: 640x480 24-bit RGB
        uint8_t* src = (uint8_t*)video;
        for (int i = 0; i < 640 * 480; ++i)
        {
            m_frontBuffer[i * 4 + 0] = src[i * 3 + 2]; // B
            m_frontBuffer[i * 4 + 1] = src[i * 3 + 1]; // G
            m_frontBuffer[i * 4 + 2] = src[i * 3 + 0]; // R
            m_frontBuffer[i * 4 + 3] = 255;            // A
        }
    }

    m_hasNewFrame = true;
}

void KinectRGBCam::SwitchVideoMode(int newMode)
{
    if (!m_fDevice || newMode == m_currentVideoMode)
        return;

    m_currentVideoMode = newMode;

    freenect_stop_video(m_fDevice);

    if (newMode == KinectSettings::VIDEO_MODE_IR)
    {
        freenect_set_video_mode(m_fDevice, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_IR_8BIT));
    }
    else
    {
        freenect_set_video_mode(m_fDevice, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB));
    }

    freenect_start_video(m_fDevice);
}

void KinectRGBCam::ThreadWorker()
{
    int pollCounter = 0;
    while (m_running)
    {
        timeval tv = { 0, 10000 }; // 10ms timeout
        if (m_fContext)
        {
            freenect_process_events_timeout(m_fContext, &tv);
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        // Check for mode change requested by KinectCamControl every 100ms
        if (++pollCounter >= 10)
        {
            pollCounter = 0;
            int desiredMode = KinectSettings::GetVideoMode();
            if (desiredMode != m_currentVideoMode && m_fDevice)
            {
                SwitchVideoMode(desiredMode);
            }
        }
    }
}

HRESULT KinectRGBCam::CreateFirstConnected()
{
    if (m_fDevice != nullptr)
    {
        return S_OK;
    }

    if (freenect_init(&m_fContext, NULL) < 0)
    {
        return E_FAIL;
    }

    int count = freenect_num_devices(m_fContext);
    if (count <= 0)
    {
        freenect_shutdown(m_fContext);
        m_fContext = nullptr;
        return E_FAIL;
    }

    // Read user configuration from registry
    m_currentVideoMode = KinectSettings::GetVideoMode();

    // The virtual camera only claims the CAMERA subdevice for video streaming.
    // This allows KinectCamControl to concurrently manage the motor & LED without USB contention.
    freenect_select_subdevices(m_fContext, FREENECT_DEVICE_CAMERA);

    if (freenect_open_device(m_fContext, &m_fDevice, 0) < 0)
    {
        freenect_shutdown(m_fContext);
        m_fContext = nullptr;
        return E_FAIL;
    }

    if (m_currentVideoMode == KinectSettings::VIDEO_MODE_IR)
    {
        freenect_set_video_mode(m_fDevice, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_IR_8BIT));
    }
    else
    {
        freenect_set_video_mode(m_fDevice, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB));
    }

    freenect_set_video_callback(m_fDevice, VideoCallback);
    freenect_start_video(m_fDevice);

    m_running = true;
    m_workerThread = std::thread(ThreadWorker);

    return S_OK;
}

void KinectRGBCam::Nui_UnInit()
{
    if (m_running)
    {
        m_running = false;
        if (m_workerThread.joinable())
        {
            m_workerThread.join();
        }
    }

    if (m_fDevice)
    {
        freenect_stop_video(m_fDevice);
        freenect_close_device(m_fDevice);
        m_fDevice = nullptr;
    }

    if (m_fContext)
    {
        freenect_shutdown(m_fContext);
        m_fContext = nullptr;
    }
}

void KinectRGBCam::Nui_GetCamFrame(BYTE* frameBuffer, int frameSize)
{
    std::lock_guard<std::mutex> lock(m_frameMutex);
    memcpy(frameBuffer, m_frontBuffer, frameSize);
}