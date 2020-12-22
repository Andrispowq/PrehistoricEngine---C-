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
	class PR_API EnvironmentMapRenderer
	{
	public:
		static EnvironmentMapRenderer* instance;

		EnvironmentMapRenderer(Window* window, AssembledAssetManager* manager);
		virtual ~EnvironmentMapRenderer() {}

		void GenerateEnvironmentMap();
		void RenderCube(Camera* camera);

		Texture* getEnvironmentMap() const { return environmentMap; }
		Texture* getIrradianceMap() const { return irradianceMap; }
		Texture* getPrefilterMap() const { return prefilterMap; }
		Texture* getBRDFMap() const { return brdfMap; }

	private:
		Window* window;
		AssembledAssetManager* manager;

		Framebuffer* framebuffer;
		VertexBuffer* cubeBuffer;

		Texture* equirectangularMap;
		Texture* environmentMap = nullptr;
		Texture* irradianceMap = nullptr;
		Texture* prefilterMap = nullptr;
		Texture* brdfMap;

		GLEnvironmentMapShader* environmentMapShader;
		GLIrradianceShader* irradianceShader;
		GLPrefilterShader* prefilterShader;
		GLBRDFIntegrateShader* brdfIntegrateShader;
		GLEnvironmentShader* environmentShader;

		Pipeline* environmentMapPipeline;
		Pipeline* irradiancePipeline;
		Pipeline* prefilterPipeline;
		Pipeline* brdfIntegratePipeline;
		Pipeline* backgroundPipeline;

		Matrix4f viewMatrices[6];
		Matrix4f projectionMatrix;

		//Just so that we have the handles to the resources in the manager:
		size_t cubeID;

		size_t equirectangularMapID;
		size_t environmentMapID;
		size_t irradianceMapID;
		size_t prefilterMapID;
		size_t brdfMapID;

		size_t environmentMapShaderID;
		size_t irradianceShaderID;
		size_t prefilterShaderID;
		size_t brdfIntegrateShaderID;
		size_t environmentShaderID;

		size_t environmentMapPipelineID;
		size_t irradiancePipelineID;
		size_t prefilterPipelineID;
		size_t brdfIntegratePipelineID;
		size_t backgroundPipelineID;
	};
};

#endif