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

const vec4 ambientColor = vec4(1.0, 1.0, 1.0, 1.0);
const vec4 diffuseColor = vec4(1.0, 1.0, 1.0, 1.0);
const vec4 specularColor = vec4(255, 255, 255, 1.0);
const float roughnessValue = 10.0;
const vec4 lightColor = vec4(255, 255, 255, 1.0);
const float irradiPerp = 2.0;
const float F0 = 0.8;
const float k = 0.7;

void main()
{

				vec3 lightDirection = normalize(-sceneData.sunlightDirection.xyz); // to light
			    vec3 normal = normalize(fn);

			    float NdotL = dot(normal, lightDirection);

			    float specular = 0.0;
			    if(NdotL > 0.0)
			    {
			        vec3 eyeDir = normalize(-vertPos); // to eye
			        vec3 halfVector = normalize(lightDirection + eyeDir);
			        float NdotH = max(dot(normal, halfVector), 0.0);
			        float NdotV = max(dot(normal, eyeDir), 0.0);
			        float VdotH = max(dot(eyeDir, halfVector), 0.0);
			        float mSquared = roughnessValue * roughnessValue;

			        // geometric attenuation
			        // Blinn's model
			        float NH2 = 2.0 * NdotH;
			        float g1 = (NH2 * NdotV) / VdotH;
			        float g2 = (NH2 * NdotL) / VdotH;
			        float geoAtt = min(1.0, min(g1, g2));

			        // roughness (microfacet distribution function)
			        // Beckmann distribution
			        float r1 = 1.0 / ( 3.14159265358979323846264 * mSquared * pow(NdotH, 4.0));
			        float r2 = (NdotH * NdotH - 1.0) / (mSquared * NdotH * NdotH);
			        float roughness = r1 * exp(r2);

			        // Fresnel
			        // Schlick's approximation
			        float fresnel = pow(1.0 - VdotH, 5.0);
			        fresnel *= (1.0 - F0);
			        fresnel += F0;

			        specular = (fresnel * geoAtt * roughness) / (NdotV * NdotL * 3.14159265358979323846264);
			    }

	   			vec3 finalValue = NdotL * ((k * diffuseColor.xyz) + (specularColor.xyz * specular * (1.0 - k)));
			    outFragColour = vec4(finalValue, 1.0);
    //outFragColour.a = 1.0;
    // green colour
    //outFragColour = vec4(inColour + sceneData.ambientColour.xyz,1.0f);
}

