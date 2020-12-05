#ifndef GL_ATMOSPHERE_SCATTERING_SHADER_H
#define GL_ATMOSPHERE_SCATTERING_SHADER_H

#include "platform/opengl/rendering/shaders/GLShader.h"

#include <glew.h>

#include "prehistoric/core/config/AtmosphereConfig.h"
#include "prehistoric/core/config/FrameworkConfig.h"

namespace Prehistoric
{
	class GLAtmosphereScatteringShader : public GLShader
	{
	public:
		GLAtmosphereScatteringShader();
		virtual ~GLAtmosphereScatteringShader() {}

		virtual void UpdateShaderUniforms(Camera* camera, const std::vector<Light*>& lights, uint32_t instance_index = 0) const override;
		virtual void UpdateObjectUniforms(GameObject* object, uint32_t instance_index = 0) const override;
	};
};

#endif