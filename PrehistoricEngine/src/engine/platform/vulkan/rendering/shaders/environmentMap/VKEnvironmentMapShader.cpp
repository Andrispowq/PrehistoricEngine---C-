#include "Includes.hpp"
#include "VKEnvironmentMapShader.h"

#include "prehistoric/application/Application.h"

namespace Prehistoric
{
	VKEnvironmentMapShader::VKEnvironmentMapShader(Window* window) : VKShader(window->getContext(), window->getSwapchain())
	{
		AddShader(ResourceLoader::LoadShaderVK("vulkan/basic/environmentMap_CS.spv"), COMPUTE_SHADER);

		AddUniform("data", COMPUTE_SHADER, UniformBuffer, 1, 0, 2 * Vector2f::size());

		AddUniform("environmentMap", FRAGMENT_SHADER, CombinedImageSampler, 0, 0, 0);
		AddUniform("equirectangularMap", FRAGMENT_SHADER, CombinedImageSampler, 0, 1, 0);

		descriptorPool->finalise(pipelineLayout);
	}
	
	void VKEnvironmentMapShader::BindGlobalSets() const
	{
		if (commandBuffer)
		{
			BindSets(commandBuffer, 1, 1);
		}
	}

	void VKEnvironmentMapShader::BindTextureSets(uint32_t descriptor_index) const
	{
		if (commandBuffer)
		{
			BindSets(commandBuffer, 0, 1, descriptor_index);
		}
	}

	void VKEnvironmentMapShader::UpdateUniforms(Vector2f equirectMapRes) const
	{
		SetUniform("data", equirectMapRes, Vector2f::size() * 0);
		SetUniformf("data", (float)__EnvironmentMapConfig.environmentMapResolution, Vector2f::size() * 1);
	}
}