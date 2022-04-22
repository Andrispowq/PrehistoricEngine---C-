#ifndef GL_LIGHT_CULLING_PASS_H
#define GL_LIGHT_CULLING_PASS_H

#include "prehistoric/common/rendering/renderStage/RenderStage.h"
#include "prehistoric/common/rendering/pipeline/Pipeline.h"

#include "platform/opengl/rendering/pipeline/GLGraphicsPipeline.h"
#include "platform/opengl/buffer/GLShaderStorageBuffer.h"

namespace Prehistoric
{
	class AssembledAssetManager;
	class Renderer;

	class GLLightCullingPass : public RenderStage
	{
	public:
		GLLightCullingPass(Renderer* renderer);
		virtual ~GLLightCullingPass();

		virtual void OnResized() override;
		virtual void Render() override;

		void GLLightCullingPass::UpdateLightBuffer();

		GLShaderStorageBuffer* getLightBuffer() const { return lightBuffer.get(); }
		GLShaderStorageBuffer* getVisibleLightIndicesBuffer() const { return visibleLightIndicesBuffer.get(); }
	private:
		std::unique_ptr<GLShaderStorageBuffer> lightBuffer;
		std::unique_ptr<GLShaderStorageBuffer> visibleLightIndicesBuffer;

		ShaderHandle lightCullingShader;
		PipelineHandle lightCullingPipeline;
	};
};

#endif