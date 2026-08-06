#ifndef ENGINE_MATERIAL_HPP
#define ENGINE_MATERIAL_HPP
#include "common.h"

class Material {

    protected:
    glm::vec3 diffuse;
    glm::vec4 specular;

    public:
    virtual ~Material() = default;
    virtual void updateUBO(SimpleUniformBufferObject& ubo);
};

class LambertMaterial : public Material {
    public:
    LambertMaterial(glm::vec3 diffuse, glm::vec4 specular) {
        this->diffuse = diffuse;
        this->specular = specular;
    }

    ~LambertMaterial() override = default;
    void updateUBO(SimpleUniformBufferObject& ubo) override {
        ubo.color = diffuse;
        ubo.specular = specular;
    }
};

class ToonMaterial : public Material {
public:
    float tD, mD1, mD2;
    float tS, mS1, mS2;

    ToonMaterial(glm::vec3 diffuse, glm::vec4 specular, float tD, float mD1, float mD2, float tS, float mS1, float mS2) {
        this->diffuse = diffuse;
        this->specular = specular;
        this->tD = tD;
        this->tS = tS;
        this->mD1 = mD1;
        this->mD2 = mD2;
        this->mS1 = mS1;
        this->mS2 = mS2;
    }

    ~ToonMaterial() override = default;
    void updateUBO(SimpleUniformBufferObject& ubo) override {
        ubo.color = diffuse;
        ubo.specular = specular;
        ubo.param1.x = tD;
        ubo.param1.y = mD1;
        ubo.param1.z = mD2;
        ubo.param2.x = tS;
        ubo.param2.y = mS1;
        ubo.param2.z = mS2;
    }
};

#endif