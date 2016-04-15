#include "Scene.h"
#include "Sphere.h"
#include "Plane.h"

Scene::~Scene()
{
    // Call Reset()
    Reset();
}

bool Scene::Load(LPCTSTR DefFile)
{
    char Buffer[1024], Section[128];
    u_int i, SphereCount, PlaneCount, MaterialCount, LightCount;

    // Load general scene info
    strcpy(Section, "Scene");
    Width = GetPrivateProfileInt(Section, "ImageWidth", 640, DefFile);
    Height = GetPrivateProfileInt(Section, "ImageHeight", 480, DefFile);

    SphereCount = GetPrivateProfileInt(Section, "SphereNum", 0, DefFile);
    PlaneCount = GetPrivateProfileInt(Section, "PlaneNum", 0, DefFile);
    MaterialCount = GetPrivateProfileInt(Section, "MaterialNum", 0, DefFile);
    LightCount = GetPrivateProfileInt(Section, "LightsNum", 0, DefFile);
    m_TraceDepth = GetPrivateProfileInt(Section, "TraceDepth", 0, DefFile);

    GetPrivateProfileString(Section, "Background", "0, 0, 0", Buffer, 1024, DefFile);
    sscanf(Buffer, "%g,%g,%g", &m_Background.Red, &m_Background.Green, &m_Background.Blue);
    GetPrivateProfileString(Section, "sRGB", "false", Buffer, 1024, DefFile);
    if (strcmp(Buffer, "true") == 0)
        m_sRGB = true;
    else
        m_sRGB = false;

    // Load Sampling info
    strcpy(Section, "Image Sampling");
    GetPrivateProfileString(Section, "GridSize", "2.0", Buffer, 1024, DefFile);
    sscanf(Buffer, "%g", &m_GridSize);

    // Camera info
    strcpy(Section, "Camera");
    GetPrivateProfileString(Section, "EyePoint", "0, 0, -1000", Buffer, 1024, DefFile);
    sscanf(Buffer, "%g,%g,%g", &m_EyePt.x, &m_EyePt.y, &m_EyePt.z);
    GetPrivateProfileString(Section, "LookAt", "0, 0, 0", Buffer, 1024, DefFile);
    sscanf(Buffer, "%g,%g,%g", &m_LookAt.x, &m_LookAt.y, &m_LookAt.z);
    GetPrivateProfileString(Section, "FOV", "45", Buffer, 1024, DefFile);
    sscanf(Buffer, "%g", &m_FOV);
    GetPrivateProfileString(Section, "ExposureValue", "-1.00", Buffer, 1024, DefFile);
    sscanf(Buffer, "%g", &m_EV);

    // Load materials
    for (i = 1; i <= MaterialCount; ++i)
    {
        Material* mat = new Material;
        sprintf(Section, "Material %i", i);

        GetPrivateProfileString(Section, "Diffuse", "1, 1, 1", Buffer, 1024, DefFile);
        sscanf(Buffer, "%g,%g,%g", &mat->Red, &mat->Green, &mat->Blue);
        GetPrivateProfileString(Section, "Specular", "1, 1, 1", Buffer, 1024, DefFile);
        sscanf(Buffer, "%g,%g,%g", &mat->Specular.Red, &mat->Specular.Green, &mat->Specular.Blue);
        GetPrivateProfileString(Section, "Reflection", "0.5", Buffer, 1024, DefFile);
        sscanf(Buffer, "%g", &mat->Reflection);
        mat->Power = GetPrivateProfileInt(Section, "Power", 65, DefFile);

        m_vecMaterials.push_back(mat);
    }

    // Load planes (has to be loaded before the spheres)
    for (i = 1; i <= PlaneCount; ++i)
    {
        Plane* p = new Plane;
        sprintf(Section, "Plane %i", i);

        GetPrivateProfileString(Section, "Normal", "0, 0, 0", Buffer, 1024, DefFile);
        sscanf(Buffer, "%g,%g,%g", &p->Normal.x, &p->Normal.y, &p->Normal.z);

        GetPrivateProfileString(Section, "Point", "0, 0, 0", Buffer, 1024, DefFile);
        sscanf(Buffer, "%g,%g,%g", &p->Point.x, &p->Point.y, &p->Point.z);

        u_int MaterialID = GetPrivateProfileInt(Section, "MaterialID", 0, DefFile);

        p->SetMaterial(m_vecMaterials[MaterialID]);

        m_vecObjects.push_back(p);
    }

    // Load spheres
    for (i = 1; i <= SphereCount; ++i)
    {
        Sphere* sph = new Sphere;
        sprintf(Section, "Sphere %i", i);

        GetPrivateProfileString(Section, "Position", "0, 0, 0", Buffer, 1024, DefFile);
        sscanf(Buffer, "%g,%g,%g", &sph->Position.x, &sph->Position.y, &sph->Position.z);
        GetPrivateProfileString(Section, "Radius", "50", Buffer, 1024, DefFile);
        sscanf(Buffer, "%g", &sph->Radius);
        u_int MaterialID = GetPrivateProfileInt(Section, "MaterialID", 0, DefFile);

        sph->SetMaterial(m_vecMaterials[MaterialID]);

        m_vecObjects.push_back(sph);
    }

    // Load lights
    for (i = 1; i <= LightCount; ++i)
    {
        Light light;
        sprintf(Section, "Light %i", i);

        GetPrivateProfileString(Section, "Position", "0, 0, 0", Buffer, 1024, DefFile);
        sscanf(Buffer, "%g,%g,%g", &light.Position.x, &light.Position.y, &light.Position.z);
        GetPrivateProfileString(Section, "Diffuse", "1, 1, 1", Buffer, 1024, DefFile);
        sscanf(Buffer, "%g,%g,%g", &light.Red, &light.Green, &light.Blue);

        m_vecLights.push_back(light);
    }

    // Change loaded state
    m_bLoaded = true;

    // Finished
    return true;
}

void Scene::Reset()
{
    // Reset all info only if something is loaded
    if (m_bLoaded)
    {
        Width = 0;
        Height = 0;
        m_TraceDepth = 0;
        m_FOV = 45.0f;
        m_EV = -1.00f;
        m_sRGB = false;
        m_Background = Color(0, 0, 0);
        m_vecLights.clear();

        m_bLoaded = false;

        for (u_int i = 0; i < m_vecObjects.size(); i++)
        {
            if (m_vecObjects[i] != nullptr)
            {
                delete m_vecObjects[i];
                m_vecObjects[i] = nullptr;
            }
        }

        m_vecObjects.clear();

        // Cleanup is done here as we might hold the same material pointer for several objects.
        for (u_int i = 0; i < m_vecMaterials.size(); i++)
        {
            if (m_vecMaterials[i] != nullptr)
            {
                delete m_vecMaterials[i];
                m_vecMaterials[i] = nullptr;
            }
        }

        m_vecMaterials.clear();
    }
}
