#ifndef SKELETONTOCHANGE_COMMON_H
#define SKELETONTOCHANGE_COMMON_H

#define  STARTER_IMPLEMENTATION
#include "modules/Starter.hpp"
#define  TEXTMAKER_IMPLEMENTATION
#include "modules/TextMaker.hpp"
#define  SCENE_IMPLEMENTATION
#include "modules/Scene.hpp"

struct Vertex {
    glm::vec3 pos;
    glm::vec3 norm;
    glm::vec2 UV;
    glm::vec4 tan;
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

};

struct UniformBufferObject {
    alignas(16) glm::mat4 mvpMat;
    alignas(16) glm::mat4 mMat;
    alignas(16) glm::mat4 nMat;
    alignas(16) glm::vec3 color;
    alignas(16) glm::vec4 specular;
    alignas(16) glm::vec3 param1;
    alignas(16) glm::vec3 param2;
};

enum ShaderType {LAMBERT_BLINN, LAMBERT_TEX, TOON};

constexpr std::initializer_list<ShaderType> allShadersTypes = {LAMBERT_BLINN, LAMBERT_TEX, TOON};

const std::string getShaderFragName(ShaderType s)
{
    switch (s)
    {
        case LAMBERT_BLINN:   return "LambertBlinn";
        case LAMBERT_TEX:     return "LambertBlinnTexture";
        case TOON:   return "Toon";
        default:      return "Error";
    }
}

const std::string getShaderVertName(ShaderType s)
{
    switch (s)
    {
        case LAMBERT_BLINN:   return "PosNorm";
        case LAMBERT_TEX:     return "PosNormUV";
        case TOON:   return "PosNorm";
        default:      return "Error";
    }
}


#endif