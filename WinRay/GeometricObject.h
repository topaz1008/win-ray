#ifndef GeometricObject_h__
#define GeometricObject_h__

#include "Common.h"

class GeometricObject
{
public:
    GeometricObject() : m_pMaterial(nullptr) {}
    virtual ~GeometricObject() = default;

    virtual bool Hit(const Ray& ray, float& t) const = 0;
    virtual Vec3 GetNormal(const Vec3& hitPoint) const = 0;

    Material* GetMaterial() const;
    void SetMaterial(Material* material);

protected:
    Material* m_pMaterial;
};

inline Material* GeometricObject::GetMaterial() const
{
    return m_pMaterial;
}

#endif // !GeometricObject_h__
