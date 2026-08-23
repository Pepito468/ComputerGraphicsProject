#ifndef ENGINE_MATERIAL_HPP
#define ENGINE_MATERIAL_HPP
#include "common.h"


///NOTE: Each time you create a new shader you must create a new material type that realize that shader.
///      You will also need to update information inside the common.h file

///A material is meant as a configuration of parameters for the associated shader
class Material {

    protected:
    glm::vec3 diffuse;
    glm::vec4 specular;

    ShaderType shaderType;
    std::string textureName;
    Texture* albedoTex;
    Texture* ambientOcclusionTex;
    Texture* metallicTex;
    Texture* normalTex;
    Texture* roughnessTex;


    public:

    virtual ~Material() = default;
    virtual void updateUBO(UniformBufferObject& ubo) = 0;
    ShaderType getShaderType() const {return shaderType;}
    std::string getTextureName() const {return textureName;}
    Texture* getAlbedoTex() const {return albedoTex;}
    Texture* getAmbientOcclusionTex() const {return ambientOcclusionTex;}
    Texture* getMetallicTex() const {return metallicTex;}
    Texture* getNormalTex() const {return normalTex;}
    Texture* getRoughnessTex() const {return roughnessTex;}
    void setAlbedoTex(Texture* texture) {this->albedoTex = texture;}
    void setAmbientOcclusionTex(Texture* texture) {this->ambientOcclusionTex = texture;}
    void setMetallicTex(Texture* texture) {this->metallicTex = texture;}
    void setNormalTex(Texture* texture) {this->normalTex = texture;}
    void setRoughnessTex(Texture* texture) {this->roughnessTex = texture;}
};

class LambertTexMaterial : public Material {

    public:
    LambertTexMaterial(glm::vec3 diffuse, glm::vec4 specular, std::string textureName = "Default.png") {
        this->diffuse = diffuse;
        this->specular = specular;

        this->textureName = textureName;

        shaderType = ShaderType::LAMBERT_TEX;
    }

    ~LambertTexMaterial() override = default;
    void updateUBO(UniformBufferObject& ubo) override {
        ubo.color = diffuse;
        ubo.specular = specular;
    }
};

class LambertMaterial : public Material {
    public:
    LambertMaterial(glm::vec3 diffuse, glm::vec4 specular, std::string textureName = "Default.png") {
        this->diffuse = diffuse;
        this->specular = specular;

        this->textureName = textureName;

        shaderType = ShaderType::LAMBERT_BLINN;
    }

    ~LambertMaterial() override = default;
    void updateUBO(UniformBufferObject& ubo) override {
        ubo.color = diffuse;
        ubo.specular = specular;
    }
};

class ToonMaterial : public Material {
public:
    float tD, mD1, mD2;
    float tS, mS1, mS2;

    ToonMaterial(glm::vec3 diffuse, glm::vec4 specular, float tD, float mD1, float mD2, float tS, float mS1, float mS2, std::string textureName = "Default.png") {
        this->diffuse = diffuse;
        this->specular = specular;
        this->tD = tD;
        this->tS = tS;
        this->mD1 = mD1;
        this->mD2 = mD2;
        this->mS1 = mS1;
        this->mS2 = mS2;

        this->textureName = textureName;

        shaderType = ShaderType::TOON;
    }

    ~ToonMaterial() override = default;
    void updateUBO(UniformBufferObject& ubo) override {
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