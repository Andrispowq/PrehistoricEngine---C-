#ifndef GL_RENDERER_H
#define GL_RENDERER_H

#include "prehistoric/common/rendering/Renderer.h"

#include <glad/glad.h>

#include "prehistoric/core/resources/AssembledAssetManager.h"

#include "platform/opengl/rendering/framebuffer/GLFramebuffer.h"

namespace Prehistoric
{
	class GLRenderer : public Renderer
	{
	public:
		GLRenderer(Window* window, Camera* camera, AssembledAssetManager* manager);
		virtual ~GLRenderer();

		virtual void PrepareRendering();
		virtual void EndRendering();

		virtual void Render() override;

		Texture* getPositionMetallic() const { return positionMetalic; }
		Texture* getAlbedoRoughness() const { return albedoRoughness; }
		Texture* getNormalLit() const { return normalLit; }
		Texture* getEmissionExtra() const { return emissionExtra; }

	private:
		std::unique_ptr<GLFramebuffer> deferredFBO;

		Texture* positionMetalic; //RGB -> position, A -> metallic
		Texture* albedoRoughness; //RGB -> albedo, A -> roughness
		Texture* normalLit; //RGB -> normal, A -> whether that pixel should be shaded (makes sure that the atmosphere is not shaded)
		Texture* emissionExtra; //RGB -> emission, A -> some extra value which might be needed later on (perhaps object ID)

		size_t vboID;
		size_t shaderID;
		Pipeline* pipeline;

		AssembledAssetManager* manager;
	};
};

#endif