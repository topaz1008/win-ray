#include "RayTracer.h"

RayTracer::RayTracer()
{
    // Clear all required values
    m_hWnd = NULL;
    m_hdcFrameBuffer = NULL;
    m_hbmSelectOut = NULL;
    m_hbmFrameBuffer = NULL;
    m_pFrameBuffer = NULL;
    m_nViewWidth = 0;
    m_nViewHeight = 0;
    m_fRenderTime = 0;
}

RayTracer::~RayTracer()
{
    // Destroy the frame buffer and DC
    if (m_hdcFrameBuffer && m_hbmFrameBuffer)
    {
        SelectObject(m_hdcFrameBuffer, m_hbmSelectOut);
        DeleteObject(m_hbmFrameBuffer);
        DeleteDC(m_hdcFrameBuffer);
    }

    m_hWnd = NULL;
    m_hbmFrameBuffer = NULL;
    m_hdcFrameBuffer = NULL;
    
    m_Scene.Reset();
}

int RayTracer::InitRayTracer(HWND hWnd)
{
    if (!IsLoaded())
    {
        return -1;
    }

    // Store values
    m_hWnd = hWnd;
    m_nViewWidth = m_Scene.Width;
    m_nViewHeight = m_Scene.Height;

    // Build the frame buffer
    if (!BuildFrameBuffer(m_nViewWidth, m_nViewHeight))
        return -1;

    // Clear the frame buffer initially
    ClearFrameBuffer(0x00FFFFFF);

    return 0;
}

bool RayTracer::BuildFrameBuffer(u_long Width, u_long Height)
{
    // Get the window DC
    HDC hDC = GetDC(m_hWnd);

    // Create frame buffers DC if not already existent
    if (!m_hdcFrameBuffer) m_hdcFrameBuffer = CreateCompatibleDC(hDC);

    // If an old frame buffer exist delete it
    if (m_hbmFrameBuffer)
    {
        // Select and destroy it
        SelectObject(m_hdcFrameBuffer, m_hbmSelectOut);
        DeleteObject(m_hbmFrameBuffer);
        m_hbmFrameBuffer = NULL;
        m_hbmSelectOut = NULL;
    }

    // BITMAPINFO structure
    ZeroMemory( &m_Info, sizeof(BITMAPINFO) );
    m_Info.bmiHeader.biSize = 40;
    m_Info.bmiHeader.biWidth = (int)Width;
    m_Info.bmiHeader.biHeight = (int)Height;
    m_Info.bmiHeader.biPlanes = 1;
    m_Info.bmiHeader.biBitCount = 32;
    m_Info.bmiHeader.biSizeImage = Width * Height;

    // Create the frame buffer as a DIB section
    m_hbmFrameBuffer = CreateDIBSection(hDC, &m_Info, DIB_RGB_COLORS, (void**)&m_pFrameBuffer, NULL, 0);
    if (!m_hbmFrameBuffer) return false;

    // Select it into the DC
    m_hbmSelectOut = (HBITMAP)SelectObject(m_hdcFrameBuffer, m_hbmFrameBuffer);

    // Release window DC
    ReleaseDC(m_hWnd, hDC);

    // Set DC background mode
    SetBkMode(m_hdcFrameBuffer, TRANSPARENT);

    return true;
}

void RayTracer::ClearFrameBuffer(u_long Color) const
{
    LOGBRUSH logBrush;
    HBRUSH hBrush = NULL, hOldBrush = NULL;

    // Set up the brush
    logBrush.lbStyle = BS_SOLID;

    // Set up the color converted to BGR & stripped of alpha
    logBrush.lbColor = 0x00FFFFFF & RGB2BGR( Color );

    // Create the brush
    hBrush = CreateBrushIndirect(&logBrush);
    if (!hBrush) return;

    // Select it
    hOldBrush = (HBRUSH)SelectObject(m_hdcFrameBuffer, hBrush);

    // Draw the rectangle
    Rectangle(m_hdcFrameBuffer, 0, 0, m_nViewWidth, m_nViewHeight);

    // Destroy the brush
    SelectObject(m_hdcFrameBuffer, hOldBrush);
    DeleteObject(hBrush);
}

void RayTracer::PresentFrameBuffer() const
{
    // Get the DC
    HDC hDC = GetDC(m_hWnd);

    // Blit the frame buffer to the client area
    BitBlt(hDC, 0, 0, m_nViewWidth, m_nViewHeight, m_hdcFrameBuffer, 0, 0, SRCCOPY);
    ReleaseDC(m_hWnd, hDC);
}

void RayTracer::Clear() const
{
    ClearFrameBuffer(0x00FFFFFF);
    PresentFrameBuffer();
}

bool RayTracer::LoadScene(LPCTSTR DefFile)
{
    return m_Scene.Load(DefFile);
}

bool RayTracer::IsLoaded() const
{
    return m_Scene.m_bLoaded;
}

bool RayTracer::Render()
{
    static TCHAR Buffer[512];

    // Bail if there is no scene loaded
    if (!m_Scene.m_bLoaded) return false;

    SetWindowText(m_hWnd, "WinRay - Rendering... (ESC To Stop)");
    u_long nStartTime = timeGetTime();

    if (m_Scene.m_GridSize == 0.0f) m_Scene.m_GridSize = 2.0f;

    // Projection distance
    float invProjDist = 1.0f / (0.5f * m_Scene.Width / tanf(0.5f * PI_OVER180 * m_Scene.m_FOV));
    float invfWeight = 1.0f / (m_Scene.m_GridSize * m_Scene.m_GridSize);
    float PixelOffset = m_Scene.m_GridSize / 2.0f;

#pragma omp parallel for schedule(dynamic)
    // For each pixel
    for (int y = 0; y < m_Scene.Height; ++y)
    {
        u_long* pBits = m_pFrameBuffer;
        for (int x = 0; x < m_Scene.Width; ++x)
        {
            float Red = 0, Green = 0, Blue = 0;

            // Divide each pixel to its respective sub-pixels
            for (float frX = (float)x; frX < (x + PixelOffset); frX += 0.5f)
            {
                for (float frY = (float)y; frY < (y + PixelOffset); frY += 0.5f)
                {
                    float Rho = invfWeight;
                    int CurDepth = 0;

                    // Conic ray projection
                    Ray ViewRay;
                    ViewRay.Origin = Vec3((float)(0.5f * m_Scene.Width) + m_Scene.m_EyePt.x,
                                          (float)(0.5f * m_Scene.Height) + m_Scene.m_EyePt.y, m_Scene.m_EyePt.z);
                    ViewRay.Dir = Vec3((float)((frX + m_Scene.m_LookAt.x) - 0.5f * m_Scene.Width) * invProjDist,
                                       (float)((frY + m_Scene.m_LookAt.y) - 0.5f * m_Scene.Height) * invProjDist, 1.0f + m_Scene.m_LookAt.z);
                    ViewRay.Dir.NormalizeIP();

                    // Start shooting rays
                    do
                    {
                        float t = FLT_MAX; // t = infinity
                        GeometricObject* CurrentObject = nullptr;

                        // Check for ray intersection with any objects
                        for (u_int i = 0; i < m_Scene.m_vecObjects.size(); ++i)
                        {
                            if (m_Scene.m_vecObjects[i]->Hit(ViewRay, t))
                                CurrentObject = m_Scene.m_vecObjects[i];
                        }

                        // If nothing is hit use background and bail
                        if (CurrentObject == nullptr)
                        {
                            Red += Rho * m_Scene.m_Background.Red;
                            Green += Rho * m_Scene.m_Background.Green;
                            Blue += Rho * m_Scene.m_Background.Blue;
                            break;
                        }

                        // Find intersection point and normal
                        Vec3 newStart = ViewRay.Origin + t * ViewRay.Dir;
                        Vec3 n = CurrentObject->GetNormal(newStart);
                        n.NormalizeIP();

                        // Get the material for the object hit
                        Material* CurMaterial = CurrentObject->GetMaterial();

                        // Cast shadow feelers for all lights
                        Ray LightRay;
                        LightRay.Origin = newStart;

                        for (u_int j = 0; j < m_Scene.m_vecLights.size(); ++j)
                        {
                            Light CurLight = m_Scene.m_vecLights[j];

                            // Trace the ray from the intersection point
                            // back to the light
                            LightRay.Dir = CurLight.Position - newStart;
                            float fLightProj = LightRay.Dir * n;

                            if (fLightProj <= 0.0f) continue;

                            float LightDist = LightRay.Dir.Norm();
                            if (LightDist == 0.0f) continue;
                            fLightProj /= LightDist;
                            LightRay.Dir.NormalizeIP();

                            // Check if the light ray is intersecting with anything
                            // but only between the origin and the light
                            bool inShadow = false;
                            float t = LightDist;
                            for (u_int i = 0; i < m_Scene.m_vecObjects.size(); ++i)
                            {
                                if (m_Scene.m_vecObjects[i]->Hit(LightRay, t))
                                {
                                    inShadow = true;
                                    break;
                                }
                            }

                            if (!inShadow)
                            {
                                // Calculate lighting
                                // Lambert diffuse
                                float Lambert = (LightRay.Dir * n) * Rho;
                                Red += Lambert * CurLight.Red * CurMaterial->Red;
                                Green += Lambert * CurLight.Green * CurMaterial->Green;
                                Blue += Lambert * CurLight.Blue * CurMaterial->Blue;

                                // Blinn-Phong specular
                                float fViewProj = ViewRay.Dir * n;
                                Vec3 BlinnDir = LightRay.Dir - ViewRay.Dir; // Half way vector
                                float NormSq = BlinnDir * BlinnDir;
                                if (NormSq != 0.0f)
                                {
                                    float Blinn = (1.0f / sqrtf(NormSq)) * MAX(fLightProj - fViewProj, 0.0f);
                                    Blinn = Rho * powf(Blinn, (float)CurMaterial->Power);
                                    Red += Blinn * CurLight.Red * CurMaterial->Specular.Red;
                                    Green += Blinn * CurLight.Green * CurMaterial->Specular.Green;
                                    Blue += Blinn * CurLight.Blue * CurMaterial->Specular.Blue;
                                }
                            }
                        }

                        // Calculate reflection vector
                        Rho *= CurMaterial->Reflection;
                        float Reflect = -2.0f * (ViewRay.Dir * n);
                        ViewRay.Origin = newStart;
                        ViewRay.Dir = ViewRay.Dir + Reflect * n;
                        CurDepth++;
                    }
                    while (Rho > 0.0f && CurDepth <= m_Scene.m_TraceDepth);

                    // Scale colors by the ray weight
                    Red += Red * invfWeight;
                    Green += Green * invfWeight;
                    Blue += Blue * invfWeight;
                }
            }

            // Exponential Exposure Control
            float Exposure = m_Scene.m_EV * 0.25f;
            Red = 1.0f - expf(Red * Exposure);
            Green = 1.0f - expf(Green * Exposure);
            Blue = 1.0f - expf(Blue * Exposure);

            // Gamma correction
            if (m_Scene.m_sRGB)
            {
                Red = sRGB(Red);
                Green = sRGB(Green);
                Blue = sRGB(Blue);
            }

            // Clamp and write final pixel colors mapped to 0-255
            pBits[x + y * m_Info.bmiHeader.biWidth] = RGB_COLOR(
                (u_char)MIN(Red * 255.0f, 255.0f),
                (u_char)MIN(Green * 255.0f, 255.0f),
                (u_char)MIN(Blue * 255.0f, 255.0f)
            );
        }

        // Update window every 4 lines
        if ((y % 4) == 0)
            PresentFrameBuffer();

        // Check for render termination
        //if (GetAsyncKeyState(VK_ESCAPE) & 0x00FF) break; // Can't be used with omp multi threading
    }

    // Blit the frame buffer
    PresentFrameBuffer();

    u_long nEndTime = timeGetTime();
    m_fRenderTime = (nEndTime - nStartTime) / 1000.0f;
    sprintf(Buffer, "WinRay - Render Time: %g sec", m_fRenderTime);
    SetWindowText(m_hWnd, Buffer);

    // Finished
    return true;
}

void RayTracer::Save(LPCTSTR FileName) const
{
    if (!m_Scene.m_bLoaded) return;

    // Open a file stream for binary write
    std::ofstream outFile(FileName, std::ios_base::binary);
    if (!outFile) return;

    /*typedef struct
    {
        byte  identsize;       // size of ID field that follows 18 byte header (0 usually)
        byte  colourmaptype;   // type of color map 0=none, 1=has palette
        byte  imagetype;       // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

        short colourmapstart;  // first color map entry in palette
        short colourmaplength; // number of colors in palette
        byte  colourmapbits;   // number of bits per palette entry 15,16,24,32

        short xstart;          // image x origin
        short ystart;          // image y origin
        short width;           // image width in pixels
        short height;          // image height in pixels
        byte  bits;            // image bits per pixel 8,16,24,32
        byte  descriptor;      // image descriptor bits (vh flip bits)
        // pixel data follows header
    } TGA_HEADER;*/

    // Start TGA header
    outFile.put(0);
    outFile.put(0);
    outFile.put(2); // RGB

    outFile.put(0).put(0);
    outFile.put(0).put(0);
    outFile.put(0);

    outFile.put(0).put(0); // X origin
    outFile.put(0).put(0); // Y origin
    outFile.put((u_char)(m_Scene.Width & 0x00FF)).put((u_char)((m_Scene.Width & 0xFF00) / 256));
    outFile.put((u_char)(m_Scene.Height & 0x00FF)).put((u_char)((m_Scene.Height & 0xFF00) / 256));
    outFile.put(32); // 32bit bitmap
    outFile.put(0);
    // End TGA header

    // Write actual pixel data and close the file
    outFile.write((char*)m_pFrameBuffer, m_Scene.Width * m_Scene.Height * 4);
    outFile.close();
}
