#ifndef Sphere_h__
#define Sphere_h__

#include "GeometricObject.h"
#include "Common.h"

class Sphere : public GeometricObject
{
public:
    Sphere() : Radius(0.0f) {};
    ~Sphere() = default;

    bool Hit(const Ray& ray, float& t) const override;
    Vec3 GetNormal(const Vec3& hitPoint) const override;

public:
    Vec3 Position;
    float Radius;
};

#endif // !Sphere_h__
