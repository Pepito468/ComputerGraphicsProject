#ifndef ENGINE_PIPELINERENDERER_HPP
#define ENGINE_PIPELINERENDERER_HPP

#include "common.h"
#include "Model3D.hpp"

class PipelineRenderer {

    ShaderType shaderType;

    std::string vertShader;
    std::string fragShader;

    DescriptorSetLayout localLayout;

    Pipeline pipeline;

    std::vector <Model3D*> pool;

    public:

    PipelineRenderer(ShaderType type = ShaderType::LAMBERT_BLINN) : shaderType(type) {
        vertShader = "shaders/" + getShaderVertName(shaderType) + ".vert.spv";
        fragShader = "shaders/" + getShaderFragName(shaderType) + ".frag.spv";
    }
    ~PipelineRenderer() = default;

    //TODO: rendere dinamico
    void addModel3D(Model3D* model) {
        pool.push_back(model);
    }

    void localInit(BaseProject* bp, DescriptorSetLayout& globalLayout, VertexDescriptor& vertexDescriptor) {

        localLayout.init(bp, {
                    // this array contains the binding:
                    // first  element : the binding number
                    // second element : the type of element (buffer or texture)
                    // third  element : the pipeline stage where it will be used
                    {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, sizeof(SimpleUniformBufferObject), 1},
                    {1,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_ALL_GRAPHICS,0,1}
                  });

        pipeline.init(bp, &vertexDescriptor, vertShader, fragShader, {&globalLayout, &localLayout});
    }

    void descriptorSetsInits(BaseProject* bp, RenderPass* rp) {
        pipeline.create(rp);


        for (auto& p : pool) {
            p->descriptorSetInit(bp, &localLayout);
        }
    }

    void descriptorSetsCleanup() {
        pipeline.cleanup();
        for (auto& p : pool) {
            p->descriptorSetCleanup();
        }
    }

    void localCleanup() {
        pipeline.destroy();
        localLayout.cleanup();
    }

    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, DescriptorSet& global) {
        pipeline.bind(commandBuffer);
        global.bind(commandBuffer, pipeline, 0, currentImage);
        for (auto& p : pool) {
            p->populateCommandBuffer(commandBuffer, currentImage, pipeline);
        }
    }

    void updateUniformBuffer(uint32_t currentImage,  glm::vec3 CamPos, glm::mat4 Projection, glm::mat4 View) {

        for (auto& p : pool) {
            p->updateUniformBuffer(currentImage, Projection, View);
        }
    }
};

#endif
