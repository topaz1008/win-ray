#ifndef Plane_h__
#define Plane_h__

#include "GeometricObject.h"
#include "Common.h"

class Plane : public GeometricObject
{
public:
    Plane() = default;
    ~Plane() = default;

    bool Hit(const Ray& ray, float& t) const override;
    Vec3 GetNormal(const Vec3& hitPoint) const override;

public:
    Vec3 Normal;
    Vec3 Point;
};

#endif // !Plane_h__
