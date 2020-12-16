#ifndef GL_TERRAIN_HEIGHTS_SHADER_H
#define GL_TERRAIN_HEIGHTS_SHADER_H

#include "platform/opengl/rendering/shaders/GLShader.h"

#include <glad/glad.h>

namespace Prehistoric
{
	class GLTerrainHeightsShader : public GLShader
	{
	public:
		GLTerrainHeightsShader();
		virtual ~GLTerrainHeightsShader() {}

		void UpdateUniforms(Texture* texture, uint32_t N) const;
	};
};

#endif