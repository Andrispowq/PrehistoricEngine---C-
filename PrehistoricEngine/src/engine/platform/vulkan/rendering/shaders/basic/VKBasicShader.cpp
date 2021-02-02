#include "Includes.hpp"
#include "VKBasicShader.h"

namespace Prehistoric
{
	VKBasicShader::VKBasicShader(Window* window) : VKShader(window->getContext(), window->getSwapchain())
	{
		AddShader(ResourceLoader::LoadShaderVK("vulkan/basic/basic_VS.spv"), VERTEX_SHADER);
		AddShader(ResourceLoader::LoadShaderVK("vulkan/basic/basic_FS.spv"), FRAGMENT_SHADER);

		AddUniform("camera", VERTEX_SHADER | FRAGMENT_SHADER, UniformBuffer, 0, 0, 2 * sizeof(float) * 16 + Vector3f::size());
		AddUniform("model", VERTEX_SHADER, UniformBuffer, 1, 0, sizeof(float) * 16);
		AddUniform("lights", FRAGMENT_SHADER, UniformBuffer, 0, 1, EngineConfig::lightsMaxNumber * 3 * Vector4f::size());
		AddUniform("lightConditions", FRAGMENT_SHADER, UniformBuffer, 0, 2, sizeof(float) * 2);
		AddUniform("material", FRAGMENT_SHADER, UniformBuffer, 1, 1, Vector3f::size() + 4 * sizeof(float));

		AddUniform("albedoMap", FRAGMENT_SHADER, CombinedImageSampler, 1, 2, 0);
		AddUniform("mrotMap", FRAGMENT_SHADER, CombinedImageSampler, 1, 3, 0);

		descriptorPool->finalise(pipelineLayout);
	}

	void VKBasicShader::UpdateShaderUniforms(Camera* camera, const std::vector<Light*>& lights, uint32_t instance_index) const
	{
		SetUniform("camera", camera->getViewMatrix(), 16 * sizeof(float) * 0, instance_index);
		SetUniform("camera", camera->getProjectionMatrix(), 16 * sizeof(float) * 1, instance_index);
		SetUniform("camera", camera->getPosition(), 16 * sizeof(float) * 2, instance_index);

		SetUniformf("lightConditions", EngineConfig::rendererExposure, sizeof(float) * 0, instance_index);
		SetUniformf("lightConditions", EngineConfig::rendererGamma, sizeof(float) * 1, instance_index);

		size_t baseOffset = EngineConfig::lightsMaxNumber * Vector4f::size();

		for (uint32_t i = 0; i < EngineConfig::lightsMaxNumber; i++)
		{
			size_t currentOffset = Vector4f::size() * i;

			if (i < lights.size())
			{
				Light* light = lights[i];

				SetUniform("lights", Vector4f(light->getParent()->getWorldTransform().getPosition(), 0), baseOffset * 0 + currentOffset, instance_index);
				SetUniform("lights", Vector4f(light->getColour(), 0), baseOffset * 1 + currentOffset, instance_index);
				SetUniform("lights", Vector4f(light->getIntensity(), 0), baseOffset * 2 + currentOffset, instance_index);
			}
			else
			{
				SetUniform("lights", Vector4f(), baseOffset * 0 + currentOffset, instance_index);
				SetUniform("lights", Vector4f(), baseOffset * 1 + currentOffset, instance_index);
				SetUniform("lights", Vector4f(), baseOffset * 2 + currentOffset, instance_index);
			}
		}

		if (commandBuffer)
			BindSets(commandBuffer, 0, 1, instance_index);
	}

	void VKBasicShader::UpdateObjectUniforms(GameObject* object, uint32_t instance_index) const
	{
		SetUniform("model", object->getWorldTransform().getTransformationMatrix(), instance_index);

		Material* material = ((RendererComponent*)object->GetComponent(RENDERER_COMPONENT))->getMaterial();

		SetTexture(ALBEDO_MAP, material->getTexture(ALBEDO_MAP), instance_index);
		SetTexture(MROT_MAP, material->getTexture(MROT_MAP), instance_index);

		SetUniform("material", material->getVector3f(COLOUR), instance_index);
		SetUniform("material", material->getVector4f(MROT), Vector4f::size(), instance_index);

		if (commandBuffer)
			BindSets(commandBuffer, 1, 1, instance_index);
	}
};
