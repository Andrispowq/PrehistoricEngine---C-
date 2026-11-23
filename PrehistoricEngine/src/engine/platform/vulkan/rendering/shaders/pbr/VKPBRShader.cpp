#include "Includes.hpp"
#include <algorithm>
#include "VKPBRShader.h"
#include "prehistoric/core/modules/terrain/TerrainMaps.h"
#include "platform/vulkan/rendering/VKRenderer.h"
#include "platform/vulkan/rendering/VKEnvironmentLighting.h"

#include "prehistoric/application/Application.h"

namespace Prehistoric {
    VKPBRShader::VKPBRShader(Window *window, bool terrain) : VKShader(window->getContext(), window->getSwapchain()), terrain(terrain) {
        uint32_t terrainStages = 0;
        if (terrain)
        {
            terrainStages = TESSELLATION_CONTROL_SHADER | TESSELLATION_EVALUATION_SHADER;
            AddShader(ResourceLoader::LoadShaderVK("vulkan/terrain/terrain.vert.spv"), VERTEX_SHADER);
            AddShader(ResourceLoader::LoadShaderVK("vulkan/terrain/terrain.tesc.spv"), TESSELLATION_CONTROL_SHADER);
            AddShader(ResourceLoader::LoadShaderVK("vulkan/terrain/terrain.tese.spv"), TESSELLATION_EVALUATION_SHADER);
            AddShader(ResourceLoader::LoadShaderVK("vulkan/terrain/terrain.frag.spv"), FRAGMENT_SHADER);
        }
        else
        {
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

        if (terrain)
        {
            AddUniform("heightMap", terrainStages | FRAGMENT_SHADER, CombinedImageSampler, 1, 0, 0, nullptr);
            uint32_t binding = 1;
            for (int i = 0; i < 3; ++i)
            {
                for (const char* map : {"albedoMap", "normalMap", "mrotMap"})
                    AddUniform(map + std::to_string(i), FRAGMENT_SHADER, CombinedImageSampler, 1, binding++, 0, nullptr);
            }
        }
        else
        {
            AddUniform(ALBEDO_MAP, FRAGMENT_SHADER, CombinedImageSampler, 1, 0, 0, nullptr);
            AddUniform(NORMAL_MAP, FRAGMENT_SHADER, CombinedImageSampler, 1, 1, 0, nullptr);
            AddUniform(MROT_MAP, FRAGMENT_SHADER, CombinedImageSampler, 1, 2, 0, nullptr);
        }
        AddUniform("m_transform", VERTEX_SHADER | terrainStages, UniformBuffer, 2, 0, sizeof(float) * 16);
        AddUniform("material", FRAGMENT_SHADER, UniformBuffer, 2, 1,
                   Vector4f::size() * 2 + sizeof(int) + sizeof(float) * 4);

        if (terrain)
            AddUniform("terrainParams", terrainStages | FRAGMENT_SHADER, UniformBuffer, 2, 2, 80);

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
                    light->getIntensity(), 1, 1, 0);
            } else {
                *(Vector4f *) (uintptr_t(lightData) + (baseOffset * 0 + currentOffset)) = Vector4f();
                *(Vector4f *) (uintptr_t(lightData) + (baseOffset * 1 + currentOffset)) = Vector4f();
                *(Vector4f *) (uintptr_t(lightData) + (baseOffset * 2 + currentOffset)) = Vector4f();
            }
        }

        SetUniform("lights", lightData, Vector4f::size() * 3 * __EngineConfig.lightsMaxNumber, 0);
    }

    void VKPBRShader::UpdateMaterialUniforms(Material *material, uint32_t descriptor_index) const {
        if (terrain)
        {
            SetTexture("heightMap", material->getTexture("heightMap"), descriptor_index);
            for (int i = 0; i < 3; ++i)
            {
                for (const char* map : {"albedoMap", "normalMap", "mrotMap"})
                {
                    auto name = map + std::to_string(i);
                    SetTexture(name, material->getTexture(name), descriptor_index);
                }
            }
            return;
        }
        SetTexture(ALBEDO_MAP, material->getTexture(ALBEDO_MAP), descriptor_index);
        SetTexture(NORMAL_MAP, material->getTexture(NORMAL_MAP), descriptor_index);
        SetTexture(MROT_MAP, material->getTexture(MROT_MAP), descriptor_index);
    }

    void VKPBRShader::UpdateObjectUniforms(GameObject *object, uint32_t instance_index) const {
        Material *material = ((RendererComponent *) object->GetComponent(RENDERER_COMPONENT))->getMaterial();

        //Offset values are copied from shaders
        SetUniform("m_transform", object->getWorldTransform().getTransformationMatrix(), 0, instance_index);

        if (terrain)
        {
            Vector4f tess(float(__TerrainConfig.tessellationFactor), __TerrainConfig.tessellationSlope, __TerrainConfig.tessellationShift, TerrainMaps::normalStrength);
            SetUniform("terrainParams", tess, 0, instance_index);
            SetUniform("terrainParams", Vector4f(material->getFloat("tiling0"), material->getFloat("tiling1"), material->getFloat("tiling2"), 0), 16, instance_index);
            for (int i = 0; i < 3; ++i)
                SetUniform("terrainParams", material->getVector4f("mrot" + std::to_string(i)), 32 + 16 * i, instance_index);
            return;
        }
        SetUniform("material", material->getVector4f(MROT), 0, instance_index);
        SetUniform("material", material->getVector3f(COLOUR), 16, instance_index);
        // Emission uses the same scalar material property as OpenGL. Missing = -1.
        SetUniformf("material", (std::max)(material->getFloat(EMISSION), 0.0f), 32, instance_index);
        SetUniformi("material", material->exists(NORMAL_MAP), 48, instance_index);
    }
};