#include "Includes.hpp"
#include "GLBRDFIntegrateShader.h"

namespace Prehistoric
{
	GLBRDFIntegrateShader::GLBRDFIntegrateShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/environmentMap/brdf_CS.glsl"), COMPUTE_SHADER);

		CompileShader();
	}
};
