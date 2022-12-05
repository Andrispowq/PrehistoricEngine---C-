#ifndef VK_ENVIRONMENT_MAP_SHADER_H
#define VK_ENVIRONMENT_MAP_SHADER_H

#include "platform/vulkan/rendering/shaders/VkShader.h"

#include <vulkan/vulkan.h>

#include "prehistoric/common/texture/Texture.h"

namespace Prehistoric
{
	class VKEnvironmentMapShader : public VKShader
	{
	public:
		VKEnvironmentMapShader(Window* window);
		virtual ~VKEnvironmentMapShader() {}

		virtual void BindGlobalSets() const;
		virtual void BindTextureSets(uint32_t descriptor_index = 0) const;

		void UpdateUniforms(Vector2f equirectMapRes) const;
	};
}

#endif