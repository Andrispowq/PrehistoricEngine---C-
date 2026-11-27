#include "Includes.hpp"
#include <algorithm>
#include "VKPBRShader.h"
#include "platform/vulkan/rendering/VKShadowPass.h"
#include "prehistoric/core/modules/terrain/Terrain.h"
#include "platform/vulkan/rendering/VKRenderer.h"
#include "platform/vulkan/rendering/VKEnvironmentLighting.h"

#include "prehistoric/application/Application.h"

namespace Prehistoric {
    VKPBRShader::VKPBRShader(Window *window, bool terrain) : VKShader(window->getContext(), window->getSwapchain()),
                                                             terrain(terrain) {
        uint32_t terrainStages = 0;
        if (terrain) {
            VkPhysicalDeviceFeatures features{};
            vkGetPhysicalDeviceFeatures(device->getPhysicalDevice(), &features);
            terrainGeometry = features.geometryShader;
            terrainStages = TESSELLATION_CONTROL_SHADER | TESSELLATION_EVALUATION_SHADER;
            AddShader(ResourceLoader::LoadShaderVK("vulkan/terrain/terrain.vert.spv"), VERTEX_SHADER);
            AddShader(ResourceLoader::LoadShaderVK("vulkan/terrain/terrain.tesc.spv"), TESSELLATION_CONTROL_SHADER);
            const char *evaluation = terrainGeometry ? "geometry.tese.spv" : "terrain.tese.spv";
            AddShader(ResourceLoader::LoadShaderVK(std::string("vulkan/terrain/") + evaluation),
                      TESSELLATION_EVALUATION_SHADER);
            if (terrainGeometry) {
                terrainStages |= GEOMETRY_SHADER;
                AddShader(ResourceLoader::LoadShaderVK("vulkan/terrain/terrain.geom.spv"), GEOMETRY_SHADER);
            }
            AddShader(ResourceLoader::LoadShaderVK("vulkan/terrain/terrain.frag.spv"), FRAGMENT_SHADER);
        } else {
            AddShader(ResourceLoader::LoadShaderVK("vulkan/pbr/pbr_VS.spv"), VERTEX_SHADER);
            AddShader(ResourceLoader::LoadShaderVK("vulkan/pbr/pbr_FS.spv"), FRAGMENT_SHADER);
        }
        AddUniform("camera", VERTEX_SHADER | FRAGMENT_SHADER | terrainStages, UniformBuffer, 0, 0,
                   sizeof(float) * 16 * 2 + Vector4f::size());
        AddUniform("lightConditions", FRAGMENT_SHADER, UniformBuffer, 0, 1, sizeof(int) * 2);
        AddUniform("lights", FRAGMENT_SHADER, UniformBuffer, 0, 2,
                   Vector4f::size() * 3 * __EngineConfig.lightsMaxNumber);

        AddUniform("irradianceMap", FRAGMENT_SHADER, CombinedImageSampler, 0, 3, 0, nullptr);
        AddUniform("prefilterMap", FRAGMENT_SHADER, CombinedImageSampler, 0, 4, 0, nullptr);
        AddUniform("brdfLUT", FRAGMENT_SHADER, CombinedImageSampler, 0, 5, 0, nullptr);
        AddUniform("environment", FRAGMENT_SHADER, UniformBuffer, 0, 6, sizeof(float) * 4);

        if (!terrain) {
            AddUniform("shadowMap", FRAGMENT_SHADER, CombinedImageSampler, 0, 7, 0);
            AddUniform("shadowParameters", FRAGMENT_SHADER, UniformBuffer, 0, 8, sizeof(VKShadowPass::Uniforms));
            AddUniform("voxelGrid", FRAGMENT_SHADER, StorageBuffer, 0, 9, 4);
            AddUniform("voxelSettings", FRAGMENT_SHADER, UniformBuffer, 0, 10, 64);
            AddUniform("voxelPositions", FRAGMENT_SHADER, CombinedImageSampler, 0, 11, 0);
            AddUniform("voxelNormals", FRAGMENT_SHADER, CombinedImageSampler, 0, 12, 0);
            AddUniform("reflectionDepth", FRAGMENT_SHADER, CombinedImageSampler, 0, 15, 0);
            AddUniform("screenRadiance", FRAGMENT_SHADER, CombinedImageSampler, 0, 14, 0);
            AddUniform("voxelIndirect", FRAGMENT_SHADER, CombinedImageSampler, 0, 13, 0);
        }

        if (terrain) {
            AddUniform("voxelGrid", FRAGMENT_SHADER, StorageBuffer, 0, 7, 4);
            AddUniform("voxelSettings", FRAGMENT_SHADER, UniformBuffer, 0, 8, 64);
        }
        if (terrain) {
            AddUniform("heightMap", VERTEX_SHADER | TESSELLATION_EVALUATION_SHADER, CombinedImageSampler, 1, 0, 0,
                       nullptr);
            uint32_t binding = 1;
            for (int i = 0; i < 3; ++i) {
                for (const char *map: {"albedoMap", "normalMap", "mrotMap"})
                    AddUniform(map + std::to_string(i),
                               FRAGMENT_SHADER | (std::string(map) == "mrotMap" ? terrainStages : 0),
                               CombinedImageSampler, 1, binding++, 0, nullptr);
            }
            AddUniform("terrainNormal", FRAGMENT_SHADER, SampledImage, 1, binding++, 0);
            AddUniform("terrainSplat", terrainStages | FRAGMENT_SHADER, SampledImage, 1, binding++, 0);
            for (int i = 0; i < 3; ++i)
                AddUniform("emissionMap" + std::to_string(i), FRAGMENT_SHADER, CombinedImageSampler, 1, binding++, 0);
            AddUniform("mapSampler", terrainStages | FRAGMENT_SHADER, Sampler, 1, binding++, 0);
        } else {
            AddUniform(ALBEDO_MAP, FRAGMENT_SHADER, CombinedImageSampler, 1, 0, 0, nullptr);
            AddUniform(NORMAL_MAP, FRAGMENT_SHADER, CombinedImageSampler, 1, 1, 0, nullptr);
            AddUniform(MROT_MAP, FRAGMENT_SHADER, CombinedImageSampler, 1, 2, 0, nullptr);
            AddUniform("material", FRAGMENT_SHADER, UniformBuffer, 1, 3, 52);
        }
        AddUniform("m_transform", VERTEX_SHADER | terrainStages, UniformBuffer, 2, 0, sizeof(float) * 16);

        if (terrain) {
            AddUniform("material", FRAGMENT_SHADER, UniformBuffer, 2, 1, 52);
            AddUniform("terrainParams", terrainStages | FRAGMENT_SHADER, UniformBuffer, 2, 2, 112);
            AddUniform("leaves", VERTEX_SHADER, StorageBuffer, 2, 3, TerrainQuadtree::maxNodes * sizeof(QuadPatch));
        }

        descriptorPool->finalise(pipelineLayout);

        lightData = new char[Vector4f::size() * 3 * __EngineConfig.lightsMaxNumber];
    }

    VKPBRShader::~VKPBRShader() {
        delete[] lightData;
    }

    void VKPBRShader::BindGlobalSets() const {
        if (commandBuffer) {
            BindSets(commandBuffer, 0, 1);
        }
    }

    void VKPBRShader::BindTextureSets(uint32_t descriptor_index) const {
        if (commandBuffer) {
            BindSets(commandBuffer, 1, 1, descriptor_index);
        }
    }

    void VKPBRShader::BindObjectSets(uint32_t instance_index) const {
        if (commandBuffer) {
            BindSets(commandBuffer, 2, 1, instance_index);
        }
    }

    void VKPBRShader::UpdateGlobalUniforms(Renderer *renderer) const {
        auto *environment = static_cast<VKRenderer *>(renderer)->getEnvironmentLighting();
        // Refresh along with other global uniforms: registering scene instances
        // can reallocate the descriptor pool and invalidate previous bindings.
        SetSampledImage("irradianceMap", environment->getIrradiance());
        SetSampledImage("prefilterMap", environment->getPrefilter());
        SetSampledImage("brdfLUT", environment->getBRDF());
        SetUniformf("environment", environment->getMaxReflectionLOD());

        Camera *camera = renderer->getCamera();
        std::vector<Light *> lights = renderer->getLights();

        const auto &shadows = static_cast<VKRenderer *>(renderer)->GetShadows();
        if (!terrain) {
            SetSampledImage("shadowMap", shadows.GetDepth());
            auto *vk = static_cast<VKRenderer *>(renderer);
            SetSampledImage("voxelPositions", vk->GetVoxelScreenInput(0));
            SetSampledImage("voxelNormals", vk->GetVoxelScreenInput(1));
            SetSampledImage("reflectionDepth", vk->GetVoxelScreenInput(4));
            SetSampledImage("screenRadiance", vk->GetVoxelScreenInput(3));
            SetSampledImage("voxelIndirect", vk->GetVoxelScreenInput(2));
            SetUniformi("voxelSettings", vk->voxelHalfResolution ? 2 : 1, 32);
            SetUniformi("voxelSettings", int(vk->voxelGI), 36);
            const auto &ao = vk->ambientOcclusion;
            SetUniform("voxelSettings",
                       Vector4f(ao.radius, ao.active() ? ao.strength : 0.0f, float(vk->screenSpaceReflections), 0), 48);
            auto buffer = vk->GetVoxelBuffer();
            auto *fallback = descriptorPool->getUniform("voxelGrid", 0)->getBuffer();
            VkDescriptorBufferInfo grid{buffer ? buffer : fallback->getBuffer(), 0, VK_WHOLE_SIZE};
            auto slot = descriptorPool->getUniformLocation("voxelGrid");
            auto set = descriptorPool->getSet(slot.first, 0)->getSets()[swapchain->getAquiredImageIndex()];
            VkWriteDescriptorSet write{
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, set, slot.second, 0, 1,
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &grid
            };
            vkUpdateDescriptorSets(device->getDevice(), 1, &write, 0, nullptr);
            auto min = vk->voxelCenter - Vector3f(vk->voxelExtent * 0.5f);
            SetUniform("voxelSettings", Vector4f(min, vk->voxelExtent), 0);
            SetUniform("voxelSettings",
                       Vector4f(buffer ? float(vk->voxelResolution) : 0.0f, vk->voxelStrength,
                                float(vk->voxelDirectional), float(vk->voxelSunShadows)),
                       16);
            SetUniform("shadowParameters", &shadows.GetUniforms(), sizeof(VKShadowPass::Uniforms), 0);
        }
        if (terrain) {
            auto *vk = static_cast<VKRenderer *>(renderer);
            auto buffer = vk->GetVoxelBuffer();
            auto *fallback = descriptorPool->getUniform("voxelGrid", 0)->getBuffer();
            VkDescriptorBufferInfo grid{buffer ? buffer : fallback->getBuffer(), 0, VK_WHOLE_SIZE};
            auto slot = descriptorPool->getUniformLocation("voxelGrid");
            auto set = descriptorPool->getSet(slot.first, 0)->getSets()[swapchain->getAquiredImageIndex()];
            VkWriteDescriptorSet write{
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, set, slot.second, 0, 1,
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &grid
            };
            vkUpdateDescriptorSets(device->getDevice(), 1, &write, 0, nullptr);
            auto minimum = vk->voxelCenter - Vector3f(vk->voxelExtent * 0.5f);
            SetUniform("voxelSettings", Vector4f(minimum, vk->voxelExtent), 0);
            SetUniform("voxelSettings", Vector4f(buffer ? float(vk->voxelResolution) : 0, vk->voxelStrength,
                                                 float(vk->voxelDirectional), float(vk->voxelSunShadows)), 16);
            SetUniformi("voxelSettings", int(vk->voxelGI), 36);
        }
        lights.erase(std::remove_if(lights.begin(), lights.end(), [&](const Light *light) {
            return !light->illuminates() || light == shadows.GetSun();
        }), lights.end());

        //Offset values copied from the shader
        SetUniform("camera", camera->getViewMatrix(), 0);
        SetUniform("camera", camera->getProjectionMatrix(), 64);
        SetUniform("camera", camera->getPosition(), 128);

        struct LightCond {
            int highDetailRange, lights;
        } lightCond;

        lightCond.highDetailRange = __EngineConfig.rendererHighDetailRange;
        lightCond.lights = (uint32_t) lights.size() >= __EngineConfig.lightsMaxNumber
                               ? __EngineConfig.lightsMaxNumber
                               : (uint32_t) lights.size();
        SetUniform("lightConditions", &lightCond, sizeof(LightCond), 0);

        size_t baseOffset = __EngineConfig.lightsMaxNumber * Vector4f::size();
        for (uint32_t i = 0; i < __EngineConfig.lightsMaxNumber; i++) {
            size_t currentOffset = Vector4f::size() * i;

            if (i < lights.size()) {
                Light *light = lights[i];

                *(Vector4f *) (uintptr_t(lightData) + (baseOffset * 0 + currentOffset)) = Vector4f(
                    light->getParent()->getWorldTransform().getPosition(), 0);
                *(Vector4f *) (uintptr_t(lightData) + (baseOffset * 1 + currentOffset)) = Vector4f(
                    light->getColour(), 0);
                *(Vector4f *) (uintptr_t(lightData) + (baseOffset * 2 + currentOffset)) = Vector4f(
                    light->getIntensity(), light->getRadius(), 1, 0);
            } else {
                *(Vector4f *) (uintptr_t(lightData) + (baseOffset * 0 + currentOffset)) = Vector4f();
                *(Vector4f *) (uintptr_t(lightData) + (baseOffset * 1 + currentOffset)) = Vector4f();
                *(Vector4f *) (uintptr_t(lightData) + (baseOffset * 2 + currentOffset)) = Vector4f();
            }
        }

        SetUniform("lights", lightData, Vector4f::size() * 3 * __EngineConfig.lightsMaxNumber, 0);
    }

    void VKPBRShader::UpdateMaterialUniforms(Material *material, uint32_t descriptor_index) const {
        if (terrain) {
            SetTexture("mapSampler", material->getTexture("terrainNormal"), descriptor_index);
            SetTexture("terrainNormal", material->getTexture("terrainNormal"), descriptor_index);
            SetTexture("terrainSplat", material->getTexture("terrainSplat"), descriptor_index);
            SetTexture("heightMap", material->getTexture("heightMap"), descriptor_index);
            for (int i = 0; i < 3; ++i) {
                for (const char *map: {"albedoMap", "normalMap", "mrotMap", "emissionMap"}) {
                    auto name = map + std::to_string(i);
                    SetTexture(name, material->getTexture(name), descriptor_index);
                }
            }
            return;
        }
        SetTexture(ALBEDO_MAP, material->getTexture(ALBEDO_MAP), descriptor_index);
        SetTexture(NORMAL_MAP, material->getTexture(NORMAL_MAP), descriptor_index);
        SetTexture(MROT_MAP, material->getTexture(MROT_MAP), descriptor_index);
        SetUniform("material", material->getVector4f(MROT), 0, descriptor_index);
        SetUniform("material", material->getVector3f(COLOUR), 16, descriptor_index);
        // Emission uses the same scalar material property as OpenGL. Missing = -1.
        SetUniformf("material", (std::max)(material->getFloat(EMISSION), 0.0f), 32, descriptor_index);
        SetUniformi("material", material->exists(NORMAL_MAP), 48, descriptor_index);
    }

    void VKPBRShader::UpdateObjectUniforms(GameObject *object, uint32_t instance_index) const {
        auto *component = static_cast<RendererComponent *>(object->GetComponent(RENDERER_COMPONENT));
        Material *material = component->getMaterial();

        //Offset values are copied from shaders
        SetUniform("m_transform", object->getWorldTransform().getTransformationMatrix(), 0, instance_index);

        if (terrain) {
            auto *surface = static_cast<Terrain *>(object);
            const auto &leaves = surface->getQuadtree()->getLeaves();
            auto *pipeline = static_cast<VKGraphicsPipeline *>(component->getPipeline());
            static_cast<VKMeshVertexBuffer *>(pipeline->getVertexBuffer())->setInstanceCount(
                static_cast<uint32_t>(leaves.size()));
            static_assert(sizeof(QuadPatch) == 32, "Terrain patch must match the std430 array stride");
            SetUniform("leaves", leaves.data(), leaves.size() * sizeof(QuadPatch), 0, instance_index);
            SetUniform("terrainParams", Vector4f(material->getFloat("heightScale0"), material->getFloat("heightScale1"),
                                                 material->getFloat("heightScale2"),
                                                 float(__EngineConfig.rendererHighDetailRange)), 80, instance_index);
            SetUniform("terrainParams", Vector4f(material->getFloat("hasEmission0"), material->getFloat("hasEmission1"),
                                                 material->getFloat("hasEmission2"), terrainGeometry ? 1.0f : 0.0f), 96,
                       instance_index);
            Vector4f tess(float(__TerrainConfig.tessellationFactor), __TerrainConfig.tessellationSlope,
                          __TerrainConfig.tessellationShift, TerrainMaps::normalStrength);
            SetUniform("terrainParams", tess, 0, instance_index);
            SetUniform("terrainParams",
                       Vector4f(material->getFloat("tiling0"), material->getFloat("tiling1"),
                                material->getFloat("tiling2"), 0), 16, instance_index);
            for (int i = 0; i < 3; ++i)
                SetUniform("terrainParams", material->getVector4f("mrot" + std::to_string(i)), 32 + 16 * i,
                           instance_index);
        }
    }
};