#ifndef ENGINE_MODEL3D_H
#define ENGINE_MODEL3D_H
#include "Node3D.hpp"
#include "common.h"
#include "Material.hpp"

/// A node represented with a 3D model
class Model3D : public Node3D {

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
    void descriptorSetInit(BaseProject* bp, DescriptorSetLayout* localLayout, RenderPass* RPoffScreen) {
        local.init(bp, localLayout, {material->getAlbedoTex()->getViewAndSampler(),
            material->getAmbientOcclusionTex()->getViewAndSampler(),
            material->getMetallicTex()->getViewAndSampler(),
            material->getNormalTex()->getViewAndSampler(),
            material->getRoughnessTex()->getViewAndSampler(),
            RPoffScreen->attachments[0].getViewAndSampler()});
    }

    ///This must be called inside PipelineRender.descriptorSetsCleanups()
    void descriptorSetCleanup() {
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

    static Model3D* fromJSON(const nlohmann::json& json) {
        Model3D *newNode = new Model3D();

        if (json.contains("name")) newNode->name = json["name"].get<std::string>();

        glm::vec3 globalPosition = VEC3_ZERO;
        glm::vec3 globalRotation = VEC3_ZERO;
        glm::vec3 globalScale = VEC3_ONE;
        if (json.contains("globalPosition")) globalPosition = glm::vec3(json["globalPosition"][0].get<float>(), json["globalPosition"][1].get<float>(), json["globalPosition"][2].get<float>());
        if (json.contains("globalRotation")) globalRotation = glm::vec3(json["globalRotation"][0].get<float>(), json["globalRotation"][1].get<float>(), json["globalRotation"][2].get<float>());
        if (json.contains("globalScale")) globalScale = glm::vec3(json["globalScale"][0].get<float>(), json["globalScale"][1].get<float>(), json["globalScale"][2].get<float>());
        newNode->setGlobalPosition(globalPosition);
        newNode->setGlobalRotation(globalRotation);
        newNode->setGlobalScale(globalScale);

        return newNode;
    }

    ///This must be called inside PipelineRender.updateUniformBuffer()
    void updateUniformBuffer(uint32_t currentImage, glm::mat4 projection, glm::mat4 view) {
        if (isVisible) {
            UniformBufferObject ubo;

            glm::mat4 world = getLocalMatrix();

            // now we fill the uniforms
            ubo.mMat = world;
            ubo.nMat = glm::inverse(glm::transpose(world));
            ubo.mvpMat = projection * view * world;
            material->updateUBO(ubo);

            local.map(currentImage, &ubo, 0);
        }
    }

    //Getters and setters
    ShaderType getShaderType() const {return material->getShaderType();}
    std::string getModelPath() const {return modelPath;}
    Material* getMaterial() const {return material;}
    Model* getModel() const {return model;}
    void setModel(Model* model) {this->model = model;}
    bool IsVisible() const {return isVisible;}
    void setIsVisible(bool visible) {this->isVisible = visible;}
};
#endif
