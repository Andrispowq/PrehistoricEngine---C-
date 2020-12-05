#ifndef VK_BASIC_SHADER_H
#define VK_BASIC_SHADER_H

#include "platform/vulkan/rendering/shaders/VkShader.h"

#include <vulkan/vulkan.h>

#include "prehistoric/core/node/GameObject.h"
#include "prehistoric/core/model/material/Material.h"
#include "prehistoric/core/node/component/light/Light.h"
#include "prehistoric/core/node/component/renderer/RendererComponent.h"

namespace Prehistoric
{
	class VKBasicShader : public VKShader
	{
	public:
		VKBasicShader(Window* window);
		virtual ~VKBasicShader() {}

		virtual void UpdateShaderUniforms(Camera* camera, const std::vector<Light*>& lights, uint32_t instance_index = 0) const override;
		virtual void UpdateObjectUniforms(GameObject* object, uint32_t instance_index = 0) const override;
	};
};

#endif