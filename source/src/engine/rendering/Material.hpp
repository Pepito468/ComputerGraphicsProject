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
    Texture* armTex;
    Texture* normalTex;


    public:

    virtual ~Material() = default;
    virtual void updateUBO(UniformBufferObject& ubo) = 0;
    ShaderType getShaderType() const {return shaderType;}
    std::string getTextureName() const {return textureName;}
    Texture* getAlbedoTex() const {return albedoTex;}
    Texture* getArmTex() const {return armTex;}
    Texture* getNormalTex() const {return normalTex;}
    void setAlbedoTex(Texture* texture) {this->albedoTex = texture;}
    void setArmTex(Texture* texture) {this->armTex = texture;}
    void setNormalTex(Texture* texture) {this->normalTex = texture;}
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

class WaterMaterial : public Material {
    public:
    WaterMaterial(glm::vec3 diffuse, glm::vec4 specular, std::string textureName = "Default.png") {
        this->diffuse = diffuse;
        this->specular = specular;
        this->textureName = textureName;


        shaderType = ShaderType::WATER;
    }
    ~WaterMaterial() override = default;
    void updateUBO(UniformBufferObject& ubo) override {
        ubo.color = diffuse;
        ubo.specular = specular;
    }
};

class FireMaterial : public Material {

    glm::vec4 param1, param2, param3, param4;

    public:

    FireMaterial(glm::vec3 diffuse, glm::vec4 specular, std::string textureName = "Default.png")
    : FireMaterial(diffuse, specular, {1.0, 0.0, 0.0,1.0}, {0.9,0.9,0.0,1.0},0.05,20,{1,1},1.2,5,3,5, textureName)
    {}

    FireMaterial(glm::vec3 diffuse, glm::vec4 specular, glm::vec4 firstColor, glm::vec4 secondColor, float distortionAmount, float distortionScale, glm::vec2 distortionSpeed, float dissolveAmount, float dissolveScale, float dissolveSpeed, float intensity, std::string textureName = "Default.png") {
        this->diffuse = diffuse;
        this->specular = specular;
        this->textureName = textureName;

        this->param1 = firstColor;
        this->param2 = secondColor;
        this->param3 = glm::vec4(distortionAmount, distortionScale, distortionSpeed);
        this->param4 = glm::vec4(dissolveAmount, dissolveScale, dissolveSpeed, intensity);


        shaderType = ShaderType::FIRE;
    }
    ~FireMaterial() override = default;
    void updateUBO(UniformBufferObject& ubo) override {
        ubo.color = diffuse;
        ubo.specular = specular;

        ubo.param1 = param1;
        ubo.param2 = param2;
        ubo.param3 = param3;
        ubo.param4 = param4;
    }
};

class MagicCirleMaterial : public Material {
    glm::vec4 param1, param2, param3, param4;

    public:

    MagicCirleMaterial(glm::vec3 diffuse, glm::vec4 specular, std::string textureName = "Default.png") {
        this->diffuse = diffuse;
        this->specular = specular;
        this->textureName = textureName;

        shaderType = ShaderType::MAGIC_CIRCLE;
    }
    ~MagicCirleMaterial() override = default;
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

class CookTorranceAnimMaterial : public Material {
public:
    CookTorranceAnimMaterial(glm::vec3 diffuse, glm::vec4 specular, std::string textureName = "Default.png") {
        this->diffuse = diffuse;
        this->specular = specular;

        this->textureName = textureName;

        shaderType = ShaderType::COOK_TORRANCE_ANIM;
    }

    ~CookTorranceAnimMaterial() override = default;
    //TODO: al momento non lo invoca nessuno
    void updateUBO(UniformBufferObject& ubo) override {
        ubo.color = diffuse;
        ubo.specular = specular;
    }
};

class RainbowMaterial : public Material {
    struct params {
        float cycleSpeed;
        float saturation;
        float brightness;
        float heightInfluence;
    } params = {1, 1, 1, 0.3};

    public:

        RainbowMaterial(float cycleSpeed = 1.0f, float saturation = 1.0f, float brightness = 1.0f,
                float heightInfluence = 0.3f, std::string textureName = "Default.png") {

            this->diffuse = glm::vec3(1.0f);
            this->specular = glm::vec4(0.0f);

            this->textureName = textureName;

            this->params = {cycleSpeed, saturation, brightness, heightInfluence};

            shaderType = ShaderType::RAINBOW;
        };

        ~RainbowMaterial() override = default;

        void updateUBO(UniformBufferObject& ubo) override {
            ubo.color = diffuse;
            ubo.specular = specular;
            ubo.param1 = {params.cycleSpeed, params.saturation, params.brightness, params.heightInfluence};
        };
};

#define MAX_RINGS 7
class SonarMaterial : public Material {
    struct params {
        float speed;
        float ringWidth;
        float maxRadius;
    } params = {5, 0.5, 20};

    struct ringEntry {
        glm::vec3 emitterPosition;
        float startTime;
    } ringEntries[MAX_RINGS];
    size_t currentRingEntry = 0;

    public:
        SonarMaterial(float speed = 5.0f, float ringWidth = 0.5f, float maxRadius = 20.0f, std::string textureName = "Default.png") {

            this->diffuse = glm::vec3(1.0f);
            this->specular = glm::vec4(0.0f);

            // Params
            this->params.speed = speed;
            this->params.ringWidth = ringWidth;
            this->params.maxRadius = maxRadius;

            for (size_t i = 0; i < MAX_RINGS; i++) {
                ringEntries[i] = {{0.0f, 0.0f, 0.0f}, -1e9}; // initialize at a very low time so that it is never fired unless triggered
            }

            this->textureName = textureName;

            shaderType = ShaderType::SONAR;
        };

        ~SonarMaterial() override = default;

        /** Trigger the sonar 
         * @param pos emitter position
         * @param currentTime time at the start of the propagation (total)
         * */
        void trigger(glm::vec3 pos, float currentTime) {
            // Cycle the entries
            ringEntries[currentRingEntry] = {pos, currentTime};
            currentRingEntry = (currentRingEntry + 1) % MAX_RINGS;
        }

        void updateUBO(UniformBufferObject& ubo) override {
            ubo.color = diffuse;
            ubo.specular = specular;
            ubo.param1 = {params.speed, params.ringWidth, params.maxRadius, 0.0f};

            // Each ring has {POS, startTime} as entry
            ubo.param2 = glm::vec4(ringEntries[0].emitterPosition, ringEntries[0].startTime);
            ubo.param3 = glm::vec4(ringEntries[1].emitterPosition, ringEntries[1].startTime);
            ubo.param4 = glm::vec4(ringEntries[2].emitterPosition, ringEntries[2].startTime);
            ubo.param5 = glm::vec4(ringEntries[3].emitterPosition, ringEntries[3].startTime);
            ubo.param6 = glm::vec4(ringEntries[4].emitterPosition, ringEntries[4].startTime);
            ubo.param7 = glm::vec4(ringEntries[5].emitterPosition, ringEntries[5].startTime);
            ubo.param8 = glm::vec4(ringEntries[6].emitterPosition, ringEntries[6].startTime);
        };

};
#endif
