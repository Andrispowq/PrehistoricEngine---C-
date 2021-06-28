#include "Includes.hpp"
#include "GLLightCullingPassShader.h"

namespace Prehistoric
{
	GLLightCullingPassShader::GLLightCullingPassShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/forwardPlus/light_culling_CS.glsl"), COMPUTE_SHADER);
		CompileShader();

		AddUniform("depthMap");
		AddUniform("m_view");
		AddUniform("m_projection");
		AddUniform("scrnSize");
		AddUniform("lightCount");
	}

	void GLLightCullingPassShader::UpdateUniforms(Camera* camera, const std::vector<Light*>& lights, Texture* depthMap)
	{
		depthMap->Bind();
		SetUniformi("depthMap", 0);

		SetUniform("m_view", camera->getViewMatrix());
		SetUniform("m_projection", camera->getProjectionMatrix());
		SetUniform("scrnSize", Vector2f(FrameworkConfig::windowWidth, FrameworkConfig::windowHeight));
		SetUniformi("lightCount", lights.size());
	}
};
