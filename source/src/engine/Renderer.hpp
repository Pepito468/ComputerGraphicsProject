#ifndef ENGINE_RENDERER_HPP
#define ENGINE_RENDERER_HPP
#include <unordered_map>

#include "common.h"
#include "PipelineRenderer.hpp"

class Renderer {

    DescriptorSetLayout globalLayout;
    DescriptorSet global;

    VertexDescriptor vertexDescriptor;

    std::unordered_map<ShaderType, PipelineRenderer*> pipelinesMap;
    std::unordered_map <std::string, Model> modelsAssets;
    std::unordered_map <std::string, Texture> texturesAssets;

    public:
    Renderer() {
        pipelinesMap = {};
        modelsAssets = {};
        texturesAssets = {};

        /*
        for (auto t : allShadersTypes) {
            pipelinesMap.insert({t, new PipelineRenderer(t)});
            std::cout << getShaderFragName(t) << std::endl;
        }
        */
    }

    ~Renderer() = default;

    ///Draw a 3D model on the screen
    void instantiate(Model3D* model3D) {

        //Key doesn't exist
        if (modelsAssets.find(model3D->getModelPath()) == modelsAssets.end())
            modelsAssets.insert({model3D->getModelPath(), {}});
        if ( texturesAssets.find(model3D->getMaterial()->getTextureName()) == texturesAssets.end())
            texturesAssets.insert({model3D->getMaterial()->getTextureName(),{}});
        if (pipelinesMap.find(model3D->getShaderType()) == pipelinesMap.end())
            pipelinesMap.insert({model3D->getShaderType(), new PipelineRenderer(model3D->getShaderType())});

        //Insert model and texture
        model3D->model = &modelsAssets[model3D->getModelPath()];
        model3D->getMaterial()->texture = &texturesAssets[model3D->getMaterial()->getTextureName()];

        //Add to existing pipeline
        pipelinesMap.at(model3D->getShaderType())->addModel3D(model3D);
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
                 {0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, UV),
                     sizeof(glm::vec2), UV}
           });

        globalLayout.init(bp, {
            // this array contains the binding:
            // first  element : the binding number
            // second element : the type of element (buffer or texture)
            // third  element : the pipeline stage where it will be used
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, sizeof(GlobalUniformBufferObject), 1}
          });

        for (auto& m : modelsAssets) {
            m.second.init(bp, &vertexDescriptor, "assets/models/" + m.first,OBJ);
        }

        for (auto& t : texturesAssets) {
            t.second.init(bp,"assets/textures/" + t.first);
        }


        for (auto& p : pipelinesMap) {
            p.second->localInit(bp, globalLayout, vertexDescriptor);
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

        for (auto& m : modelsAssets) {
            m.second.cleanup();
        }

        for (auto& t : texturesAssets) {
            t.second.cleanup();
        }

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