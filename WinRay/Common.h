#ifndef Common_h__
#define Common_h__

#include <cmath>

//=========================================================================
// Commonly used types, structures and macros
//=========================================================================

// Typedefs
typedef unsigned int u_int;
typedef unsigned char u_char;
typedef unsigned long u_long;

//typedef float Real;

// Constants
#define PI_OVER180 0.017453292519943f

// Math Macros
#define MAX( x, y ) ( ((x) > (y)) ? (x) : (y) )
#define MIN( x, y ) ( ((x) < (y)) ? (x) : (y) )

// Color Macros
#define RGB_COLOR( r, g, b ) ( (u_long)((((0xFF) & 0xFF) << 24 ) | (((r) & 0xFF) << 16) | (((g) & 0xFF) << 8) | ((b) & 0xFF)) )
#define RGB2BGR( c )		 ( ((c) & 0xFF000000) | (((c) & 0xFF0000) >> 16) | ((c) & 0x00FF00) | (((c) & 0x0000FF) << 16) )

// Misc Macros
#define SAFE_DELETE( p )	  { if( (p) ) { delete   (p); (p) = NULL; } }
#define SAFE_DELETEARRAY( p ) { if( (p) ) { delete[] (p); (p) = NULL; } }

class Vec3
{
public:
    Vec3() : x(0.0f), y(0.0f), z(0.0f) {}

    Vec3(float fx, float fy, float fz) : x(fx), y(fy), z(fz) {}

    friend Vec3 operator +(const Vec3& v1, const Vec3& v2)
    {
        return Vec3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
    }

    friend Vec3 operator -(const Vec3& v1, const Vec3& v2)
    {
        return Vec3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
    }

    friend float operator *(const Vec3& v1, const Vec3& v2)
    {
        // Dot product
        return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
    }

    friend Vec3 operator *(float t, const Vec3& v1)
    {
        // Scalar multiplication
        return Vec3(v1.x * t, v1.y * t, v1.z * t);
    }

    friend Vec3& operator +=(Vec3& v1, const Vec3& v2)
    {
        v1.x += v2.x;
        v1.y += v2.y;
        v1.z += v2.z;

        return v1;
    }

    friend Vec3& operator -=(Vec3& v1, const Vec3& v2)
    {
        v1.x -= v2.x;
        v1.y -= v2.y;
        v1.z -= v2.z;

        return v1;
    }

    Vec3 Normalize() const
    {
        // Normalize and return a new vector
        float Length = 1.0f / sqrtf(x * x + y * y + z * z);

        return Vec3(x * Length, y * Length, z * Length);
    }

    void NormalizeIP()
    {
        // Normalize in place (faster)
        float Length = 1.0f / sqrtf(x * x + y * y + z * z);
        x *= Length;
        y *= Length;
        z *= Length;
    }

    float Norm() const
    {
        return sqrtf(x * x + y * y + z * z);
    }

public:
    float x, y, z;
};

class Color
{
public:
    Color() : Red(1.0f), Green(1.0f), Blue(1.0f) {}

    Color(float fR, float fG, float fB) : Red(fR), Green(fG), Blue(fB) {}

    friend Color operator +(const Color& c1, const Color& c2)
    {
        // Piecewise addition
        return Color(c1.Red + c2.Red, c1.Green + c2.Green, c1.Blue + c2.Blue);
    }

    friend Color& operator +=(Color& c1, const Color& c2)
    {
        // Compound piecewise addition
        c1.Red += c2.Red;
        c1.Green += c2.Green;
        c1.Red += c2.Blue;

        return c1;
    }

    friend Color operator -(const Color& c1, const Color& c2)
    {
        // Piecewise subtraction
        return Color(c1.Red - c2.Red, c1.Green - c2.Green, c1.Blue - c2.Blue);
    }

    friend Color& operator -=(Color& c1, const Color& c2)
    {
        // Compound piecewise subtraction
        c1.Red -= c2.Red;
        c1.Green -= c2.Green;
        c1.Red -= c2.Blue;

        return c1;
    }

    friend Color operator *(const Color& c1, const Color& c2)
    {
        // Piecewise multiplication
        return Color(c1.Red * c2.Red, c1.Green * c2.Green, c1.Blue * c2.Blue);
    }

    friend Color& operator *=(Color& c1, const Color& c2)
    {
        // Compound piecewise multiplication
        c1.Red *= c2.Red;
        c1.Green *= c2.Green;
        c1.Red *= c2.Blue;

        return c1;
    }

    friend Color operator *(float t, const Color& c1)
    {
        // Scalar multiplication
        return Color(c1.Red * t, c1.Green * t, c1.Blue * t);
    }

public:
    float Red, Green, Blue;
};

struct Ray
{
    Vec3 Origin, Dir;
};

// Point Light
struct Light
{
    Light() : Red(1.0f), Green(1.0f), Blue(1.0f) {}

    Vec3 Position;
    float Red, Green, Blue;
};

struct Material
{
    Material() : Reflection(0.5f),
        Red(1.0f), Green(1.0f), Blue(1.0f),
        Power(65) {}

    float Reflection;
    //float Refraction;
    //float Density;
    float Red, Green, Blue;
    Color Specular;
    u_int Power;
};

#endif // !Common_h__
