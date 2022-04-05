#ifndef VECTOR3D_H
#define VECTOR3D_H
#include <cmath>
#include <vector>
#include <ostream>
struct Vector3D
{

float x,y,z;


Vector3D()
{

}
Vector3D(const std::vector <float> &vertex)
{
    if(vertex.size() == 3)
    {
      x = vertex[0];
      y = vertex[1];
      z = vertex[2];
    }

};
Vector3D(float a, float b, float c)
{
    x=a;
    y=b;
    z=c;
};


inline Vector3D Cross (const Vector3D& a, const Vector3D& b)
{
    return (Vector3D (a.y * b.z - a.z * b.y,
                      a.z * b.x - a.x * b.z,
                      a.x * b.y - a.y * b.x));

}

inline float Dot(const Vector3D& vec1, const Vector3D& vec2)
{
    return (vec1.x*vec2.x + vec1.y*vec2.y+vec1.z*vec2.z);
}

inline float Mag(const Vector3D& vec1)
{
    return sqrt(pow(vec1.x,2.0) + pow(vec1.y,2.0) + pow(vec1.z,2.0));
}

inline float Mag2D(const Vector3D& vec1)
{
    return sqrt(pow(vec1.x,2.0) + pow(vec1.y,2.0));
}

};

inline Vector3D operator / (const Vector3D& v, float s)
{
    s = 1.0f / s;
    return (Vector3D(v.x*s, v.y*s, v.z*s));
}

inline bool operator != (const Vector3D v1, const Vector3D v2)
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

inline bool operator == (const Vector3D v1, const Vector3D v2)
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

inline bool operator > (const Vector3D v1, const Vector3D v2)
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

inline Vector3D operator * (const Vector3D& v, float s)
{
    return (Vector3D(v.x*s, v.y*s, v.z*s));
}
inline Vector3D operator * (float s, const Vector3D& v)
{
    return (Vector3D(v.x*s, v.y*s, v.z*s));
}
inline Vector3D operator * (const Vector3D& vec1, const Vector3D& vec2)
{
    return (Vector3D(vec1.x*vec2.x, vec1.y*vec2.y, vec1.z*vec2.z));
}

inline Vector3D operator + (const Vector3D& vec1, const Vector3D& vec2)
{
    return (Vector3D(vec1.x+vec2.x, vec1.y+vec2.y, vec1.z+vec2.z));
}

inline Vector3D operator - (const Vector3D& vec1, const Vector3D& vec2)
{
    return (Vector3D(vec1.x-vec2.x, vec1.y-vec2.y, vec1.z-vec2.z));
}

inline Vector3D operator - (const Vector3D& vec1, const float& s)
{
    return (Vector3D(vec1.x-s, vec1.y-s, vec1.z-s));
}
inline std::ostream& operator << (std::ostream& o, const Vector3D& vec)
{

    o << "x: " << vec.x << "\ty: " << vec.y << "\tz: " << vec.z;
}
#endif // VECTOR3D_H
