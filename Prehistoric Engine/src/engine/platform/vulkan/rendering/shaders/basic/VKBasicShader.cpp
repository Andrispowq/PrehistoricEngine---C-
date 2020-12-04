#include "engine/prehistoric/core/util/Includes.hpp"
#include <glew.h>
#include "VKBasicShader.h"

VKBasicShader::VKBasicShader(Window* window) : VKShader(window->getContext(), window->getSwapchain())
{
	AddShader(ResourceLoader::LoadShaderVK("vulkan/basic/basic_VS.spv"), VERTEX_SHADER);
	AddShader(ResourceLoader::LoadShaderVK("vulkan/basic/basic_FS.spv"), FRAGMENT_SHADER);
	
	AddUniform("camera", VERTEX_SHADER | FRAGMENT_SHADER, UniformBuffer, 0, 0, 2 * sizeof(float) * 16 + Vector3f::size());
	AddUniform("model", VERTEX_SHADER, UniformBuffer, 0, 1, sizeof(float) * 16);
	AddUniform("lights", FRAGMENT_SHADER, UniformBuffer, 0, 2, EngineConfig::lightsMaxNumber * 3 * Vector4f::size());
	AddUniform("material", FRAGMENT_SHADER, UniformBuffer, 0, 3, Vector3f::size() + 4 * sizeof(float));

	AddUniform("albedoMap", FRAGMENT_SHADER, CombinedImageSampler, 0, 4, 0);
	AddUniform("metallicMap", FRAGMENT_SHADER, CombinedImageSampler, 0, 5, 0);
	AddUniform("roughnessMap", FRAGMENT_SHADER, CombinedImageSampler, 0, 6, 0);

	AddUniform("gamma", FRAGMENT_SHADER, UniformBuffer, 0, 7, sizeof(float));

	descriptorPool->finalise(pipelineLayout);
}

void VKBasicShader::UpdateShaderUniforms(Camera* camera, const std::vector<Light*>& lights, uint32_t instance_index) const
{
	SetUniform("camera", camera->getViewMatrix(), 16 * sizeof(float) * 0, instance_index);
	SetUniform("camera", camera->getProjectionMatrix(), 16 * sizeof(float) * 1, instance_index);
	SetUniform("camera", camera->getPosition(), 16 * sizeof(float) * 2, instance_index);

	SetUniformf("gamma", EngineConfig::rendererGamma, instance_index);

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
}

void VKBasicShader::UpdateObjectUniforms(GameObject* object, uint32_t instance_index) const
{
	SetUniform("model", object->getWorldTransform().getTransformationMatrix(), instance_index);

	Material* material = ((RendererComponent*)object->GetComponent(RENDERER_COMPONENT))->getMaterial();

	SetTexture(ALBEDO_MAP, material->getTexture(ALBEDO_MAP), instance_index);
	SetTexture(METALLIC_MAP, material->getTexture(METALLIC_MAP), instance_index);
	SetTexture(ROUGHNESS_MAP, material->getTexture(ROUGHNESS_MAP), instance_index);
	
	SetUniform("material", material->getVector3f(COLOUR), instance_index);
	SetUniformf("material", material->getFloat(METALLIC), Vector3f::size() + sizeof(float), instance_index);
	SetUniformf("material", material->getFloat(ROUGHNESS), Vector3f::size() + 2 * sizeof(float), instance_index);
	SetUniformf("material", EngineConfig::rendererGamma, Vector3f::size() + 3 * sizeof(float), instance_index);

	BindSets(commandBuffer, 0, 2, instance_index);
}
