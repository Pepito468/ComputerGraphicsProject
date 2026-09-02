#version 450
#extension GL_ARB_separate_shader_objects : enable

#define MAX_RINGS 7

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
	vec4 param1; // params

    // Rings
	vec4 param2;
	vec4 param3;
	vec4 param4;
	vec4 param5;
	vec4 param6;
	vec4 param7;
	vec4 param8;
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


void main() {
    float speed = ubo.param1.x;
    float ringWidth = ubo.param1.y;
    float maxRadius = ubo.param1.z;
    vec4 rings[MAX_RINGS];
    rings[0] = ubo.param2;
    rings[1] = ubo.param3;
    rings[2] = ubo.param4;
    rings[3] = ubo.param5;
    rings[4] = ubo.param6;
    rings[5] = ubo.param7;
    rings[6] = ubo.param8;


    float intensity = 0.0;

    // For every ring
    for (int i = 0; i < MAX_RINGS; i++) {
        // Get ring data
        vec3 emitterPos = rings[i].xyz;
        float startTime = rings[i].w;

        // Know how much the ring has expanded
        float t = gubo.time - startTime;

        // Skip invalid rings
        if (t < 0)
            continue;

        // get distance from emitter
        float dist = length(fragPos - emitterPos);
        // how much the ring has expanded
        float front = t * speed;
        // get distance from current wavefront radius and normalize it
        float band = 1.0 - abs(dist - front) / ringWidth;
        band = clamp(band, 0.0, 1.0);
        // make line thinner
        band = pow(band, 2.0);

        // do not expand forever
        float lifeFade = clamp(1.0 - front/maxRadius, 0.0, 1.0);

        intensity = max(intensity, band * lifeFade);
    }

    // Ignore already black material
    if (intensity = 0)
        discard;
    outColor = vec4(vec3(intensity), 1.0);
}
