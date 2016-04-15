#include "Sphere.h"

bool Sphere::Hit(const Ray& ray, float& t) const
{
    Vec3 Dist = Position - ray.Origin;
    float b = ray.Dir * Dist;
    float d = b * b - Dist * Dist + Radius * Radius;

    if (d < 0.0f) return false;

    // Solve for the 2 roots and save the closest one
    float fSqrt = sqrtf(d);
    float t0 = b - fSqrt;
    float t1 = b + fSqrt;
    bool Res = false;

    if (t0 > 0.1f && t0 < t)
    {
        t = t0;
        Res = true;
    }
    if (t1 > 0.1f && t1 < t)
    {
        t = t1;
        Res = true;
    }

    return Res;
}

Vec3 Sphere::GetNormal(const Vec3& hitPoint) const
{
    return hitPoint - Position;
}
