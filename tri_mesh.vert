// using GLSL version 4.5

#version 450

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vColour;

layout (location = 0) out vec3 outColour;

layout( push_constant ) uniform constants
{
    vec4 data;
    mat4 mat;
} PushConstants;
void main()
{


    //output position of each vertex
    gl_Position = PushConstants.mat * vec4(vPosition, 1.0f);
    outColour = vColour;
}
