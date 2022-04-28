// GLSL v4.5
#version 450

//output write

layout (location = 0) in vec3 inColour;

layout(location = 0) out vec4 outFragColour;

layout(set = 0, binding = 1) uniform SceneData
{
    vec4 fogColour;
    vec4 fogDistances;
    vec4 ambientColour;
    vec4 sunlightDirection;
    vec4 sunlightColour;
}sceneData;

void main()
{
    // green colour
    outFragColour = vec4(inColour + sceneData.ambientColour.xyz,1.0f);
}

