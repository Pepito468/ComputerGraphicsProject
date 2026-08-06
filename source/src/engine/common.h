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

enum ShaderType {LAMBERT_BLINN, TOON};

const std::string shaderTypeToString(ShaderType s)
{
    switch (s)
    {
        case LAMBERT_BLINN:   return "LambertBlinn";
        case TOON:   return "Toon";
        default:      return "LambertBlinn";
    }
}

#endif