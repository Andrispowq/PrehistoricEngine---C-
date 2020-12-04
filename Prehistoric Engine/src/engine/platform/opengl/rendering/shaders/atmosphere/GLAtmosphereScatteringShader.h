#ifndef GL_ATMOSPHERE_SCATTERING_SHADER_H
#define GL_ATMOSPHERE_SCATTERING_SHADER_H

#include "engine/platform/opengl/rendering/shaders/GLShader.h"

#include <glew.h>

#include "engine/config/AtmosphereConfig.h"
#include "engine/config/FrameworkConfig.h"

class GLAtmosphereScatteringShader : public GLShader
{
public:
	GLAtmosphereScatteringShader();
	virtual ~GLAtmosphereScatteringShader() {}

	virtual void UpdateShaderUniforms(Camera* camera, const std::vector<Light*>& lights, uint32_t instance_index = 0) const override;
	virtual void UpdateObjectUniforms(GameObject* object, uint32_t instance_index = 0) const override;
};

#endif