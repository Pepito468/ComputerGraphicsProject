#ifndef ENGINE_MODEL3D_H
#define ENGINE_MODEL3D_H
#include "Node3D.hpp"
#include "common.h"
#include "Material.hpp"

/// A node represented with a 3D model
class Model3D : public Node3D {
    protected:
    std::string modelPath;
    Material* material;
    Model* model;
    bool isVisible;

    //Vulkan variable
    DescriptorSet local;

    public:

    Model3D() {}

    Model3D(std::string modelPath, const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale, Material* material, bool isVisible = true) :
        Node3D(position, rotation, scale)
    {
        this->modelPath = modelPath;
        this->material = material;
        this->isVisible = isVisible;
    }

    ~Model3D() = default;

    ///This must be called inside PipelineRender.descriptorSetsInits()
    virtual void descriptorSetInit(BaseProject* bp, DescriptorSetLayout* localLayout, RenderPass* RPoffScreen, RenderPass* RPsceneDepth, RenderPass* RPsceneColor) {
        local.init(bp, localLayout, {material->getAlbedoTex()->getViewAndSampler(),
            material->getArmTex()->getViewAndSampler(),
            material->getNormalTex()->getViewAndSampler(),
            RPoffScreen->attachments[0].getViewAndSampler(),
            RPsceneDepth->attachments[0].getViewAndSampler(),
            RPsceneColor->attachments[0].getViewAndSampler()});
    }

    ///This must be called inside PipelineRender.descriptorSetsCleanups()
    virtual void descriptorSetCleanup() {
        local.cleanup();
    }

    ///This must be called inside PipelineRender.populateCommandBuffer()
    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, Pipeline& pipeline) {
        if (isVisible) {
            //select the model
            model->bind(commandBuffer);

            //select the local descriptor set for this object
            local.bind(commandBuffer, pipeline, 1, currentImage);

            // draws something. This is a real Vulkan command, not wrapped by the system
            // note that now we use the "Indexed" version, since we have also the index buffer.
            // moreover, we can count the number of elements to write, from the size of the index array
            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(model->indices.size()), 1, 0, 0, 0);
        }
    }

    static Model3D* fromJSON(const nlohmann::json& json, Model3D* node = nullptr) {
        Model3D *newNode = node ? node: new Model3D();

        Node3D::fromJSON(json, newNode);
        //TODO add support for model and material
        return newNode;
    }

    ///This must be called inside PipelineRender.updateUniformBuffer()
    virtual void updateUniformBuffer(uint32_t currentImage, glm::mat4 projection, glm::mat4 view, float deltaT) {
        if (isVisible) {
            UniformBufferObject ubo;

            glm::mat4 world = getGlobalMatrix();

            // now we fill the uniforms
            ubo.mMat = world;
            ubo.nMat = glm::inverse(glm::transpose(world));
            ubo.mvpMat = projection * view * world;
            material->updateUBO(ubo);

            local.map(currentImage, &ubo, 0);
        }
    }

    virtual void modelInit(BaseProject* bp, VertexDescriptor* vertexDescriptor) {
        if (modelPath.find(".obj") != std::string::npos)
            model->init(bp, vertexDescriptor, "assets/models/" + modelPath,OBJ);
        else if (modelPath.find(".gltf") != std::string::npos)
            model->init(bp, vertexDescriptor, "assets/models/" + modelPath,GLTF);
        else
            std::cout << "Error: " <<  modelPath << ", not a valid object file" << std::endl;
    }

    virtual void localCleanup() {}

    //Getters and setters
    ShaderType getShaderType() const {return material->getShaderType();}
    std::string getModelPath() const {return modelPath;}
    Material* getMaterial() const {return material;}
    Model* getModel() const {return model;}
    void setModel(Model* model) {this->model = model;}
    bool IsVisible() const {return isVisible;}
    void setIsVisible(bool visible) {this->isVisible = visible;}
};

class AnimatedModel3D : public Model3D {

    static constexpr float speedUpAnimFact = 0.85f;
    std::vector<AssetFile> assets;
    std::vector<AnimBlendSegment> segments;
    AnimBlender AB;
    std::vector<Animations> Anim;
    SkeletalAnimation SKA;

    std::string armatureName;
    std::string bodyName;

    bool justOnce = true;

public:

    AnimatedModel3D(){}

    /** <li>assetsPath: contains both model and animations, the model must be place at index 0</li>
     *  <li>armatureName: the right one should be find inside the model file</li>
     *  <li>bodyName: is the name of the model inside the model file</li>
     */
    AnimatedModel3D(std::vector<std::string> assetsPath,const std::vector<AnimBlendSegment>& segments, std::string armatureName, std::string bodyName, const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale, Material* material, bool isVisible = true)
        : Model3D(assetsPath[0], position, rotation, scale, material, isVisible) {

        for (auto s : assetsPath) {
            assets.emplace_back();
            if (s.find(".obj") != std::string::npos)
                assets.back().init(s, OBJ);
            else if (s.find(".gltf") != std::string::npos)
                assets.back().init(s, GLTF);
            else
                std::cout << "Error: " <<  s << ", not a valid object file" << std::endl;
        }

        this->segments = segments;
        this->armatureName = armatureName;
        this->bodyName = bodyName;
    }
    ~AnimatedModel3D() = default;

    void descriptorSetInit(BaseProject *bp, DescriptorSetLayout *localLayout, RenderPass *RPoffScreen, RenderPass *RPsceneDepth, RenderPass *RPsceneColor) override {
        local.init(bp, localLayout, {material->getAlbedoTex()->getViewAndSampler()});


        if (justOnce) {
            Anim.resize(assets.size());
            for(int i = 0; i < Anim.size(); i++) {
                Anim[i].init(assets[i]);
            }
            AB.init(segments);
            AB.Start(2, 0.0f); //TODO: temp
            SKA.init(Anim.data(), assets.size(), armatureName, 0);

            justOnce = false;
        }
    }

    void updateUniformBuffer(uint32_t currentImage, glm::mat4 projection, glm::mat4 view, float deltaT) override {
        glm::mat4 World = getGlobalMatrix();
        AB.Advance(deltaT * speedUpAnimFact);
        UniformBufferObjectAnimated uboa {};
        SKA.Sample(AB);
        std::vector<glm::mat4> *TMsp = SKA.getTransformMatrices();
        glm::mat4 AdaptMat =
            glm::scale(glm::mat4(1.0f), glm::vec3(0.01f)) *
            glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f,0.0f,0.0f));
        for (size_t i = 0; i < MAX_BONES; i++) {
            uboa.mMat[i]   = World * AdaptMat * (*TMsp)[i];
            uboa.mvpMat[i] = projection * view * uboa.mMat[i];
            uboa.nMat[i]   = glm::inverse(glm::transpose(uboa.mMat[i]));
        }
        local.map(currentImage, &uboa, 0);
    }

    void descriptorSetCleanup() override {
        local.cleanup();
    }

    void localCleanup() override {
        for(size_t ian = 0; ian < Anim.size(); ian++) {
            Anim[ian].cleanup();
        }
    }

    void modelInit(BaseProject *bp, VertexDescriptor *vertexDescriptor) override {
        model->initFromAsset(bp, vertexDescriptor, &assets[0], "Mesh", 0, bodyName);
    }

    void changeAnim(int animationNumber, float blendTime) {
        AB.Start(animationNumber, blendTime);
    }


};
#endif
