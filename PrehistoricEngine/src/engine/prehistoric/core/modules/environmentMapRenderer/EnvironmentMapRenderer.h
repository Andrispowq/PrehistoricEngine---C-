#ifndef ENVIROMENT_RENDERER_H
#define ENVIROMENT_RENDERER_H

#include "prehistoric/common/rendering/framebuffer/Framebuffer.h"
#include "prehistoric/common/texture/Texture.h"
#include "prehistoric/common/buffer/MeshVertexBuffer.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

#include "platform/opengl/rendering/shaders/environmentMap/GLBRDFIntegrateShader.h"
#include "platform/opengl/rendering/shaders/environmentMap/GLEnvironmentShader.h"
#include "platform/opengl/rendering/shaders/environmentMap/GLEnvironmentMapShader.h"
#include "platform/opengl/rendering/shaders/environmentMap/GLIrradianceShader.h"
#include "platform/opengl/rendering/shaders/environmentMap/GLPrefilterShader.h"

namespace Prehistoric
{
	class EnvironmentMapRenderer
	{
	public:
		static EnvironmentMapRenderer* instance;

		EnvironmentMapRenderer(Window* window, AssembledAssetManager* manager);
		virtual ~EnvironmentMapRenderer() {}

		void GenerateEnvironmentMap();
		void RenderCube(Camera* camera);

		Texture* getEnvironmentMap() const { return environmentMap.pointer; }
		Texture* getIrradianceMap() const { return irradianceMap.pointer; }
		Texture* getPrefilterMap() const { return prefilterMap.pointer; }
		Texture* getBRDFMap() const { return brdfMap.pointer; }

	private:
		Window* window;
		AssembledAssetManager* manager;

		Framebuffer* framebuffer;
		VertexBufferHandle cubeBuffer;

		TextureHandle equirectangularMap;
		TextureHandle environmentMap;
		TextureHandle irradianceMap;
		TextureHandle prefilterMap;
		TextureHandle brdfMap;

		ShaderHandle environmentMapShader;
		ShaderHandle irradianceShader;
		ShaderHandle prefilterShader;
		ShaderHandle brdfIntegrateShader;
		ShaderHandle environmentShader;

		PipelineHandle environmentMapPipeline;
		PipelineHandle irradiancePipeline;
		PipelineHandle prefilterPipeline;
		PipelineHandle brdfIntegratePipeline;
		PipelineHandle backgroundPipeline;

		Matrix4f viewMatrices[6];
		Matrix4f projectionMatrix;
	};
};

#endif