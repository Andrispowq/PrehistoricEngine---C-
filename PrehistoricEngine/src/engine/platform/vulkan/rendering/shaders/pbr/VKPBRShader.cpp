#include "Includes.hpp"
#include <glad/glad.h>
#include "VKPBRShader.h" 

namespace Prehistoric
{
	VKPBRShader::VKPBRShader(Window* window) : VKShader(window->getContext(), window->getSwapchain())
	{
		AddShader(ResourceLoader::LoadShaderVK("vulkan/pbr/pbr_VS.spv"), VERTEX_SHADER);
		AddShader(ResourceLoader::LoadShaderVK("vulkan/pbr/pbr_FS.spv"), FRAGMENT_SHADER);

		AddUniform("camera", VERTEX_SHADER | FRAGMENT_SHADER, UniformBuffer, 0, 0, sizeof(float) * 16 * 2 + Vector4f::size());
		AddUniform("lightConditions", VERTEX_SHADER | FRAGMENT_SHADER, UniformBuffer, 0, 1, sizeof(int) * 2 + sizeof(float) * 2);
		AddUniform("lights", VERTEX_SHADER | FRAGMENT_SHADER, UniformBuffer, 0, 2, Vector4f::size() * 3 * EngineConfig::lightsMaxNumber);

		AddUniform(ALBEDO_MAP, FRAGMENT_SHADER, CombinedImageSampler, 1, 0, 0, nullptr);
		AddUniform(NORMAL_MAP, FRAGMENT_SHADER, CombinedImageSampler, 1, 1, 0, nullptr);
		AddUniform(MROT_MAP, FRAGMENT_SHADER, CombinedImageSampler, 1, 2, 0, nullptr);
		AddUniform(EMISSION_MAP, FRAGMENT_SHADER, CombinedImageSampler, 1, 3, 0, nullptr);

		AddUniform("m_transform", VERTEX_SHADER, UniformBuffer, 2, 0, sizeof(float) * 16);
		AddUniform("material", FRAGMENT_SHADER, UniformBuffer, 2, 1, Vector4f::size() * 2 + sizeof(int) + sizeof(float) * 4);

		descriptorPool->finalise(pipelineLayout);
	}

	void VKPBRShader::UpdateGlobalUniforms(Camera* camera, const std::vector<Light*>& lights) const
	{
		//Offset values are copied from shaders
		SetUniform("camera", camera->getViewMatrix(), 0);
		SetUniform("camera", camera->getProjectionMatrix(), 64);
		SetUniform("camera", camera->getPosition(), 128);

		struct LightCond
		{
			int highDetailRange, lights;
			float gamma, exposure;
		} lightCond;

		lightCond.highDetailRange = EngineConfig::rendererHighDetailRange;
		lightCond.lights = (uint32_t)lights.size() >= EngineConfig::lightsMaxNumber ? EngineConfig::lightsMaxNumber : (uint32_t)lights.size();
		lightCond.gamma = EngineConfig::rendererGamma;
		lightCond.exposure = EngineConfig::rendererExposure;

		/*SetUniformi("lightConditions", EngineConfig::rendererHighDetailRange, 0, instance_index);
		SetUniformi("lightConditions", (uint32_t)lights.size() >= EngineConfig::lightsMaxNumber ? EngineConfig::lightsMaxNumber : (uint32_t)lights.size(), 4, instance_index);
		SetUniformf("lightConditions", EngineConfig::rendererGamma, 8, instance_index);
		SetUniformf("lightConditions", EngineConfig::rendererExposure, 12, instance_index);*/
		
		SetUniform("lightConditions", &lightCond, sizeof(LightCond), 0);

		void* lightData = new char[Vector4f::size() * 3 * EngineConfig::lightsMaxNumber];

		size_t baseOffset = EngineConfig::lightsMaxNumber * Vector4f::size();

		for (uint32_t i = 0; i < EngineConfig::lightsMaxNumber; i++)
		{
			size_t currentOffset = Vector4f::size() * i;

			if (i < lights.size())
			{
				Light* light = lights[i];
				
				*(Vector4f*)(uintptr_t(lightData) + (baseOffset * 0 + currentOffset)) = Vector4f(light->getParent()->getWorldTransform().getPosition(), 0);
				*(Vector4f*)(uintptr_t(lightData) + (baseOffset * 1 + currentOffset)) = Vector4f(light->getColour(), 0);
				*(Vector4f*)(uintptr_t(lightData) + (baseOffset * 2 + currentOffset)) = Vector4f(light->getIntensity(), 0);

				/*SetUniform("lights", Vector4f(light->getParent()->getWorldTransform().getPosition(), 0), baseOffset * 0 + currentOffset, instance_index);
				SetUniform("lights", Vector4f(light->getColour(), 0), baseOffset * 1 + currentOffset, instance_index);
				SetUniform("lights", Vector4f(light->getIntensity(), 0), baseOffset * 2 + currentOffset, instance_index);*/
			}
			else
			{
				*(Vector4f*)(uintptr_t(lightData) + (baseOffset * 0 + currentOffset)) = Vector4f();
				*(Vector4f*)(uintptr_t(lightData) + (baseOffset * 1 + currentOffset)) = Vector4f();
				*(Vector4f*)(uintptr_t(lightData) + (baseOffset * 2 + currentOffset)) = Vector4f();

				/*SetUniform("lights", Vector4f(), baseOffset * 0 + currentOffset, instance_index);
				SetUniform("lights", Vector4f(), baseOffset * 1 + currentOffset, instance_index);
				SetUniform("lights", Vector4f(), baseOffset * 2 + currentOffset, instance_index);*/
			}
		}

		SetUniform("lights", lightData, Vector4f::size() * 3 * EngineConfig::lightsMaxNumber, 0);

		delete[] lightData;

		BindSets(commandBuffer, 0, 1);
	}

	void VKPBRShader::UpdateTextureUniforms(Material* material, uint32_t descriptor_index) const
	{
		SetTexture(ALBEDO_MAP, material->getTexture(ALBEDO_MAP), descriptor_index);
		SetTexture(NORMAL_MAP, material->getTexture(NORMAL_MAP), descriptor_index);
		SetTexture(MROT_MAP, material->getTexture(MROT_MAP), descriptor_index);
		SetTexture(EMISSION_MAP, material->getTexture(EMISSION_MAP), descriptor_index);

		BindSets(commandBuffer, 1, 1, descriptor_index);
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

		BindSets(commandBuffer, 2, 1, instance_index);
	}
};