#ifndef MODEL_H
#define MODEL_H

#include "Node3D.hpp"
#include "common.h"
#include "Material.hpp"

/// A node represented with a 3D model
class Model3D : public Node3D {
    std::string modelPath;

    DescriptorSet local;

    Material* material;

    public:
    //TODO: non si dovrebbe usare invece getter e setter?
    Model* model;
    bool isVisible;

    Model3D(std::string modelPath, const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale, Material* material, bool isVisible = true) :
        Node3D(position, rotation, scale)
    {
        this->modelPath = modelPath;
        this->material = material;
        this->isVisible = isVisible;
    }
    ~Model3D() = default;

    void descriptorSetInit(BaseProject* bp, DescriptorSetLayout* localLayout) {
        local.init(bp, localLayout, {material->texture->getViewAndSampler()});
    }

    void descriptorSetCleanup() {
        local.cleanup();
    }

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

    void updateUniformBuffer(uint32_t currentImage, glm::mat4 projection, glm::mat4 view) {
        if (isVisible) {
            SimpleUniformBufferObject ubo;

            glm::mat4 world = localMatrix;

            // now we fill the uniforms
            ubo.mMat = world;
            ubo.nMat = glm::inverse(glm::transpose(world));
            ubo.mvpMat = projection * view * world;
            material->updateUBO(ubo);

            local.map(currentImage, &ubo, 0);
        }
    }

    ShaderType getShaderType() const {
        return material->getShaderType();
    }

    std::string getModelPath() const {
        return modelPath;
    }

    Material* getMaterial() const {
        return material;
    }
};
#endif
