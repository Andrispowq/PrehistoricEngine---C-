#ifndef GL_PBR_SHADER_H
#define GL_PBR_SHADER_H

#include "engine/platform/opengl/rendering/shaders/GLShader.h"

#include <glew.h>

#include "engine/prehistoric/core/node/GameObject.h"
#include "engine/prehistoric/core/node/movement/Camera.h"
#include "engine/prehistoric/core/node/component/renderer/RendererComponent.h"
#include "engine/prehistoric/core/model/material/Material.h"
#include "engine/platform/opengl/texture/GLTexture.h"

#include "engine/prehistoric/core/config/EngineConfig.h"

class GLPBRShader : public GLShader
{
public:
	GLPBRShader();
	virtual ~GLPBRShader() {}

	virtual void UpdateShaderUniforms(Camera* camera, const std::vector<Light*>& lights, uint32_t instance_index = 0) const override;
	virtual void UpdateObjectUniforms(GameObject* object, uint32_t instance_index = 0) const override;

private:
	int m_transform;

	int albedoMap;
	int normalMap;
	int displacementMap;
	int metallicMap;
	int roughnessMap;
	int occlusionMap;
	int emissionMap;

	int colour;
	int usesNormalMap;
	int heightScale;
	int metallic;
	int roughness;
	int ambientOcclusion;
	int emission;
};

#endif