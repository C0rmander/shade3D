// GLSL v4.5
#version 450

//output write

layout (location = 0) in vec3 inColour;
layout (location = 1) in vec3 fn;
layout (location = 2) in vec3 vertPos;

layout(location = 0) out vec4 outFragColour;

layout(set = 0, binding = 1) uniform SceneData
{
    vec4 fogColour;
    vec4 fogDistances;
    vec4 ambientColour;
    vec4 sunlightDirection;
    vec4 sunlightColour;
}sceneData;

const vec4 ambientColor = vec4(0.1, 0.1, 0.1, 1.0);
const vec4 diffuseColor = vec4(0.1, 0.1, 0.1, 1.0);
const vec4 specularColor = vec4(1.0, 1.0, 1.0, 1.0);
const float shininess = 20.0;
const vec4 lightColor = vec4(1.0, 1.0, 1.0, 1.0);
const float irradiPerp = 10.0;

vec3 phongBRDF(vec3 lightDir, vec3 viewDir, vec3 normal, vec3 phongDiffuseCol, vec3 phongSpecularCol, float phongShininess) {
  vec3 color = phongDiffuseCol;
  vec3 reflectDir = reflect(-lightDir, normal);
  float specDot = max(dot(reflectDir, viewDir), 0.0);
  color += pow(specDot, phongShininess) * phongSpecularCol;
  return color;
}

void main()
{
    vec3 lightDir = normalize(-sceneData.sunlightDirection.xyz);
    vec3 viewDir = normalize(-vertPos);
    vec3 n = normalize(fn);

    vec3 radiance = ambientColor.rgb;

    float irradiance = max(dot(lightDir, n), 0.0) * irradiPerp;
    if(irradiance > 0.0) {
        vec3 brdf = phongBRDF(lightDir, viewDir, n, diffuseColor.rgb, specularColor.rgb, shininess);
        radiance += brdf * irradiance * lightColor.rgb;
    }

    radiance = pow(radiance, vec3(1.0 / 2.2) ); // gamma correction
    outFragColour.rgb = radiance;
    outFragColour.a = 1.0;
    // green colour
    //outFragColour = vec4(inColour + sceneData.ambientColour.xyz,1.0f);
}

