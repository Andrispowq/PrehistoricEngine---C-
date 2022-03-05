#include "Includes.hpp"
#include <glad/glad.h>
#include "VKPBRShader.h" 

#include "prehistoric/application/Application.h"

namespace Prehistoric
{
	VKPBRShader::VKPBRShader(Window* window) : VKShader(window->getContext(), window->getSwapchain())
	{
		AddShader(ResourceLoader::LoadShaderVK("vulkan/pbr/pbr_VS.spv"), VERTEX_SHADER);
		AddShader(ResourceLoader::LoadShaderVK("vulkan/pbr/pbr_FS.spv"), FRAGMENT_SHADER);

		AddUniform("camera", VERTEX_SHADER | FRAGMENT_SHADER, UniformBuffer, 0, 0, sizeof(float) * 16 * 2 + Vector4f::size());
		AddUniform("lightConditions", VERTEX_SHADER | FRAGMENT_SHADER, UniformBuffer, 0, 1, sizeof(int) * 2 + sizeof(float) * 2);
		AddUniform("lights", VERTEX_SHADER | FRAGMENT_SHADER, UniformBuffer, 0, 2, Vector4f::size() * 3 * __EngineConfig.lightsMaxNumber);

		AddUniform(ALBEDO_MAP, FRAGMENT_SHADER, CombinedImageSampler, 1, 0, 0, nullptr);
		AddUniform(NORMAL_MAP, FRAGMENT_SHADER, CombinedImageSampler, 1, 1, 0, nullptr);
		AddUniform(MROT_MAP, FRAGMENT_SHADER, CombinedImageSampler, 1, 2, 0, nullptr);
		AddUniform(EMISSION_MAP, FRAGMENT_SHADER, CombinedImageSampler, 1, 3, 0, nullptr);

		AddUniform("m_transform", VERTEX_SHADER, UniformBuffer, 2, 0, sizeof(float) * 16);
		AddUniform("material", FRAGMENT_SHADER, UniformBuffer, 2, 1, Vector4f::size() * 2 + sizeof(int) + sizeof(float) * 4);

		descriptorPool->finalise(pipelineLayout);

		lightData = new char[Vector4f::size() * 3 * __EngineConfig.lightsMaxNumber];
	}

	VKPBRShader::~VKPBRShader()
	{
		delete[] lightData;
	}

	void VKPBRShader::BindGlobalSets() const
	{
		if (commandBuffer)
		{
			BindSets(commandBuffer, 0, 1);
		}
	}

	void VKPBRShader::BindTextureSets(uint32_t descriptor_index) const
	{
		if (commandBuffer)
		{
			BindSets(commandBuffer, 1, 1, descriptor_index);
		}
	}

	void VKPBRShader::BindObjectSets(uint32_t instance_index) const
	{
		if (commandBuffer)
		{
			BindSets(commandBuffer, 2, 1, instance_index);
		}
	}

	void VKPBRShader::UpdateGlobalUniforms(Renderer* renderer) const
	{
		Camera* camera = renderer->getCamera();
		std::vector<Light*> lights = renderer->getLights();

		//Offset values copied from the shader
		SetUniform("camera", camera->getViewMatrix(), 0);
		SetUniform("camera", camera->getProjectionMatrix(), 64);
		SetUniform("camera", camera->getPosition(), 128);

		struct LightCond
		{
			int highDetailRange, lights;
			float gamma, exposure;
		} lightCond;

		lightCond.highDetailRange = __EngineConfig.rendererHighDetailRange;
		lightCond.lights = (uint32_t)lights.size() >= __EngineConfig.lightsMaxNumber ? __EngineConfig.lightsMaxNumber : (uint32_t)lights.size();
		lightCond.gamma = __EngineConfig.rendererGamma;
		lightCond.exposure = __EngineConfig.rendererExposure;

		/*SetUniformi("lightConditions", __EngineConfig.rendererHighDetailRange, 0, instance_index);
		SetUniformi("lightConditions", (uint32_t)lights.size() >= __EngineConfig.lightsMaxNumber ? __EngineConfig.lightsMaxNumber : (uint32_t)lights.size(), 4, instance_index);
		SetUniformf("lightConditions", __EngineConfig.rendererGamma, 8, instance_index);
		SetUniformf("lightConditions", __EngineConfig.rendererExposure, 12, instance_index);*/
		
		SetUniform("lightConditions", &lightCond, sizeof(LightCond), 0);

		size_t baseOffset = __EngineConfig.lightsMaxNumber * Vector4f::size();
		for (uint32_t i = 0; i < __EngineConfig.lightsMaxNumber; i++)
		{
			size_t currentOffset = Vector4f::size() * i;

			if (i < lights.size())
			{
				Light* light = lights[i];
				
				*(Vector4f*)(uintptr_t(lightData) + (baseOffset * 0 + currentOffset)) = Vector4f(light->getParent()->getWorldTransform().getPosition(), 0);
				*(Vector4f*)(uintptr_t(lightData) + (baseOffset * 1 + currentOffset)) = Vector4f(light->getColour(), 0);
				*(Vector4f*)(uintptr_t(lightData) + (baseOffset * 2 + currentOffset)) = Vector4f(light->getIntensity(), 1, 1, 0);
			}
			else
			{
				*(Vector4f*)(uintptr_t(lightData) + (baseOffset * 0 + currentOffset)) = Vector4f();
				*(Vector4f*)(uintptr_t(lightData) + (baseOffset * 1 + currentOffset)) = Vector4f();
				*(Vector4f*)(uintptr_t(lightData) + (baseOffset * 2 + currentOffset)) = Vector4f();
			}
		}

		SetUniform("lights", lightData, Vector4f::size() * 3 * __EngineConfig.lightsMaxNumber, 0);
	}

	void VKPBRShader::UpdateMaterialUniforms(Material* material, uint32_t descriptor_index) const
	{
		SetTexture(ALBEDO_MAP, material->getTexture(ALBEDO_MAP), descriptor_index);
		SetTexture(NORMAL_MAP, material->getTexture(NORMAL_MAP), descriptor_index);
		SetTexture(MROT_MAP, material->getTexture(MROT_MAP), descriptor_index);
		SetTexture(EMISSION_MAP, material->getTexture(EMISSION_MAP), descriptor_index);
	}

	void VKPBRShader::UpdateObjectUniforms(GameObject* object, uint32_t instance_index) const
	{
		Material* material = ((RendererComponent*)object->GetComponent(RENDERER_COMPONENT))->getMaterial();

		//Offset values are copied from shaders
		SetUniform("m_transform", object->getWorldTransform().getTransformationMatrix(), 0, instance_index);

		SetUniform("material", material->getVector4f(MROT), 0, instance_index);
		SetUniform("material", material->getVector3f(COLOUR), 16, instance_index);
		SetUniform("material", material->getVector3f(EMISSION), 32, instance_index);
		SetUniformi("material", material->exists(NORMAL_MAP), 48, instance_index);
	}
};