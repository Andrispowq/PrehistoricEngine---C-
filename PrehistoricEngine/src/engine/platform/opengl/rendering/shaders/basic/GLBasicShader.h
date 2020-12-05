#ifndef GL_BASIC_SHADER_H
#define GL_BASIC_SHADER_H

#include "engine/platform/opengl/rendering/shaders/GLShader.h"

#include <glew.h>

#include "engine/prehistoric/core/node/GameObject.h"
#include "engine/prehistoric/core/node/movement/Camera.h"
#include "engine/prehistoric/core/node/component/renderer/RendererComponent.h"
#include "engine/prehistoric/core/model/material/Material.h"
#include "engine/prehistoric/common/texture/Texture.h"

#include "engine/prehistoric/core/config/EngineConfig.h"

namespace Prehistoric
{
	class GLBasicShader : public GLShader
	{
	public:
		GLBasicShader();
		virtual ~GLBasicShader() {}

		virtual void UpdateShaderUniforms(Camera* camera, const std::vector<Light*>& lights, uint32_t instance_index = 0) const override;
		virtual void UpdateObjectUniforms(GameObject* object, uint32_t instance_index = 0) const override;

	private:
		int m_transform;

		int albedoMap;
		int metallicMap;
		int roughnessMap;

		int colour;
		int metallic;
		int roughness;
	};
};

#endif