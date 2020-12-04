#ifndef GL_ATMOSPHERE_SHADER_H
#define GL_ATMOSPHERE_SHADER_H

#include "engine/platform/opengl/rendering/shaders/GLShader.h"

#include <glew.h>

#include "engine/prehistoric/core/config/AtmosphereConfig.h"
#include "engine/prehistoric/core/config/FrameworkConfig.h"

class GLAtmosphereShader : public GLShader
{
public:
	GLAtmosphereShader();
	virtual ~GLAtmosphereShader() {}

	virtual void UpdateShaderUniforms(Camera* camera, const std::vector<Light*>& lights, uint32_t instance_index = 0) const override;
	virtual void UpdateObjectUniforms(GameObject* object, uint32_t instance_index = 0) const override;
};

#endif