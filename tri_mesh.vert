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
    mat4 model;
} CameraData;

layout( push_constant ) uniform constants
{
    vec4 data;
    mat4 mat;
} PushConstants;

layout (location = 1) out vec3 fn;
layout (location = 2) out vec3 vertPos;

void main()
{
  mat4 normalMat = transpose(inverse(PushConstants.mat));
  fn = vec3(normalMat * vec4(vNormal, 0.0));
  vec4 vertPos4 = PushConstants.mat * vec4(vPosition, 1.0);
  vertPos = vec3(vertPos4) / vertPos4.w;
  mat4 transformMatrix = CameraData.projection * PushConstants.mat;
  gl_Position = transformMatrix * vec4(vPosition, 1.0f);
    outColour = vColour;
}
