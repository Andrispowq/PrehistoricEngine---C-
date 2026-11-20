#ifndef GL_VOXELISATION_SHADER_H
#define GL_VOXELISATION_SHADER_H

#include "platform/opengl/rendering/shaders/GLShader.h"

#include <glad/glad.h>

#include "prehistoric/common/rendering/Renderer.h"

#include "prehistoric/core/node/GameObject.h"
#include "prehistoric/core/node/movement/Camera.h"
#include "prehistoric/core/node/component/renderer/RendererComponent.h"
#include "prehistoric/core/model/material/Material.h"
#include "platform/opengl/texture/GLTexture.h"

namespace Prehistoric
{
	class GLVoxelisationShader : public GLShader
	{
	public:
		GLVoxelisationShader();
		virtual ~GLVoxelisationShader() {}

		virtual void UpdateGlobalUniforms(Renderer* renderer) const override;
		virtual void UpdateMaterialUniforms(Material* material, uint32_t descriptor_index = 0) const override;
		virtual void UpdateObjectUniforms(GameObject* object, uint32_t instance_index = 0) const override;

	private:
		int m_transform;

		int albedoMap;
		int normalMap;
		int mrotMap;

		int colour;
		int usesNormalMap;
		int mrot;
		int emission;
	};
};

#endif