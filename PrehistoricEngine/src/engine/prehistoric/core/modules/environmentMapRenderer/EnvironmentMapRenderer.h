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

#include "prehistoric/core/modules/atmosphere/Atmosphere.h"
#include "prehistoric/core/modules/atmosphere/AtmosphereCapture.h"

namespace Prehistoric
{
	class GLAtmospherePass;
	class EnvironmentMapRenderer
	{
	public:
		static EnvironmentMapRenderer* instance;

		EnvironmentMapRenderer(Window* window, AssembledAssetManager* manager, GLAtmospherePass& atmospherePass);
		virtual ~EnvironmentMapRenderer();

		void UpdateEnvironment(const Atmosphere* atmosphere);
		void RequestBake(bool usePanorama);
		void RenderCube(Camera* camera);

		Texture* getEnvironmentMap() const { return environmentMap.pointer; }
		Texture* getIrradianceMap() const { return irradianceMap.pointer; }
		Texture* getPrefilterMap() const { return prefilterMap.pointer; }
		Texture* getBRDFMap() const { return brdfMap.pointer; }

	public:
		float lodRenderedMap = 0.0;
		bool enabled = true;


	private:
        void GenerateEnvironmentMap(const EnvironmentCaptureSource& source);
        AtmosphereCapture capture;
        bool panoramaOverride = false;
        bool requested = false;
        uint32_t captureDepthSize = 0;
		GLAtmospherePass& atmospherePass;
		Window* window;
		AssembledAssetManager* manager;

		std::string mapCache;

		VertexBufferHandle cubeBuffer;
		Framebuffer* framebuffer;

		Matrix4f viewMatrices[6];
		Matrix4f projectionMatrix;

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
	};
};

#endif