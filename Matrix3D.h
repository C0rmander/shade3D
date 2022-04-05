#ifndef MATRIX3D_H
#define MATRIX3D_H
#include "Vector3D.h"
struct Matrix3D
{


private:
float test[3] [3];
public:
        Matrix3D()
        {

        }

        Matrix3D(float n00, float n01, float n02,
                 float n10, float n11, float n12,
                 float n20, float n21, float n22)
                 {
                    test[0][0] = n00; test[0][1] = n10; test[0][2] = n20;
                    test[1][0] = n01; test[1][1] = n11; test[1][2] = n21;
                    test[2][0] = n02; test[2][1] = n12; test[2][2] = n22;

                 }
//        Matrix3D(float n00, float n01, float n10, float n11)
//                 {
//                    test[0][0] = n00; test[0][1] = n01;
//                    test[1][0] = n10; test[1][1] = n11;
//                 }

//        Matrix3D(Vector3D& a, Vector3D& b, Vector3D& c)
//        {
//
//
//                    n[0][0] = a.getX(); n[0][1] = a.getY(); n[0][2] = a.getZ();
//                    n[1][0] = b.getX(); n[1][1] = b.getY(); n[1][2] = b.getZ();
//                    n[2][0] = c.getX(); n[2][1] = c.getY(); n[2][2] = c.getZ();
//        }


        float& operator () (int i, int j)
        {
           return (test[j][i]);
        }

        const Vector3D& operator [] (int j) const
        {
           return (*reinterpret_cast<const Vector3D *>(test[j]));
        }

        const float& operator () (int i, int j) const
        {
           return (test[j][i]);
        }
        //inline Vector3D operator * (const Vector3D& v, float s)


    const Matrix3D Inverse(const Matrix3D& M)
        {
            const Vector3D a = M[0];
            const Vector3D b = M[1];
            const Vector3D c = M[2];

            Vector3D r0 = r0.Cross(b,c);
            Vector3D r1 = r1.Cross(c,a);
            Vector3D r2 = r2.Cross(a,b);

            float invDet = 1.0f / r0.Dot (r2,c);

            return (Matrix3D(r0.x * invDet, r0.y * invDet, r0.z* invDet,
                             r1.x * invDet, r1.y * invDet, r1.z* invDet,
                             r2.x * invDet, r2.y * invDet, r2.z* invDet));
        }


};

inline Vector3D operator * (const Matrix3D& M, Vector3D& v)
{
    return Vector3D((M(0,0) * v.x) + (M(0,1) * v.y) + (M(0,2)* v.z),
            (M(1,0) * v.x) + (M(1,1) * v.y) + (M(1,2)* v.z),
             (M(2,0) * v.x)+(M(2,1) * v.y)+(M(2,2) * v.z));
}


#endif // MATRIX3D_H
