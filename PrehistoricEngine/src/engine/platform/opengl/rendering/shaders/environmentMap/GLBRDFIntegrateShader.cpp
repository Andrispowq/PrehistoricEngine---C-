#include "Includes.hpp"
#include "GLBRDFIntegrateShader.h"

namespace Prehistoric
{
	GLBRDFIntegrateShader::GLBRDFIntegrateShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/environmentMap/brdfIntegrate_VS.glsl"), VERTEX_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/environmentMap/brdfIntegrate_FS.glsl"), FRAGMENT_SHADER);

		CompileShader();
	}
};
