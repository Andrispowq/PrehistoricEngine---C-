#include "Includes.hpp"
#include "VKWaterShader.h"
#include "../VKRenderer.h"
#include "../VKWaterSimulation.h"
#include "prehistoric/core/modules/water/Water.h"
#include "prehistoric/application/Application.h"

namespace Prehistoric {
    VKWaterShader::VKWaterShader(Window *window) : VKShader(window->getContext(), window->getSwapchain()) {
        VkPhysicalDeviceFeatures features{};
        vkGetPhysicalDeviceFeatures(device->getPhysicalDevice(), &features);
        const bool geometry = features.geometryShader;
        uint32_t stages = VERTEX_SHADER | TESSELLATION_CONTROL_SHADER | TESSELLATION_EVALUATION_SHADER |
                          FRAGMENT_SHADER;
        AddShader(ResourceLoader::LoadShaderVK("vulkan/water/water.vert.spv"), VERTEX_SHADER);
        AddShader(ResourceLoader::LoadShaderVK("vulkan/water/water.tesc.spv"), TESSELLATION_CONTROL_SHADER);
        AddShader(ResourceLoader::LoadShaderVK(geometry
                                                   ? "vulkan/water/geometry.tese.spv"
                                                   : "vulkan/water/water.tese.spv"), TESSELLATION_EVALUATION_SHADER);
        if (geometry) {
            stages |= GEOMETRY_SHADER;
            AddShader(ResourceLoader::LoadShaderVK("vulkan/water/water.geom.spv"), GEOMETRY_SHADER);
        }
        AddShader(ResourceLoader::LoadShaderVK("vulkan/water/water.frag.spv"), FRAGMENT_SHADER);
        AddUniform("camera", stages, UniformBuffer, 0, 0, 144);
        AddUniform("waterVoxelGrid", FRAGMENT_SHADER, StorageBuffer, 0, 1, 4);
        AddUniform("waterVoxelSettings", FRAGMENT_SHADER, UniformBuffer, 0, 2, 32);
        AddUniform("dudvMap", FRAGMENT_SHADER, CombinedImageSampler, 1, 0, 0);
        AddUniform("model", VERTEX_SHADER, UniformBuffer, 2, 0, 64);
        AddUniform("leaves", VERTEX_SHADER, StorageBuffer, 2, 1, WaterQuadtree::maxNodes * sizeof(QuadPatch));
        AddUniform("params", stages, UniformBuffer, 2, 2, 128);
        unsigned binding = 3;
        for (auto name: {"Dy", "Dx", "Dz"})
            AddUniform(name, TESSELLATION_EVALUATION_SHADER, CombinedImageSampler, 2, binding++, 0);
        for (auto name: {"reflection", "refraction", "depth", "normal"})
            AddUniform(name, FRAGMENT_SHADER, CombinedImageSampler, 2, binding++, 0);
        descriptorPool->finalise(pipelineLayout);
    }

    void VKWaterShader::UpdateGlobalUniforms(Renderer *scene) const {
        renderer = static_cast<VKRenderer *>(scene);
        auto buffer = renderer->GetVoxelBuffer();
        auto *fallback = descriptorPool->getUniform("waterVoxelGrid", 0)->getBuffer();
        VkDescriptorBufferInfo grid{buffer ? buffer : fallback->getBuffer(), 0, VK_WHOLE_SIZE};
        auto slot = descriptorPool->getUniformLocation("waterVoxelGrid");
        auto set = descriptorPool->getSet(slot.first, 0)->getSets()[swapchain->getAquiredImageIndex()];
        VkWriteDescriptorSet write{
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, set, slot.second, 0, 1,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &grid
        };
        vkUpdateDescriptorSets(device->getDevice(), 1, &write, 0, nullptr);
        auto minimum = renderer->voxelCenter - Vector3f(renderer->voxelExtent * 0.5f);
        SetUniform("waterVoxelSettings", Vector4f(minimum, renderer->voxelExtent), 0);
        SetUniform("waterVoxelSettings", Vector4f(buffer ? float(renderer->voxelResolution) : 0,
                                                  renderer->voxelStrength, float(renderer->voxelGI), 0), 16);
        auto *camera = scene->getCamera();
        SetUniform("camera", camera->getViewMatrix(), 0);
        SetUniform("camera", camera->getProjectionMatrix(), 64);
        SetUniform("camera", camera->getPosition(), 128);
    }

    void VKWaterShader::UpdateMaterialUniforms(Material *material, uint32_t index) const {
        SetTexture("dudvMap", material->getTexture("dudvMap"), index);
    }

    void VKWaterShader::UpdateObjectUniforms(GameObject *object, uint32_t index) const {
        auto *water = static_cast<Water *>(object);
        const auto &leaves = water->getLeaves();
        auto *pipeline = static_cast<VKGraphicsPipeline *>(object->GetComponent<RendererComponent>()->getPipeline());
        static_cast<VKMeshVertexBuffer *>(pipeline->getVertexBuffer())->setInstanceCount(uint32_t(leaves.size()));
        SetUniform("model", object->getWorldTransform().getTransformationMatrix(), 0, index);
        SetUniform("leaves", leaves.data(), leaves.size() * sizeof(QuadPatch), 0, index);
        const auto &cfg = __WaterConfig;
        Vector3f sun(0, 1, 0), radiance(0);
        for (auto *light: renderer->getLights()) {
            if (light->castShadows()) {
                sun = light->getParent()->getWorldTransform().getPosition().normalise();
                radiance = light->getColour() * cfg.specularAmplifier;
                break;
            }
        }
        float phase = float(water->GetSimulationTime() * cfg.wavemotion * 0.001);
        const bool wireframe = renderer->isWireframeMode() && pipeline->hasWireframePipeline();
        Vector4f params[] = {
            {float(cfg.tessellationFactor), cfg.tessellationSlope, cfg.tessellationShift, float(wireframe)},
            {cfg.scaleY, cfg.displacementScale, cfg.choppy ? cfg.choppiness : 0.0f, float(cfg.displacementRange)},
            {cfg.windDirection.x, cfg.windDirection.y, phase, float(cfg.tiling)},
            {cfg.kReflection, cfg.kRefraction, cfg.reflectionBlendfactor, water->height()},
            {cfg.capillarStrength, cfg.capillarDownsampling, cfg.dudvDownsampling, water->captureDepthScale},
            Vector4f(cfg.baseColour, float(water->isCameraUnderwater())), Vector4f(sun, cfg.specularFactor),
            Vector4f(radiance, 0)
        };
        SetUniform("params", params, sizeof(params), 0, index);
        auto &simulation = water->simulation();
        SetTexture("Dy", simulation.displacement(0), index);
        SetTexture("Dx", simulation.displacement(1), index);
        SetTexture("Dz", simulation.displacement(2), index);
        SetTexture("normal", simulation.normals(), index);
        auto &targets = renderer->GetWaterTargets(water);
        SetTexture("reflection", targets.reflection.get(), index);
        SetTexture("refraction", targets.refraction.get(), index);
        SetTexture("depth", targets.refraction.get(), index);
    }
}