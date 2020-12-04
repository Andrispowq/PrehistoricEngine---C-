#include "engine/prehistoric/core/util/Includes.hpp"
#include "GLBRDFIntegrateShader.h"

GLBRDFIntegrateShader::GLBRDFIntegrateShader()
{
	AddShader(ResourceLoader::LoadShaderGL("opengl/environmentMap/brdfIntegrate_VS.glsl"), VERTEX_SHADER);
	AddShader(ResourceLoader::LoadShaderGL("opengl/environmentMap/brdfIntegrate_FS.glsl"), FRAGMENT_SHADER);

	CompileShader();
}
