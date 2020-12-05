#ifndef GL_TERRAIN_SHADER_H
#define GL_TERRAIN_SHADER_H

#include "engine/prehistoric/core/util/Includes.hpp"

#include <glew.h>

#include "engine/platform/opengl/rendering/shaders/GLShader.h"

namespace Prehistoric
{
	class GLTerrainShader : public GLShader
	{
	public:
		GLTerrainShader();
		virtual ~GLTerrainShader() {}

		virtual void UpdateShaderUniforms(Camera* camera, const std::vector<Light*>& lights, uint32_t instance_index = 0) const override;
		virtual void UpdateSharedUniforms(GameObject* object, uint32_t instance_index = 0) const override;
		virtual void UpdateObjectUniforms(GameObject* object, uint32_t instance_index = 0) const override;

	private:
		int location_localMatrix;
		int location_worldMatrix;

		int location_location;
		int location_index;
		int location_gap;
		int location_lod;
	};
};

#endif