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

		virtual void BindGlobalSets() const;
		virtual void BindTextureSets(uint32_t descriptor_index = 0) const;
		virtual void BindObjectSets(uint32_t instance_index = 0) const;

		virtual void UpdateGlobalUniforms(Camera* camera, const std::vector<Light*>& lights) const override;
		virtual void UpdateMaterialUniforms(Material* material, uint32_t descriptor_index = 0) const override;
		virtual void UpdateObjectUniforms(GameObject* object, uint32_t instance_index = 0) const override;
	};
};

#endif