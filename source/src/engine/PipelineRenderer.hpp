#ifndef ENGINE_PIPELINERENDERER_HPP
#define ENGINE_PIPELINERENDERER_HPP

#include "common.h"
#include "Model3D.hpp"

class PipelineRenderer {

    ShaderType shaderType;

    std::string vertShader;
    std::string fragShader;

    VertexDescriptor vertexDescriptor;

    DescriptorSetLayout globalLayout;
    DescriptorSetLayout localLayout;
    DescriptorSet global;


    Pipeline pipeline;

    std::vector <Model3D*> pool;

    public:
    PipelineRenderer(ShaderType type = ShaderType::LAMBERT_BLINN) : shaderType(type) {
        vertShader = "shaders/PosNorm.vert.spv";
        fragShader = "shaders/" + shaderTypeToString(shaderType) + ".frag.spv";
    }
    ~PipelineRenderer() = default;

    //TODO: rendere dinamico
    void instantiate(Model3D* model) {
        pool.push_back(model);
    }

    void localInit(BaseProject* bp) {
        vertexDescriptor.init(bp,
            {			// number of "bindings" that this vertex uses
                {0, sizeof(SimpleVertex), VK_VERTEX_INPUT_RATE_VERTEX}	// binding number, size, and type
            }, {		// this must match the structure Vertex defined above
                  {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SimpleVertex, pos),
                         sizeof(glm::vec3), POSITION},
                  {0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SimpleVertex, norm),
                         sizeof(glm::vec3), NORMAL},
            });

        localLayout.init(bp, {
                    // this array contains the binding:
                    // first  element : the binding number
                    // second element : the type of element (buffer or texture)
                    // third  element : the pipeline stage where it will be used
                    {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, sizeof(SimpleUniformBufferObject), 1},
                  });

        globalLayout.init(bp, {
                // this array contains the binding:
                // first  element : the binding number
                // second element : the type of element (buffer or texture)
                // third  element : the pipeline stage where it will be used
                {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, sizeof(GlobalUniformBufferObject), 1}
              });

        pipeline.init(bp, &vertexDescriptor, vertShader, fragShader, {&globalLayout, &localLayout});

        for (auto& p : pool) {
            p->localInit(bp, &vertexDescriptor);
        }
    }

    void descriptorSetsInits(BaseProject* bp, RenderPass* rp) {
        pipeline.create(rp);

        global.init(bp, &globalLayout, {});

        for (auto& p : pool) {
            p->descriptorSetInit(bp, &localLayout);
        }
    }

    void descriptorSetsCleanup() {
        pipeline.cleanup();
        global.cleanup();
        for (auto& p : pool) {
            p->descriptorSetCleanup();
        }
    }

    void localCleanup() {
        pipeline.destroy();
        globalLayout.cleanup();
        localLayout.cleanup();
        for (auto& p : pool) {
            p->modelCleanup();
        }
    }

    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
        pipeline.bind(commandBuffer);
        global.bind(commandBuffer, pipeline, 0, currentImage);
        for (auto& p : pool) {
            p->populateCommandBuffer(commandBuffer, currentImage, pipeline);
        }
    }

    //TODO: provvisoro per test
    void updateUniformBuffer(uint32_t currentImage,  glm::vec3 CamPos, glm::mat4 Projection, glm::mat4 View) {
        GlobalUniformBufferObject gubo;

        // fills with the relevant data
        gubo.lightDir = glm::vec3(0.5656854, 0.7071068, 0.4242641);
        gubo.lightColor = glm::vec4(1.0, 1.0, 1.0, 0.0);
        // now the eye position corresponds to the position of the camera
        gubo.eyePos = CamPos;

        // transfers the data to the GPU, by mapping it to its
        // descriptor set
        global.map(currentImage, &gubo, 0);


        for (auto& p : pool) {
            p->updateUniformBuffer(currentImage, Projection, View);
        }
    }
};

#endif
