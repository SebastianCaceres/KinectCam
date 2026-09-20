#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <libfreenect.h>
#include <thread>
#include <mutex>
#include <atomic>

class KinectRGBCam
{
public:
    HRESULT CreateFirstConnected();
    void Nui_GetCamFrame(BYTE *frameBuffer, int frameSize);
    void Nui_UnInit();

    KinectRGBCam();
    ~KinectRGBCam();
    KinectRGBCam(const KinectRGBCam&) = delete;
    KinectRGBCam& operator=(const KinectRGBCam&) = delete;

private:
    static void VideoCallback(freenect_device* dev, void* video, uint32_t timestamp);
    static void ThreadWorker();

    static freenect_context* m_fContext;
    static freenect_device* m_fDevice;
    static std::thread m_workerThread;
    static std::atomic<bool> m_running;
    static std::mutex m_frameMutex;
    static BYTE m_frontBuffer[640 * 480 * 4];
    static bool m_hasNewFrame;
};