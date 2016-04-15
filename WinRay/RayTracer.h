#ifndef RayTracer_h__
#define RayTracer_h__

#include <iostream>
#include <fstream>
#include <limits>
#include <tchar.h>
#include <cmath>

#include "Scene.h"

//=========================================================================
// The main ray tracer class, also maintains the frame buffer
//=========================================================================
class RayTracer
{
public:
    RayTracer();
    virtual ~RayTracer();

    //=========================================================================
    // Public Functions
    //=========================================================================
    int InitRayTracer(HWND hWnd);
    bool Render();
    void Save(LPCTSTR FileName) const;
    void PresentFrameBuffer() const;
    void Clear() const;
    bool LoadScene(LPCTSTR DefFile);
    bool IsLoaded() const;

    u_long GetWidth() const
    {
        return m_nViewWidth;
    }

    u_long GetHeight() const
    {
        return m_nViewHeight;
    }

private:
    //=========================================================================
    // Private Functions
    //=========================================================================
    void ClearFrameBuffer(u_long Color) const;
    bool BuildFrameBuffer(u_long Width, u_long Height);

    float sRGB(float c)
    {
        // Gamma correction function
        if (c <= 0.0031308f)
        {
            return 12.92f * c;
        }

        return 1.055f * powf(c, 0.4166667f) - 0.055f;
    }

    // Private member variables
    HWND m_hWnd; // Window handle
    HDC m_hdcFrameBuffer; // Frame buffers DC
    HBITMAP m_hbmFrameBuffer; // Frame buffers bitmap
    HBITMAP m_hbmSelectOut; // Used for selecting out the DC
    BITMAPINFO m_Info; // Used to describe the device data area
    u_long m_nViewWidth; // Width of viewport
    u_long m_nViewHeight; // Height of viewport
    u_long* m_pFrameBuffer; // Actual frame buffer data area

    float m_fRenderTime; // Render time

    Scene m_Scene; // Scene data
};

#endif // !RayTracer_h__
