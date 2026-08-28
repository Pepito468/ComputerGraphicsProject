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

#define TAU 6.28318530717958647692528676655900576839433879875021

//from: https://gist.github.com/ayamflow/c06bc0c8a64f985dd431bd0ac5b557cd
vec2 rotateUV(vec2 uv, float rotation, vec2 mid)
{
    return vec2(
      cos(rotation) * (uv.x - mid.x) + sin(rotation) * (uv.y - mid.y) + mid.x,
      cos(rotation) * (uv.y - mid.y) - sin(rotation) * (uv.x - mid.x) + mid.y
    );
}

//from: https://www.geeks3d.com/3dfr/20140627/effet-2d-le-twirl-ou-swirl-en-glsl/
vec2 twirl(float twirl_amount, float t){
    vec2 uv = fragUV.xy-0.5;
    uv.y *= -1.0;
    float angle = atan(uv.y,uv.x);
    float radius = length(uv);
    angle+= radius * twirl_amount*t;
    return radius*vec2(cos(angle), sin(angle));
}

//from: https://www.youtube.com/watch?v=mh14NDWnHog
void main()
{
    float t = gubo.time;
    vec4 lighterColor = vec4(0.94, 0.52, 0.05, 1.0);
    vec4 darkerColor = vec4(0.74, 0.32, 0.01, 1.0);
    vec4 background = vec4(0.64, 0.32, 0.075, 1.0);
    float twirlStrength = 10;
    float rippleCount = 4;
    float noiseSpeed = .2;
    float rippleStrength = 1.3;
    float colorIntensity = 20;

    vec4 twirl = texture(armTex,  twirl(twirlStrength, t)+0.5);

    //Ripple
    float dist = distance(fragUV, vec2(0.5));
    dist *= rippleCount;
    dist = 1-dist;
    dist += fract(noiseSpeed*t);
    float ripple = sin(dist*TAU)*rippleStrength;

    vec4 albedo = texture(tex,fragUV);
    outColor = mix(albedo*background, albedo*lighterColor*darkerColor*twirl*colorIntensity, clamp(ripple,0.0,1.0));
}