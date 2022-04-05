#ifndef FACE3D_H
#define FACE3D_H
#include "Vector3D.h"

struct Face3D
{

int f1,f2,f3;

Face3D()
{

}

Face3D(int a, int b, int c)
{
    f1=a;
    f2=b;
    f3=c;
}
Face3D(const std::vector <float> &face)
{
    if(face.size() == 3)
    {
      f1 = face[0];
      f2 = face[1];
      f3 = face[2];
    }
    else
    {
    f1=0;
    f2=0;
    f3=0;
    }

};

int getF1()
{
    return f1;
}

int getF2()
{
    return f2;
}

int getF3()
{
    return f3;
}
void setF1(int a)
{
    f1=a;
}
void setF2(int b)
{
    f2=b;
}
void setF3(int c)
{
    f3=c;
}
inline Vector3D FaceNormal(const Vector3D& vec1, const Vector3D& vec2,const Vector3D& vec3)
{
    Vector3D n;
    return n.Cross(vec2 - vec1,vec3 - vec1);
}
};

#endif // FACE3D_H
