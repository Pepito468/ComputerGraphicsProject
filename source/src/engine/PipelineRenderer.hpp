#ifndef ENGINE_PIPELINERENDERER_HPP
#define ENGINE_PIPELINERENDERER_HPP

#include "common.h"
#include "Model3D.hpp"

class PipelineRenderer {

    ShaderType shaderType;

    std::string vertShader;
    std::string fragShader;

    VertexDescriptor vertexDescriptor;

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

    void localInit(BaseProject* bp, DescriptorSetLayout& globalLayout) {
        vertexDescriptor.init(bp,
            {			// number of "bindings" that this vertex uses
                {0, sizeof(SimpleVertex), VK_VERTEX_INPUT_RATE_VERTEX}	// binding number, size, and type
            }, {		// this must match the structure Vertex defined above
                  {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SimpleVertex, pos),
                         sizeof(glm::vec3), POSITION},
                  {0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SimpleVertex, norm),
                      sizeof(glm::vec3), NORMAL},
                  {0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, UV),
                      sizeof(glm::vec2), UV}
            });

        localLayout.init(bp, {
                    // this array contains the binding:
                    // first  element : the binding number
                    // second element : the type of element (buffer or texture)
                    // third  element : the pipeline stage where it will be used
                    {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, sizeof(SimpleUniformBufferObject), 1},
                    {1,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_ALL_GRAPHICS,0,1}
                  });

        pipeline.init(bp, &vertexDescriptor, vertShader, fragShader, {&globalLayout, &localLayout});

        for (auto& p : pool) {
            p->localInit(bp, &vertexDescriptor);
        }
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
        for (auto& p : pool) {
            p->modelCleanup();
        }
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
