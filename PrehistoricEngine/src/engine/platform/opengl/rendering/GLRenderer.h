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

		Texture* getAlphaCoverage() const { return alphaCoverage; }
		Texture* getOutputTexture() const { return outputImage; }
		Texture* getFXAATexture() const { return fxaaTexture; }

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

		Texture* alphaCoverage;

		/* ---------- Deferred Configurations ---------- 

			Lit: 0.0 -> there is no shading on the object
				Extra: 1.0 -> the emission part is interpreted as the light scattering value
			Lit: 0.5 -> the object is not shaded, but exposure and gamma correction is applied on it
			Lit: 1.0 -> the object is fully shaded
				Extra: this value represents the occlusion value of that pixel

		   ---------- Deferred Configurations ---------- */

		size_t quadVBO;
		size_t alphaCoverageShader;
		
		size_t deferredShader;
		size_t fxaaShader;
		size_t renderShader;

		Texture* outputImage;
		Texture* fxaaTexture;

		Pipeline* alphaCoveragePipeline;

		Pipeline* deferredPipeline;
		Pipeline* fxaaPipeline;
		Pipeline* renderPipeline;

		AssembledAssetManager* manager;
	};
};

#endif