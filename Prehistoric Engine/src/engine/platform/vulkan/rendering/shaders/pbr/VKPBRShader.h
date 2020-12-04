#ifndef VK_PBR_SHADER_H
#define VK_PBR_SHADER_H

#include "engine/platform/vulkan/rendering/shaders/VkShader.h"

#include <vulkan/vulkan.h>

#include "engine/prehistoric/core/gameObject/GameObject.h"
#include "engine/prehistoric/core/model/material/Material.h"
#include "engine/prehistoric/component/light/Light.h"
#include "engine/prehistoric/component/renderer/RendererComponent.h"

class VKPBRShader : public VKShader
{
public:
	VKPBRShader(Window* window);
	virtual ~VKPBRShader() {}

	virtual void UpdateShaderUniforms(Camera* camera, const std::vector<Light*>& lights, uint32_t instance_index = 0) const override;
	virtual void UpdateObjectUniforms(GameObject* object, uint32_t instance_index = 0) const override;
};

#endif