#ifndef GL_BASIC_SHADER_H
#define GL_BASIC_SHADER_H

#include "platform/opengl/rendering/shaders/GLShader.h"

#include <glad/glad.h>

#include "prehistoric/core/node/GameObject.h"
#include "prehistoric/core/node/movement/Camera.h"
#include "prehistoric/core/node/component/renderer/RendererComponent.h"
#include "prehistoric/core/model/material/Material.h"
#include "prehistoric/common/texture/Texture.h"

#include "prehistoric/core/config/EngineConfig.h"

namespace Prehistoric
{
	class GLBasicShader : public GLShader
	{
	public:
		GLBasicShader();
		virtual ~GLBasicShader() {}

		virtual void UpdateGlobalUniforms(Camera* camera, const std::vector<Light*>& lights) const override;
		virtual void UpdateTextureUniforms(Material* material, uint32_t descriptor_index = 0) const override;
		virtual void UpdateObjectUniforms(GameObject* object, uint32_t instance_index = 0) const override;

	private:
		int m_transform;

		int albedoMap;
		int mrotMap;

		int colour;
		int mrot;
	};
};

#endif