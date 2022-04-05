// GLSL v4.5
#version 450

//output write

layout (location = 0) in vec3 inColour;

layout(location = 0) out vec4 outFragColour;

void main()
{
    // green colour
    outFragColour = vec4(inColour,1.0f);
}
