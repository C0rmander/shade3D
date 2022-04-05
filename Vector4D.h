#ifndef VECTOR4D_H
#define VECTOR4D_H
#include <cmath>
#include <vector>
#include <ostream>
#include "Vector3D.h"
struct Vector4D
{

float x,y,z,w;


Vector4D()
{

}
Vector4D(const std::vector <float> &vertex)
{
    if(vertex.size() == 3)
    {
      x = vertex[0];
      y = vertex[1];
      z = vertex[2];
      w = vertex[3];
    }

};
Vector4D(float a, float b, float c, float k)
{
    x=a;
    y=b;
    z=c;
    w=k;
};


//inline Vector3D Cross (const Vector4D& a, const Vector4D& b)
//{
//    return (Vector3D (a.y * b.z - a.z * b.y,
//                      a.z * b.x - a.x * b.z,
//                      a.x * b.y - a.y * b.x));
//
//}

inline float Dot(const Vector4D& vec1, const Vector4D& vec2)
{
    return (vec1.x*vec2.x + vec1.y*vec2.y+vec1.z*vec2.z+vec1.w*vec2.w);
}

inline float Mag(const Vector4D& vec1)
{
    return sqrt(pow(vec1.x,2.0) + pow(vec1.y,2.0) + pow(vec1.z,2.0));
}

inline float Mag2D(const Vector4D& vec1)
{
    return sqrt(pow(vec1.x,2.0) + pow(vec1.y,2.0));
}

};

inline Vector4D operator / (const Vector4D& v, float s)
{
    s = 1.0f / s;
    return (Vector4D(v.x*s, v.y*s, v.z*s, v.w*s));
}

inline bool operator != (const Vector4D v1, const Vector4D v2)
{
    if(v1.x != v2.x || v1.y != v2.y || v1.z != v2.z)
    {
        return true;
    }
    else
    {
        return false;
    }
}

inline bool operator == (const Vector4D v1, const Vector4D v2)
{
    if(v1.x == v2.x && v1.y == v2.y && v1.z == v2.z)
    {
        return true;
    }
    else
    {
        return false;
    }
}

inline bool operator > (const Vector4D v1, const Vector4D v2)
{
    if(v1.x > v2.x && v1.y > v2.y && v1.z > v2.z)
    {
        return true;
    }
    else
    {
        return false;
    }
}

inline Vector4D operator * (const Vector4D& v, float s)
{
    return (Vector4D(v.x*s, v.y*s, v.z*s, v.w*s));
}
inline Vector4D operator * (float s, const Vector4D& v)
{
    return (Vector4D(v.x*s, v.y*s, v.z*s, v.w*s));
}
inline Vector4D operator * (const Vector4D& vec1, const Vector4D& vec2)
{
    return Vector4D(vec1.x*vec2.x, vec1.y*vec2.y, vec1.z*vec2.z,vec1.w*vec2.w);
}

inline Vector4D operator + (const Vector4D& vec1, const Vector4D& vec2)
{
    return Vector4D(vec1.x+vec2.x, vec1.y+vec2.y, vec1.z+vec2.z,vec1.w+vec2.w);
}

inline Vector4D operator - (const Vector4D& vec1, const Vector4D& vec2)
{
    return Vector4D(vec1.x-vec2.x, vec1.y-vec2.y, vec1.z-vec2.z,vec1.w-vec2.w);
}

inline Vector4D operator - (const Vector4D& vec1, const float& s)
{
    return Vector4D(vec1.x-s, vec1.y-s, vec1.z-s,vec1.w-s);
}
inline std::ostream& operator << (std::ostream& o, const Vector4D& vec)
{

    o << "x: " << vec.x << "\ty: " << vec.y << "\tz: " << vec.z<< "\tw: " << vec.w;
}
#endif // Vector4D_H

