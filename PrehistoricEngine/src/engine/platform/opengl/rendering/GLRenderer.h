#ifndef GL_RENDERER_H
#define GL_RENDERER_H

#include "prehistoric/common/rendering/Renderer.h"

#include <glad/glad.h>
#include "platform/opengl/rendering/renderStage/GLAtmospherePass.h"

#include "platform/opengl/rendering/framebuffer/GLFramebuffer.h"
#include "platform/opengl/buffer/GLShaderStorageBuffer.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

#include "platform/opengl/rendering/renderStage/GLDepthPass.h"
#include "platform/opengl/rendering/renderStage/GLShadowDepthPass.h"
#include "platform/opengl/rendering/renderStage/GLLightCullingPass.h"
#include "platform/opengl/rendering/renderStage/GLMainPass.h"
#include "platform/opengl/rendering/renderStage/GLBloomPass.h"
#include "platform/opengl/rendering/renderStage/GLVolumetricPostProcessingPass.h"
#include "platform/opengl/rendering/renderStage/GLHDRPass.h"
#include "platform/opengl/rendering/renderStage/GLVoxelRendererPass.h"

#include "platform/opengl/rendering/renderStage/GLRayTracingPass.h"

namespace Prehistoric
{
	class Water;
	class EnvironmentMapRenderer;

	class GLRenderer : public Renderer
	{
	public:
		GLRenderer(Window* window, Camera* camera, AssembledAssetManager* manager);
		virtual ~GLRenderer();

		virtual void PrepareRendering();
		virtual void EndRendering();

		EnvironmentMapRenderer& getEnvironmentLighting() const { return *environmentLighting; }
		void DrawBackground(Camera* camera);

		virtual Texture* getOutputTexture() const override { return hdrPass->getOutputImage().pointer; }

        void RegisterWater(Water* water) { waters.push_back(water); }
        void DrawModelBatches(const ModelBatches& batches);
		GLAtmospherePass& getAtmospherePass() { return atmospherePass; }
		GLDepthPass* getDepthPass() const { return depthPass.get(); }
		GLShadowDepthPass* getShadowDepthPass() const { return shadowDepthPass.get(); }
		GLLightCullingPass* getLightCullingPass() const { return lightCullingPass.get(); }
		GLMainPass* getMainPass() const { return mainPass.get(); }
		GLBloomPass* getBloomPass() const { return bloomPass.get(); }
		GLVolumetricPostProcessingPass* getVolumetricPostProcessingPass() const { return volumetricPostProcessingPass.get(); }
		GLHDRPass* getHDRPass() const { return hdrPass.get(); }
		GLVoxelRendererPass* getVoxelPass() const { return voxelPass.get(); }

		GLRayTracingPass* getRayTracingPass() const { return rtxPass.get(); }

	private:
		void UpdateEnvironmentLighting() override;
		void RenderScene() override;
		void RenderHDR() override;
		void FinishFrame() override;

		std::vector<Water*> waters;
		GLAtmospherePass atmospherePass;
		std::unique_ptr<EnvironmentMapRenderer> environmentLighting;
		std::unique_ptr<GLDepthPass> depthPass;
		std::unique_ptr<GLShadowDepthPass> shadowDepthPass;
		std::unique_ptr<GLLightCullingPass> lightCullingPass;
		std::unique_ptr<GLMainPass> mainPass;
		std::unique_ptr<GLBloomPass> bloomPass;
		std::unique_ptr<GLVolumetricPostProcessingPass> volumetricPostProcessingPass;
		std::unique_ptr<GLHDRPass> hdrPass;
		std::unique_ptr<GLVoxelRendererPass> voxelPass;

		std::unique_ptr<GLRayTracingPass> rtxPass;

		VertexBufferHandle quad;
		ShaderHandle renderShader;
		PipelineHandle renderPipeline;
	};
};

#endif