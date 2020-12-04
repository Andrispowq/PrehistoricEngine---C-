#ifndef VK_BASIC_SHADER_H
#define VK_BASIC_SHADER_H

#include "engine/platform/vulkan/rendering/shaders/VkShader.h"

#include <vulkan/vulkan.h>

#include "engine/prehistoric/core/node/GameObject.h"
#include "engine/prehistoric/core/model/material/Material.h"
#include "engine/prehistoric/core/node/component/light/Light.h"
#include "engine/prehistoric/core/node/component/renderer/RendererComponent.h"

class VKBasicShader : public VKShader
{
public:
	VKBasicShader(Window* window);
	virtual ~VKBasicShader() {}

	virtual void UpdateShaderUniforms(Camera* camera, const std::vector<Light*>& lights, uint32_t instance_index = 0) const override;
	virtual void UpdateObjectUniforms(GameObject* object, uint32_t instance_index = 0) const override;
};

#endif