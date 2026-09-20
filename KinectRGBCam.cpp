#include "KinectRGBCam.h"
#include <chrono>

freenect_context* KinectRGBCam::m_fContext = nullptr;
freenect_device* KinectRGBCam::m_fDevice = nullptr;
std::thread KinectRGBCam::m_workerThread;
std::atomic<bool> KinectRGBCam::m_running(false);
std::mutex KinectRGBCam::m_frameMutex;
BYTE KinectRGBCam::m_frontBuffer[640 * 480 * 4] = { 0 };
bool KinectRGBCam::m_hasNewFrame = false;

KinectRGBCam::KinectRGBCam()
{
}

KinectRGBCam::~KinectRGBCam()
{
    Nui_UnInit();
}

void KinectRGBCam::VideoCallback(freenect_device* dev, void* video, uint32_t timestamp)
{
    uint8_t* src = (uint8_t*)video;
    std::lock_guard<std::mutex> lock(m_frameMutex);
    for (int i = 0; i < 640 * 480; ++i)
    {
        m_frontBuffer[i * 4 + 0] = src[i * 3 + 2]; // B
        m_frontBuffer[i * 4 + 1] = src[i * 3 + 1]; // G
        m_frontBuffer[i * 4 + 2] = src[i * 3 + 0]; // R
        m_frontBuffer[i * 4 + 3] = 255;            // A
    }
    m_hasNewFrame = true;
}

void KinectRGBCam::ThreadWorker()
{
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

    // Select subdevices for camera and motor
    freenect_select_subdevices(m_fContext, (freenect_device_flags)(FREENECT_DEVICE_MOTOR | FREENECT_DEVICE_CAMERA));

    int count = freenect_num_devices(m_fContext);
    if (count <= 0)
    {
        freenect_shutdown(m_fContext);
        m_fContext = nullptr;
        return E_FAIL;
    }

    if (freenect_open_device(m_fContext, &m_fDevice, 0) < 0)
    {
        freenect_shutdown(m_fContext);
        m_fContext = nullptr;
        return E_FAIL;
    }

    // Keep tilt level at 0 degrees
    freenect_set_tilt_degs(m_fDevice, 0);

    // Setup 640x480 RGB mode
    freenect_set_video_mode(m_fDevice, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB));
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
        freenect_set_tilt_degs(m_fDevice, 0);
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