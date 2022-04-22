#ifndef GL_VOLUMETRIC_POST_PROCESSING_SHADER_H
#define GL_VOLUMETRIC_POST_PROCESSING_SHADER_H

#include "platform/opengl/rendering/shaders/GLShader.h"

#include <glad/glad.h>

namespace Prehistoric
{
	class GLVolumetricPostProcessingShader : public GLShader
	{
	public:
		GLVolumetricPostProcessingShader();
		virtual ~GLVolumetricPostProcessingShader() {}

		void UpdateUniforms(Texture* scene, Texture* positionMetallic, Texture* albedoRoughness, Texture* normal, Camera* camera, std::vector<Light*>& lights) const;
	};
};

#endif