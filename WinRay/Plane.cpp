#include "Plane.h"

bool Plane::Hit(const Ray& ray, float& t) const
{
    float t0 = (Point - ray.Origin) * Normal / (ray.Dir * Normal);

    if (t0 > 0.1f)
    {
        t = t0;

        return true;
    }

    return false;
}

Vec3 Plane::GetNormal(const Vec3& hitPoint) const
{
    return Normal;
}
