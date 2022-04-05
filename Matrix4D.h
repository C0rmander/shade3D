#ifndef MATRIX4D_H
#define MATRIX4D_H
#include "Vector4D.h"
struct Matrix4D
{


private:

public:
    float matrix[4][4];
        Matrix4D()
        {

        }

        Matrix4D(float n00, float n01, float n02, float n03,
                 float n10, float n11, float n12, float n13,
                 float n20, float n21, float n22, float n23,
                 float n30, float n31, float n32, float n33)
                 {
                    matrix[0][0] = n00; matrix[0][1] = n10; matrix[0][2] = n20; matrix[0][3] = n30;
                    matrix[1][0] = n01; matrix[1][1] = n11; matrix[1][2] = n21; matrix[1][3] = n31;
                    matrix[2][0] = n02; matrix[2][1] = n12; matrix[2][2] = n22; matrix[2][3] = n32;
                    matrix[3][0] = n03; matrix[3][1] = n13; matrix[3][2] = n23; matrix[3][3] = n33;
                 }
        Matrix4D(float f)
                 {
                    matrix[0][0] = f; matrix[0][1] = f; matrix[0][2] = f; matrix[0][3] = f;
                    matrix[1][0] = f; matrix[1][1] = f; matrix[1][2] = f; matrix[1][3] = f;
                    matrix[2][0] = f; matrix[2][1] = f; matrix[2][2] = f; matrix[2][3] = f;
                    matrix[3][0] = f; matrix[3][1] = f; matrix[3][2] = f; matrix[3][3] = f;
                 }
//        Matrix3D(float n00, float n01, float n10, float n11)
//                 {
//                    matrix[0][0] = n00; matrix[0][1] = n01;
//                    matrix[1][0] = n10; matrix[1][1] = n11;
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
           return (matrix[j][i]);
        }

        const Vector4D& operator [] (int j) const
        {
           return (*reinterpret_cast<const Vector4D *>(matrix[j]));
        }

        const float& operator () (int i, int j) const
        {
           return (matrix[j][i]);
        }
        //inline Vector3D operator * (const Vector3D& v, float s)


//    const Matrix3D Inverse(const Matrix3D& M)
//        {
//            const Vector3D a = M[0];
//            const Vector3D b = M[1];
//            const Vector3D c = M[2];
//
//            Vector3D r0 = r0.Cross(b,c);
//            Vector3D r1 = r1.Cross(c,a);
//            Vector3D r2 = r2.Cross(a,b);
//
//            float invDet = 1.0f / r0.Dot (r2,c);
//
//            return (Matrix3D(r0.x * invDet, r0.y * invDet, r0.z* invDet,
//                             r1.x * invDet, r1.y * invDet, r1.z* invDet,
//                             r2.x * invDet, r2.y * invDet, r2.z* invDet));
//        }


};

inline Vector4D operator * (const Matrix4D& M, Vector4D& v)
{
    return Vector4D((M(0,0) * v.x)+(M(0,1) * v.y)+(M(0,2) * v.z)+(M(0,3) * v.w),
                    (M(1,0) * v.x)+(M(1,1) * v.y)+(M(1,2) * v.z)+(M(1,3) * v.w),
                    (M(2,0) * v.x)+(M(2,1) * v.y)+(M(2,2) * v.z)+(M(2,3) * v.w),
                    (M(3,0) * v.x)+(M(3,1) * v.y)+(M(3,2) * v.z)+(M(3,3) * v.w));
}
inline Vector4D operator * (const Matrix4D& M, Vector3D& v)
{
    return Vector4D((M(0,0) * v.x) + (M(0,1) * v.y) + (M(0,2)* v.z) + (M(0,3)* 1),
            (M(1,0) * v.x)+(M(1,1) * v.y) + (M(1,2)* v.z)+ (M(1,3)* 1),
            (M(2,0) * v.x)+(M(2,1) * v.y)+(M(2,2) * v.z)+ (M(2,3)* 1),
            (M(3,0) * v.x)+(M(3,1) * v.y)+(M(3,2) * v.z)+ (M(3,3)* 1));
}
inline Matrix4D operator + (const Matrix4D M1, Matrix4D M2)
{
    return Matrix4D(M1(0,0) + M2(0,0) , M1(1,0)+ M2(1,0) ,M1(2,0)+ M2(2,0),M1(3,0)+ M2(3,0),
                    M1(0,1) + M2(0,1) , M1(1,1)+ M2(1,1) ,M1(2,1)+ M2(2,1),M1(3,1)+ M2(3,1),
                    M1(0,2) + M2(0,2) , M1(1,2)+ M2(1,2) ,M1(2,2)+ M2(2,2),M1(3,2)+ M2(3,2),
                    M1(0,3) + M2(0,3) , M1(1,3)+ M2(1,3) ,M1(2,3)+ M2(2,3),M1(3,3)+ M2(3,3));
}
inline Matrix4D operator * (const Matrix4D M1, Matrix4D M2)
{
    return Matrix4D(M1(0,0) * M2(0,0) , M1(1,0)* M2(1,0) ,M1(2,0)* M2(2,0),M1(3,0)* M2(3,0),
                    M1(0,1) * M2(0,1) , M1(1,1)* M2(1,1) ,M1(2,1)* M2(2,1),M1(3,1)* M2(3,1),
                    M1(0,2) * M2(0,2) , M1(1,2)* M2(1,2) ,M1(2,2)* M2(2,2),M1(3,2)* M2(3,2),
                    M1(0,3) * M2(0,3) , M1(1,3)* M2(1,3) ,M1(2,3)* M2(2,3),M1(3,3)* M2(3,3));
}
//Matrix4D operator *(const Matrix4D& A, const Matrix4D& B)
//{
//
//    return (Matrix4D (A(0,0) * B(0,0) + A(0,1) * B(1,0) + A(0,2) * B(2,0),
//                      A(0,0) * B(0,1) + A(0,1) * B(1,1) + A(0,2) * B(2,1),
//                      A(0,0) * B(0,2) + A(0,1) * B(1,2) + A(0,2) * B(2,2),
//                      A(0,0) * B(0,3) + A(0,1) * B(1,3) + A(0,2) * B(2,3) + A(0,3),
//                      A(1,0) * B(0,0) + A(1,1) * B(1,0) + A(1,2) * B(2,0),
//                      A(1,0) * B(0,1) + A(1,1) * B(1,1) + A(1,2) * B(2,1),
//                      A(1,0) * B(0,2) + A(1,1) * B(1,2) + A(1,2) * B(2,2),
//                      A(1,0) * B(0,3) + A(1,1) * B(1,3) + A(1,2) * B(2,3) + A(1,3),
//                      A(2,0) * B(0,0) + A(2,1) * B(1,0) + A(2,2) * B(2,0),
//                      A(2,0) * B(0,1) + A(2,1) * B(1,1) + A(2,2) * B(2,1),
//                      A(2,0) * B(0,2) + A(2,1) * B(1,2) + A(2,2) * B(2,2),
//                      A(2,0) * B(0,3) + A(2,1) * B(1,3) + A(2,2) * B(2,3) + A(2,3)));
//}));

inline std::ostream& operator << (std::ostream& o, Matrix4D mat)
        {
            for (auto &i : mat.matrix)
            {
                for (auto &j : i)
                {
                    o << j << " ";
                }
                o << std::endl;
            }
        }

#endif // MATRIX3D_H

