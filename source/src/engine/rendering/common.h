#ifndef SKELETONTOCHANGE_COMMON_H
#define SKELETONTOCHANGE_COMMON_H

#define  STARTER_IMPLEMENTATION
#include "modules/Starter.hpp"
#define  TEXTMAKER_IMPLEMENTATION
#include "modules/TextMaker.hpp"
#define  ANIMATIONS_IMPLEMENTATION
#include "modules/Animations.hpp"

#define MAX_BONES 65

struct Vertex {
    glm::vec3 pos;
    glm::vec3 norm;
    glm::vec2 UV;
    glm::vec4 tan;
};

struct VertexAnim {glm::vec3 pos;
    glm::vec3 norm;
    glm::vec2 UV;
    glm::uvec4 jointIndices;
    glm::vec4 weights;
};

struct UniformBufferObjectAnimated {
    alignas(16) glm::mat4 mvpMat[MAX_BONES];
    alignas(16) glm::mat4 mMat[MAX_BONES];
    alignas(16) glm::mat4 nMat[MAX_BONES];
};

struct GlobalUniformBufferObject {
    alignas(16) glm::vec4 lightDir;
    alignas(16) glm::vec4 lightColor;
    alignas(16) glm::vec4 eyePos;

    // --- Hemispheric ambient ---
    alignas(16) glm::vec4 ambientUpper;  // xyz = sky / upper  color  (lU)
    alignas(16) glm::vec4 ambientLower;  // xyz = ground / lower color (lD)
    alignas(16) glm::vec4 ambientDir;    // xyz = "up" direction for blending (d)

    // --- Point Light ---
    alignas(16)glm::vec4 pointLightPos[8];     // xyz = world position
    alignas(16)glm::vec4 pointLightColor[8];   // xyz = color * intensity
    alignas(16)glm::vec4 pointLightParams[8];  // x = beta (decay exponent), y = g (target distance)
    alignas(4) int pointInstanceCount;

    // --- Spotlight ---
    alignas(16)glm::vec4 spotLightPos[8];      // xyz = world position
    alignas(16)glm::vec4 spotLightDir[8];      // xyz = cone direction (normalized)
    alignas(16)glm::vec4 spotLightColor[8];    // xyz = color * intensity
    alignas(16)glm::vec4 spotLightParams[8];   // x = cIN = cos(alpha_IN/2), y = cOUT = cos(alpha_OUT/2)
    alignas(4) int spotInstanceCount;

    alignas(4) float time;

};

struct UniformBufferObject {
    alignas(16) glm::mat4 mvpMat;
    alignas(16) glm::mat4 mMat;
    alignas(16) glm::mat4 nMat;
    alignas(16) glm::vec3 color;
    alignas(16) glm::vec4 specular;
    alignas(16) glm::vec4 param1;
    alignas(16) glm::vec4 param2;
    alignas(16) glm::vec4 param3;
    alignas(16) glm::vec4 param4;
    alignas(16) glm::vec4 param5;
    alignas(16) glm::vec4 param6;
    alignas(16) glm::vec4 param7;
    alignas(16) glm::vec4 param8;
    alignas(16) glm::vec4 param9;
};

enum ShaderType {LAMBERT_BLINN, LAMBERT_TEX, WATER, TOON, MAGIC_CIRCLE, FIRE, COOK_TORRANCE_ANIM, RAINBOW, SONAR};

constexpr std::initializer_list<ShaderType> allShadersTypes = {LAMBERT_BLINN, LAMBERT_TEX,WATER, TOON, MAGIC_CIRCLE, FIRE, COOK_TORRANCE_ANIM, RAINBOW, SONAR};

const std::string getShaderFragName(ShaderType s)
{
    switch (s)
    {
        case LAMBERT_BLINN:   return "LambertBlinn";
        case LAMBERT_TEX:     return "LambertBlinnTexture";
        case WATER: return "Water";
        case TOON:   return "Toon";
        case FIRE:   return "Fire";
        case MAGIC_CIRCLE: return "MagicCircle";
        case COOK_TORRANCE_ANIM: return "CookTorranceAnim";
        case RAINBOW: return "Rainbow";
        case SONAR: return "Sonar";
        default:      return "Error";
    }
}

const std::string getShaderVertName(ShaderType s)
{
    switch (s)
    {
        case LAMBERT_BLINN:   return "PosNorm";
        case LAMBERT_TEX:     return "PosNormUV";
        case WATER:   return "Water";
        case TOON:   return "PosNorm";
        case FIRE:   return "Fire";
        case MAGIC_CIRCLE: return "PosNormUV";
        case COOK_TORRANCE_ANIM: return "PosNormUvWeights";
        case RAINBOW: return "Rainbow";
        case SONAR: return "Sonar";
        default:      return "Error";
    }
}

const bool IsAnimShader(ShaderType s) {
    switch (s) {
        case COOK_TORRANCE_ANIM:   return true;
        default: return false;
    }
}

const bool IsLateDraw(ShaderType s) {
    switch (s) {
        case FIRE: return true;
        case MAGIC_CIRCLE: return true;
        default: return false;
    }
}

#endif
