#version 450
#extension GL_ARB_separate_shader_objects : enable
#define PI2 6.28318530718

// this time, positions and colors are sent by the application
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNorm;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec4 inTangent;

layout (location = 0) out vec3 fragPos;
layout (location = 1) out vec3 fragNorm;
layout (location = 2) out vec2 fragUV;
layout (location = 3) out vec4 fragTan;
layout (location = 4) out vec3 shadowPos;

// now we need to read the values in the uniforms
// in this shader, we need only the local uniforms
layout (binding = 0, set = 1) uniform UniformBufferObject {
    mat4 mvpMat;
    mat4 mMat;
    mat4 nMat;
    vec3 diffuse;
    vec4 specular;
    vec4 param1;
} ubo;

layout(binding = 0, set = 2) uniform ShadowMapUniformBufferObject {
    mat4 mvpMat;
} subo;

layout(binding = 0, set = 0) uniform GlobalUniformBufferObject {
// --- Directional Light ---
    vec4 lightDir;          // xyz = direction TOWARDS the light (normalized)
    vec4 lightColor;        // xyz = color * intensity
    vec3 eyePos;


// --- Hemispheric ambient ---
    vec4 ambientUpper;  // xyz = sky / upper  color  (lU)
    vec4 ambientLower;  // xyz = ground / lower color (lD)
    vec4 ambientDir;    // xyz = "up" direction for blending (d)


// --- Point Light ---
    vec4 pointLightPos[8];     // xyz = world position
    vec4 pointLightColor[8];   // xyz = color * intensity
    vec4 pointLightParams[8];  // x = beta (decay exponent), y = g (target distance)
    int pointInstanceCount;

// --- Spotlight ---
    vec4 spotLightPos[8];      // xyz = world position
    vec4 spotLightDir[8];      // xyz = cone direction (normalized)
    vec4 spotLightColor[8];    // xyz = color * intensity
    vec4 spotLightParams[8];   // x = cIN = cos(alpha_IN/2), y = cOUT = cos(alpha_OUT/2)
    int  spotInstanceCount;

    float time;
} gubo;

void main() {
    // similar to PosNormUV.vert but also applies a shear on the XZ-plane to simulate wind
    vec2 shearDir = normalize(ubo.param1.xy);
    vec3 worldDir = vec3(shearDir.x, 0, shearDir.y);
    vec3 localDir = inverse(mat3(ubo.mMat)) * worldDir;
    // param1.z is the length of the shear for a point at y = 1 in model space
    // param1.w is the period of the wind oscilation
    float easing = (sin(gubo.time * PI2 / ubo.param1.w) + 1) / 2;
    float windStrength = ubo.param1.z * easing;
    vec3 sheared = inPos;
    sheared.xz += localDir.xz * windStrength * sheared.y;

    gl_Position = ubo.mvpMat * vec4(sheared, 1.0f);
    fragPos     = (ubo.mMat * vec4(sheared, 1.0f)).xyz;
    fragNorm    =  mat3(ubo.nMat) * inNorm;
    fragUV		= inUV;
    fragTan = vec4(normalize(mat3(ubo.nMat) * inTangent.xyz), inTangent.w);
    vec4 shadowPosPrj = subo.mvpMat * ubo.mMat * vec4(inPos, 1.0);
    shadowPos = shadowPosPrj.xyz / shadowPosPrj.w;
}
