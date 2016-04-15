#ifndef Scene_h__
#define Scene_h__

#include <Windows.h>
#include <vector>

#include "Common.h"
#include "GeometricObject.h"

//==========================================================
// Loads and stores scene information from an ini file
//==========================================================
class Scene
{
public:
    Scene() : m_bLoaded(false), Width(0), Height(0), m_TraceDepth(0), m_FOV(45.0f),
              m_EV(-1.0f), m_sRGB(false), m_GridSize(2.0f) {}

    ~Scene();

    // Public functions
    bool Load(LPCTSTR DefFile);
    void Reset();

    bool m_bLoaded; // Loaded state

    // Scene data
    int Width, Height;
    int m_TraceDepth;
    float m_FOV;
    float m_EV;
    bool m_sRGB;
    float m_GridSize;
    Color m_Background;
    Vec3 m_EyePt;
    Vec3 m_LookAt;
    std::vector<GeometricObject*> m_vecObjects;
    std::vector<Material*> m_vecMaterials;
    std::vector<Light> m_vecLights;
};

#endif // !Scene_h__
