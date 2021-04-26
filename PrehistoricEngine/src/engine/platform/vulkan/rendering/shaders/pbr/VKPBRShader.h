#ifndef VK_PBR_SHADER_H
#define VK_PBR_SHADER_H

#include "platform/vulkan/rendering/shaders/VkShader.h"

#include <vulkan/vulkan.h>

#include "prehistoric/core/node/GameObject.h"
#include "prehistoric/core/model/material/Material.h"
#include "prehistoric/core/node/component/light/Light.h"
#include "prehistoric/core/node/component/renderer/RendererComponent.h"

namespace Prehistoric
{
	class VKPBRShader : public VKShader
	{
	public:
		VKPBRShader(Window* window);
		virtual ~VKPBRShader() {}

		virtual void UpdateGlobalUniforms(Camera* camera, const std::vector<Light*>& lights) const override;
		virtual void UpdateTextureUniforms(Material* material, uint32_t descriptor_index = 0) const override;
		virtual void UpdateObjectUniforms(GameObject* object, uint32_t instance_index = 0) const override;
	};
};

#endif