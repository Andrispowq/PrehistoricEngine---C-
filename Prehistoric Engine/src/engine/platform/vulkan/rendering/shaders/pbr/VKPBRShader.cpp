#include "engine/prehistoric/core/util/Includes.hpp"
#include <glew.h>
#include "VKPBRShader.h" 

VKPBRShader::VKPBRShader(Window* window) : VKShader(window->getContext(), window->getSwapchain())
{
	AddShader(ResourceLoader::LoadShaderVK("vulkan/pbr/pbr_VS.spv"), VERTEX_SHADER);
	AddShader(ResourceLoader::LoadShaderVK("vulkan/pbr/pbr_FS.spv"), FRAGMENT_SHADER);

	AddUniform("camera", VERTEX_SHADER | FRAGMENT_SHADER, UniformBuffer, 0, 0, sizeof(float) * 16 * 2 + Vector4f::size());
	AddUniform("lightConditions", VERTEX_SHADER | FRAGMENT_SHADER, UniformBuffer, 0, 1, sizeof(int) * 2 + sizeof(float) * 2);
	AddUniform("lights", VERTEX_SHADER | FRAGMENT_SHADER, UniformBuffer, 0, 2, Vector4f::size() * 3 * EngineConfig::lightsMaxNumber);

	AddUniform("material", GEOMETRY_SHADER | FRAGMENT_SHADER, UniformBuffer, 1, 0, Vector4f::size() * 2 + sizeof(int) + sizeof(float) * 4);

	AddUniform(ALBEDO_MAP, FRAGMENT_SHADER, CombinedImageSampler, 1, 1, 0, nullptr);
	AddUniform(DISPLACEMENT_MAP, FRAGMENT_SHADER, CombinedImageSampler, 1, 2, 0, nullptr);
	AddUniform(NORMAL_MAP, FRAGMENT_SHADER, CombinedImageSampler, 1, 3, 0, nullptr);
	AddUniform(METALLIC_MAP, FRAGMENT_SHADER, CombinedImageSampler, 1, 4, 0, nullptr);
	AddUniform(ROUGHNESS_MAP, FRAGMENT_SHADER, CombinedImageSampler, 1, 5, 0, nullptr);
	AddUniform(OCCLUSION_MAP, FRAGMENT_SHADER, CombinedImageSampler, 1, 6, 0, nullptr);
	AddUniform(EMISSION_MAP, FRAGMENT_SHADER, CombinedImageSampler, 1, 7, 0, nullptr);

	AddUniform("m_transform", VERTEX_SHADER, UniformBuffer, 2, 0, sizeof(float) * 16);

	descriptorPool->finalise(pipelineLayout);
}

void VKPBRShader::UpdateShaderUniforms(Camera* camera, const std::vector<Light*>& lights, uint32_t instance_index) const
{
	//Offset values are copied from shaders
	SetUniform("camera", camera->getViewMatrix(), 0, instance_index);
	SetUniform("camera", camera->getProjectionMatrix(), 64, instance_index);
	SetUniform("camera", camera->getPosition(), 128, instance_index);

	SetUniformi("lightConditions", EngineConfig::rendererHighDetailRange, 0, instance_index);
	SetUniformi("lightConditions", (uint32_t)lights.size() >= EngineConfig::lightsMaxNumber ? EngineConfig::lightsMaxNumber : (uint32_t)lights.size(), 4, instance_index);
	SetUniformf("lightConditions", EngineConfig::rendererGamma, 8, instance_index);
	SetUniformf("lightConditions", EngineConfig::rendererExposure, 12, instance_index);

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

	BindSets(commandBuffer, 0, 1, instance_index);
}

void VKPBRShader::UpdateObjectUniforms(GameObject* object, uint32_t instance_index) const
{
	Material* material = ((RendererComponent*)object->GetComponent(RENDERER_COMPONENT))->getMaterial();

	//Offset values are copied from shaders
	SetUniform("m_transform", object->getWorldTransform().getTransformationMatrix(), instance_index);

	SetUniform("material", material->getVector3f(COLOUR), 0, instance_index);
	SetUniform("material", material->getVector3f(EMISSION), 16, instance_index);
	SetUniformi("material", material->exists(NORMAL_MAP), 32, instance_index);
	SetUniformf("material", material->getFloat(HEIGHT_SCALE), 36, instance_index);
	SetUniformf("material", material->getFloat(METALLIC), 40, instance_index);
	SetUniformf("material", material->getFloat(ROUGHNESS), 44, instance_index);
	SetUniformf("material", material->getFloat(OCCLUSION), 48, instance_index);

	SetTexture(ALBEDO_MAP, material->getTexture(ALBEDO_MAP), instance_index);
	SetTexture(DISPLACEMENT_MAP, material->getTexture(DISPLACEMENT_MAP), instance_index);
	SetTexture(NORMAL_MAP, material->getTexture(NORMAL_MAP), instance_index);
	SetTexture(METALLIC_MAP, material->getTexture(METALLIC_MAP), instance_index);
	SetTexture(ROUGHNESS_MAP, material->getTexture(ROUGHNESS_MAP), instance_index);
	SetTexture(OCCLUSION_MAP, material->getTexture(OCCLUSION_MAP), instance_index);
	SetTexture(EMISSION_MAP, material->getTexture(EMISSION_MAP), instance_index);

	BindSets(commandBuffer, 1, 2, instance_index);
}