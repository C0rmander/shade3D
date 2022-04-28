// using GLSL version 4.5

#version 450

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vColour;

layout (location = 0) out vec3 outColour;

layout( set = 0, binding = 0 ) uniform CameraBuffer
{
    mat4 view;
    mat4 projection;
    mat4 viewproj;
} CameraData;

layout( push_constant ) uniform constants
{
    vec4 data;
    mat4 mat;
} PushConstants;




void main()
{
    mat4 transformMatrix = CameraData.projection * PushConstants.mat;
   // mat4 transformMatrix = PushConstants.mat * CameraData.view;
   // transformMatrix = CameraData.projection * transformMatrix;
    gl_Position = transformMatrix * vec4(vPosition, 1.0f);
    outColour = vColour;
}
