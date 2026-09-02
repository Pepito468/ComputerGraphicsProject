#version 450
#extension GL_ARB_separate_shader_objects : enable

// the default render pass has just one attchment of type vec4, representing the pixel on screen
layout (location = 0) out vec4 outColor;

layout (location = 0) in vec3 fragPos;
layout (location = 1) in vec3 fragNorm;
layout (location = 2) in vec2 fragUV;
layout (location = 3) in vec4 fragTan;
layout (location = 4) in vec3 shadowPos;
layout (location = 5) in vec4 screenPos;


// now we need to read the values in the uniforms
// in this shader, we need the local uniforms
layout (binding = 0, set = 1) uniform UniformBufferObject {
	mat4 mvpMat;
	mat4 mMat;
	mat4 nMat;
	vec3 diffuse;
	vec4 specular;
	vec4 param1;
	vec4 param2;
	vec4 param3;
	vec4 param4;
} ubo;


// the shaders now receives also the texture in a separate set
layout (binding = 1, set = 1) uniform sampler2D tex;
layout (binding = 2, set = 1) uniform sampler2D armTex;
layout (binding = 3, set = 1) uniform sampler2D normalTex;
layout (binding = 4, set = 1) uniform sampler2D shadowMap;
layout (binding = 5, set = 1) uniform sampler2D sceneDepth;
layout (binding = 6, set = 1) uniform sampler2D sceneColor;

// and also the global
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

// Converts hue/saturation/brightness to rgb
vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0/3.0, 1.0/3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
    float speed = ubo.param1.x;
    float saturation = ubo.param1.y;
    float brightness = ubo.param1.z;
    float heightInfluence = ubo.param1.w;

    // apply color effect:
    // apply a color (from the spectrum)
    // and apply it differencly based on the position (height) in the model
    float hue = fract(gubo.time * speed + fragPos.y * heightInfluence);
    // convert to rgb
    vec3 rainbow = hsv2rgb(vec3(hue, saturation, brightness));

    // fresnel shimmer
    vec3 N = normalize(fragNorm);
    vec3 V = normalize(gubo.eyePos - fragPos);
    // 1.0 is from fresnel, 0.0 is a guard, 2.0 is the falloff
    float fresnel = pow(1.0 - max(dot(N, V), 0.0), 2.0);
    // apply additively (creates edges)
    rainbow += fresnel * 0.25;

    outColor = vec4(rainbow, 1.0);
}
