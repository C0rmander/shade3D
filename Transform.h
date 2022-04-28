#ifndef TRANSFORM_H
#define TRANSFORM_H
#include "Matrix4D.h"
#include "Vector3D.h"

class Transform
{
    public:
        Transform();
        Matrix4D translate (Vector3D vec, float pitch, float yaw, Vector3D camMan);
        Matrix4D rotate (Matrix4D model, float angle, Vector3D vec);
        Matrix4D perspective (float fovY, float aspect, float zNear, float zFar);

    protected:

    private:
};

#endif // TRANSFORM_H
