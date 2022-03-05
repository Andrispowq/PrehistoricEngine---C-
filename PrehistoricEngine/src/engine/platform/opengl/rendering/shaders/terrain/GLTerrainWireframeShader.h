#ifndef GL_TERRAIN_WIREFRAME_SHADER_H
#define GL_TERRAIN_WIREFRAME_SHADER_H

#include "platform/opengl/rendering/shaders/GLShader.h"

#include <glad/glad.h>

#include "prehistoric/common/rendering/Renderer.h"

namespace Prehistoric
{
	class GLTerrainWireframeShader : public GLShader
	{
	public:
		GLTerrainWireframeShader();
		virtual ~GLTerrainWireframeShader() {}

		virtual void UpdateGlobalUniforms(Renderer* renderer) const override;
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