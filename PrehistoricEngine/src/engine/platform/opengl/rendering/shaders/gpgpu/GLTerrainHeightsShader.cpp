#include "engine/prehistoric/core/util/Includes.hpp"
#include "GLTerrainHeightsShader.h"

namespace Prehistoric
{
	GLTerrainHeightsShader::GLTerrainHeightsShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/gpgpu/terrainHeights_CS.glsl"), COMPUTE_SHADER);
		CompileShader();

		AddUniform("heightmap");
		AddUniform("N");
	}

	void GLTerrainHeightsShader::UpdateUniforms(Texture* texture, uint32_t N) const
	{
		texture->Bind();
		SetUniformi("heightmap", 0);
		SetUniformi("N", N);
	}
};
