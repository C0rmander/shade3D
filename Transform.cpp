#include "Transform.h"
#define _USE_MATH_DEFINES
#include <math.h>

Transform::Transform()
{
    //ctor
}

Matrix4D Transform::translate (Vector3D eye, float pitch, float yaw, Vector3D camMan)
{
    // creates translation matrix given 4D matrix and 3D vector
//    Matrix4D  transMat (1.f,0.f,0.f,-vec.x,
//                       0.f,1.f,0.f, -vec.y,
//                       0.f,0.f,1.f, -vec.z,
//                       0.f,0.f,0.f, 1.f);
//    return mat * transMat;

    float cosPitch = cos(pitch);


    float sinPitch = sin(pitch);


    float cosYaw = cos(yaw);


    float sinYaw = sin(yaw);

    Vector3D xaxis = { cosYaw, 0, -sinYaw };


    Vector3D yaxis = { sinYaw * sinPitch, cosPitch, cosYaw * sinPitch };


    Vector3D zaxis = { sinYaw * cosPitch, -sinPitch, cosPitch * cosYaw };

    Vector4D finalRow = {-xaxis.Dot( xaxis, eye ), -xaxis.Dot( yaxis, eye ), -xaxis.Dot( zaxis, eye ), 1.f};

    return Matrix4D(xaxis.x, yaxis.x, zaxis.x,0,
                    xaxis.y, yaxis.y, zaxis.y,0,
                    xaxis.z, yaxis.z, zaxis.z,0,
                    finalRow.x - camMan.x, finalRow.y - camMan.y, finalRow.z - camMan.z, finalRow.w );
}

Matrix4D Transform::rotate (Matrix4D model, float angle, Vector3D vec)
{
    if(vec.x ==1)
    {
        return Matrix4D (1.f,0.f,0.f,0.f,
                         0.f,cos(angle), -sin(angle),0.f,
                         0.f,sin(angle),cos(angle),0.f,
                         0.f,0.f,0.f,1.f);
    }
//    if(vec.y ==1)
//    {
//        return Matrix4D (1,0,0,
//                         0,cos(angle), -sin(angle),
//                         0,sin(angle),cos(angle),
//                         0,0,0,1);
//    }
//    if(vec.z ==1)
//    {
//        return Matrix4D (1,0,0,
//                         0,cos(angle), -sin(angle),
//                         0,sin(angle),cos(angle),
//                         0,0,0,1);
//    }
    return Matrix4D(1.f);
}

float toRadians(float degs)
{
    const double PI  =3.141592653589793238463;
   (degs * PI)/180;
}

Matrix4D Transform::perspective (float fovY, float aspect, float zNear, float zFar)
{
// creates the below perspective matrix where f = cotangent(fovY/2)
//                   (     f                                  )
//                   |  ------   0       0            0       |
//                   |  aspect                                |
//                   |                                        |
//                   |                                        |
//                   |     0     f       0            0       |
//                   |                                        |
//                   |                                        |
//                   |               zFar+zNear  2*zFar*zNear |
//                   |     0     0   ----------  ------------ |
//                   |               zNear-zFar   zNear-zFar  |
//                   |                                        |
//                   |                                        |
//                   |     0     0      -1            0       |
//                   (                                        )


//float f = 1.0f/tan(toRadians(fovY)*0.5);
//float c = -(zFar)/ (zFar-zNear);
//float l = (zFar*zNear)/ (zFar-zNear);
//return Matrix4D (f/aspect,0,0,0,
//                        0,f,0,0,
//                        0,0,c,-zNear * c,
//                        0,0,1,0);
//return Matrix4D (f/aspect,0,0,0,
//                0,f,0,0,
//                0,0,c,1,
//                0,0,-zNear * c,0);
//return Matrix4D(1.f,0.f,0.f,0.f,
//                0.f,1.f,0.f,0.f,
//                0.f,0.f,1.f,0.f,
//                0.f,0.f,0.f,1.f);
//Matrix4D projection(0.f);
//projection.matrix[0][0] = 1/(aspect * f);
//projection.matrix[1][1] = 1/f;
//projection.matrix[2][2] = c;
//projection.matrix[2][3] = 1;
//projection.matrix[3][2] = -l;
float f = 1.0f/tan(fovY*0.5) * aspect;
float g = 1.0f/tan(fovY*0.5);
float c = (zNear+zFar)/ (zNear-zFar);
float l = 2*(zNear*zFar)/ (zNear-zFar);
return Matrix4D (f/aspect,0,0,0,
                        0,f,0,0,
                        0,0,c,l,
                        0,0,-1,0);
}
