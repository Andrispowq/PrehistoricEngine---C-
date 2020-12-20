#include "Includes.hpp"
#include "EnvironmentMapRenderer.h"

#include "prehistoric/core/config/EnvironmentMapConfig.h"

#include "platform/opengl/texture/GLTexture.h"
//#include "platform/vulkan/texture/VKTexture.h"

#include "platform/opengl/rendering/framebuffer/GLFramebuffer.h"
//#include "platform/vulkan/rendering/framebuffer/VKFramebuffer.h"

#include "platform/opengl/rendering/pipeline/GLGraphicsPipeline.h"
//#include "platform/vulkan/rendering/pipeline/VKGraphicsPipeline.h"

namespace Prehistoric
{
	EnvironmentMapRenderer* EnvironmentMapRenderer::instance;

	EnvironmentMapRenderer::EnvironmentMapRenderer(Window* window, AssembledAssetManager* manager)
		: window(window), manager(manager)
	{
		//Create the cube mesh
		cubeID = manager->getAssetManager()->getResource<VertexBuffer>("cube.obj");
		cubeBuffer = manager->getAssetManager()->getResourceByID<VertexBuffer>(cubeID);
		cubeBuffer->setFrontFace(FrontFace::DOUBLE_SIDED);

		size_t quadID = manager->getAssetManager()->getResource<VertexBuffer>("quad.obj");
		VertexBuffer* quadBuffer = manager->getAssetManager()->getResourceByID<VertexBuffer>(quadID);
		quadBuffer->setFrontFace(FrontFace::CLOCKWISE);

		//Load the shaders and the pipelines
		environmentMapShader = new GLEnvironmentMapShader();
		environmentMapShaderID = manager->getAssetManager()->addResource<Shader>(environmentMapShader);
		irradianceShader = new GLIrradianceShader();
		irradianceShaderID = manager->getAssetManager()->addResource<Shader>(irradianceShader);
		prefilterShader = new GLPrefilterShader();
		prefilterShaderID = manager->getAssetManager()->addResource<Shader>(prefilterShader);
		brdfIntegrateShader = new GLBRDFIntegrateShader();
		brdfIntegrateShaderID = manager->getAssetManager()->addResource<Shader>(brdfIntegrateShader);
		environmentShader = new GLEnvironmentShader();
		environmentShaderID = manager->getAssetManager()->addResource<Shader>(environmentShader);

		environmentMapPipeline = new GLGraphicsPipeline(window, manager->getAssetManager(), environmentMapShaderID, cubeID);
		environmentMapPipelineID = manager->loadResource<Pipeline>(environmentMapPipeline);
		irradiancePipeline = new GLGraphicsPipeline(window, manager->getAssetManager(), irradianceShaderID, cubeID);
		irradiancePipelineID = manager->loadResource<Pipeline>(irradiancePipeline);
		prefilterPipeline = new GLGraphicsPipeline(window, manager->getAssetManager(), prefilterShaderID, cubeID);
		prefilterPipelineID = manager->loadResource<Pipeline>(prefilterPipeline);
		brdfIntegratePipeline = new GLGraphicsPipeline(window, manager->getAssetManager(), brdfIntegrateShaderID, quadID);
		brdfIntegratePipelineID = manager->loadResource<Pipeline>(brdfIntegratePipeline);
		backgroundPipeline = new GLGraphicsPipeline(window, manager->getAssetManager(), environmentShaderID, cubeID);
		backgroundPipelineID = manager->loadResource<Pipeline>(backgroundPipeline);

		//Create the matrices
		projectionMatrix = Matrix4f::PerspectiveProjection(90, 1, .1f, 100.0f);

		viewMatrices[0] = Matrix4f::View(Vector3f(1, 0, 0), Vector3f(0, -1, 0));
		viewMatrices[1] = Matrix4f::View(Vector3f(-1, 0, 0), Vector3f(0, -1, 0));
		viewMatrices[2] = Matrix4f::View(Vector3f(0, 1, 0), Vector3f(0, 0, -1));
		viewMatrices[3] = Matrix4f::View(Vector3f(0, -1, 0), Vector3f(0, 0, 1));
		viewMatrices[4] = Matrix4f::View(Vector3f(0, 0, -1), Vector3f(0, -1, 0));
		viewMatrices[5] = Matrix4f::View(Vector3f(0, 0, 1), Vector3f(0, -1, 0));

		//Create the original map, and the framebuffer
		equirectangularMap = TextureLoader::LoadTexture(EnvironmentMapConfig::environmentMapLocation, window, Bilinear, ClampToEdge);
		equirectangularMapID = manager->getAssetManager()->addResource<Texture>(equirectangularMap);

		framebuffer = new GLFramebuffer(window);

		//Render the brdf map, which is constant between differnet maps
		uint32_t size = EnvironmentMapConfig::environmentMapResolution;
		brdfMap = GLTexture::Storage2D(size, size, 1, R16G16_LINEAR, Bilinear, ClampToEdge);
		brdfMapID = manager->getAssetManager()->addResource<Texture>(brdfMap);

		framebuffer->Bind();
		framebuffer->addDepthAttachment(size, size);
		window->getSwapchain()->SetWindowSize(size, size);

		framebuffer->addColourAttachment2D(brdfMap);
		framebuffer->Clear(0.0f);

		brdfIntegratePipeline->BindPipeline(nullptr);
		brdfIntegratePipeline->RenderPipeline();
		brdfIntegratePipeline->UnbindPipeline();

		framebuffer->Unbind();
		window->getSwapchain()->SetWindowSize(window->getWidth(), window->getHeight());

		EnvironmentMapConfig::brdfLUT = brdfMap;
		quadBuffer->setFrontFace(FrontFace::COUNTER_CLOCKWISE);
	}

	void EnvironmentMapRenderer::GenerateEnvironmentMap()
	{
		framebuffer->Bind();

		// Rendering the cube map
		uint32_t size = EnvironmentMapConfig::environmentMapResolution;
		framebuffer->addDepthAttachment(size, size);
		window->getSwapchain()->SetWindowSize(size, size);

		environmentMap = GLTexture::Storage3D(size, size, 0, R8G8B8_LINEAR, Trilinear, ClampToEdge, false);
		environmentMapID = manager->getAssetManager()->addResource<Texture>(environmentMap);

		environmentMapPipeline->BindPipeline(nullptr);
		for (int i = 0; i < 6; ++i)
		{
			framebuffer->addColourAttachment3D(environmentMap, i);
			framebuffer->Clear(0.0f);

			environmentMapShader->UpdateUniforms(projectionMatrix, viewMatrices[i], equirectangularMap);
			environmentMapPipeline->RenderPipeline();
		}
		environmentMapPipeline->UnbindPipeline();

		environmentMap->Bind();
		environmentMap->GenerateMipmaps();

		//Rendering the diffuse irradiance map
		size = EnvironmentMapConfig::irradianceMapResolution;
		framebuffer->addDepthAttachment(size, size);
		window->getSwapchain()->SetWindowSize(size, size);

		irradianceMap = GLTexture::Storage3D(size, size, 0, R8G8B8_LINEAR, Bilinear, ClampToEdge, false);
		irradianceMapID = manager->getAssetManager()->addResource<Texture>(irradianceMap);

		irradiancePipeline->BindPipeline(nullptr);
		for (int i = 0; i < 6; ++i)
		{
			framebuffer->addColourAttachment3D(irradianceMap, i);
			framebuffer->Clear(0.0f);

			irradianceShader->UpdateUniforms(projectionMatrix, viewMatrices[i], environmentMap);
			irradiancePipeline->RenderPipeline();
		}
		irradiancePipeline->UnbindPipeline();

		irradianceMap->Bind();

		//Rendering the pre-filter enviroment map
		size = EnvironmentMapConfig::prefilterMapResolution;

		prefilterMap = GLTexture::Storage3D(size, size, 0, R8G8B8_LINEAR, Trilinear, ClampToEdge, true);
		prefilterMapID = manager->getAssetManager()->addResource<Texture>(prefilterMap);

		prefilterPipeline->BindPipeline(nullptr);
		for (int level = 0; level < (int)EnvironmentMapConfig::prefilterLevels; ++level)
		{
			int levelSize = (int)(size * pow(0.5f, level));

			framebuffer->addDepthAttachment(levelSize, levelSize);
			window->getSwapchain()->SetWindowSize(levelSize, levelSize);

			float roughness = (float)level / (EnvironmentMapConfig::prefilterLevels - 1);

			for (int i = 0; i < 6; ++i)
			{
				framebuffer->addColourAttachment3D(prefilterMap, i, 0, level);
				framebuffer->Clear(0.0f);

				prefilterShader->UpdateUniforms(projectionMatrix, viewMatrices[i], environmentMap, roughness);
				prefilterPipeline->RenderPipeline();
			}
		}
		prefilterPipeline->UnbindPipeline();

		prefilterMap->Bind();

		framebuffer->Unbind();
		window->getSwapchain()->SetWindowSize(window->getWidth(), window->getHeight());

		EnvironmentMapConfig::environmentMap = environmentMap;
		EnvironmentMapConfig::irradianceMap = irradianceMap;
		EnvironmentMapConfig::prefilterMap = prefilterMap;
	}

	void EnvironmentMapRenderer::RenderCube(Camera* camera)
	{
		backgroundPipeline->BindPipeline(nullptr);

		environmentShader->UpdateUniforms(camera->getProjectionMatrix(), camera->getViewMatrix(), environmentMap);
		backgroundPipeline->RenderPipeline();

		backgroundPipeline->UnbindPipeline();
	}
};
