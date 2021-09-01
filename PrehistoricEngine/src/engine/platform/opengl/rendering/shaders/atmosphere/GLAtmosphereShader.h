#ifndef GL_ATMOSPHERE_SHADER_H
#define GL_ATMOSPHERE_SHADER_H

#include "platform/opengl/rendering/shaders/GLShader.h"

#include <glad/glad.h>

#include "prehistoric/core/config/AtmosphereConfig.h"
#include "prehistoric/core/config/FrameworkConfig.h"

namespace Prehistoric
{
	class GLAtmosphereShader : public GLShader
	{
	public:
		GLAtmosphereShader();
		virtual ~GLAtmosphereShader() {}

		void UpdateUniforms(Matrix4f view, Matrix4f projection) const;
		virtual void UpdateGlobalUniforms(Camera* camera, const std::vector<Light*>& lights) const override;
		virtual void UpdateObjectUniforms(GameObject* object, uint32_t instance_index = 0) const override;
	};
};

#endif