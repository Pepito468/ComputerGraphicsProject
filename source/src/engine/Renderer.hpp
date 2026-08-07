#ifndef ENGINE_RENDERER_HPP
#define ENGINE_RENDERER_HPP
#include <unordered_map>

#include "common.h"
#include "PipelineRenderer.hpp"

class Renderer {

    std::unordered_map<ShaderType, PipelineRenderer*> pipelinesMap;


    DescriptorSetLayout globalLayout;
    DescriptorSet global;

    public:
    Renderer() {
        pipelinesMap = {};

        for (auto t : allShadersTypes) {
            pipelinesMap.insert({t, new PipelineRenderer(t)});
            std::cout << getShaderFragName(t) << std::endl;
        }
    }

    ~Renderer() = default;

    void instantiate(Model3D* model3D) {
        pipelinesMap.at(model3D->getShaderType())->addModel3D(model3D);
    }

    void localInit(BaseProject* bp) {
        globalLayout.init(bp, {
            // this array contains the binding:
            // first  element : the binding number
            // second element : the type of element (buffer or texture)
            // third  element : the pipeline stage where it will be used
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, sizeof(GlobalUniformBufferObject), 1}
          });



        for (auto& p : pipelinesMap) {
            p.second->localInit(bp, globalLayout);
        }
    }

    void descriptorSetsInits(BaseProject* bp, RenderPass* rp) {

        global.init(bp, &globalLayout, {});

        for (auto& p : pipelinesMap) {
            p.second->descriptorSetsInits(bp, rp);
        }
    }

    void descriptorSetsCleanup() {

        global.cleanup();

        for (auto& p : pipelinesMap) {
            p.second->descriptorSetsCleanup();
        }
    }

    void localCleanup() {

        globalLayout.cleanup();

        for (auto& p : pipelinesMap) {
            p.second->localCleanup();
        }
    }

    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
        for (auto& p : pipelinesMap) {
            p.second->populateCommandBuffer(commandBuffer, currentImage, global);
        }
    }

    void updateUniformBuffer(uint32_t currentImage,  glm::vec3 CamPos, glm::mat4 Projection, glm::mat4 View) {
        //TODO: provvisoro per test
        GlobalUniformBufferObject gubo;

        // fills with the relevant data
        gubo.lightDir = glm::vec3(0.5656854, 0.7071068, 0.4242641);
        gubo.lightColor = glm::vec4(1.0, 1.0, 1.0, 0.0);
        // now the eye position corresponds to the position of the camera
        gubo.eyePos = CamPos;

        // transfers the data to the GPU, by mapping it to its
        // descriptor set
        global.map(currentImage, &gubo, 0);

        for (auto& p : pipelinesMap) {
            p.second->updateUniformBuffer(currentImage, CamPos, Projection, View);
        }
    }


};
#endif