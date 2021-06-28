#include "Includes.hpp"
#include "GLHDRShader.h"

#include "prehistoric/core/config/EnvironmentMapConfig.h"
#include "platform/opengl/rendering/GLRenderer.h"

#include "prehistoric/core/node/GameObject.h"

namespace Prehistoric
{
	GLHDRShader::GLHDRShader() : GLShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/postProcessing/hdr_CS.glsl"), COMPUTE_SHADER);
		CompileShader();

		AddUniform("scene");

		AddUniform("gamma");
		AddUniform("exposure");
	}

	void GLHDRShader::UpdateUniforms(Texture* scene) const
	{
		scene->Bind();
		SetUniformi("scene", 0);

		SetUniformf("gamma", EngineConfig::rendererGamma);
		SetUniformf("exposure", EngineConfig::rendererExposure);
	}
};