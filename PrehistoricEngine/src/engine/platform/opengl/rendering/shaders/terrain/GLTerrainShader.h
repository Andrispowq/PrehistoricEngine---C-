#ifndef GL_TERRAIN_SHADER_H
#define GL_TERRAIN_SHADER_H

#include "Includes.hpp"

#include <glad/glad.h>

#include "platform/opengl/rendering/shaders/GLShader.h"

namespace Prehistoric
{
	class GLTerrainShader : public GLShader
	{
	public:
		GLTerrainShader();
		virtual ~GLTerrainShader() {}

		virtual void UpdateGlobalUniforms(Camera* camera, const std::vector<Light*>& lights) const override;
		virtual void UpdateMaterialUniforms(Material* material, uint32_t descriptor_index = 0) const override;
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