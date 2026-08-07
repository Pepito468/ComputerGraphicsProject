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
    glm::vec2 UV;
};

struct SimpleVertex {
    glm::vec3 pos;
    glm::vec3 norm;
    glm::vec2 UV;
};

struct GlobalUniformBufferObject {
    alignas(16) glm::vec3 lightDir;
    alignas(16) glm::vec4 lightColor;
    alignas(16) glm::vec3 eyePos;
};

struct SimpleUniformBufferObject {
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