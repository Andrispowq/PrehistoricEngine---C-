#include "Includes.hpp"
#include "EnvironmentMapRenderer.h"

#include "prehistoric/core/config/EnvironmentMapConfig.h"
#include "prehistoric/core/modules/atmosphere/Atmosphere.h"

#include "platform/opengl/texture/GLTexture.h"
//#include "platform/vulkan/texture/VKTexture.h"

#include "platform/opengl/rendering/framebuffer/GLFramebuffer.h"
//#include "platform/vulkan/rendering/framebuffer/VKFramebuffer.h"

#include "platform/opengl/rendering/pipeline/GLGraphicsPipeline.h"
//#include "platform/vulkan/rendering/pipeline/VKGraphicsPipeline.h"

namespace Prehistoric
{
	EnvironmentMapRenderer* EnvironmentMapRenderer::instance;

	EnvironmentMapRenderer::EnvironmentMapRenderer(Window* window, ResourceStorage* resourceStorage)
		: window(window), resourceStorage(resourceStorage)
	{
		//Create the cube mesh
		cubeBuffer = resourceStorage->loadVertexBuffer(std::nullopt, "cube.obj").value();
		cubeBuffer.pointer->setFrontFace(FrontFace::DOUBLE_SIDED);

		VertexBufferHandle quad = resourceStorage->loadVertexBuffer(std::nullopt, "quad.obj").value();
		quad.pointer->setFrontFace(FrontFace::CLOCKWISE);

		//Load the shaders and the pipelines
		environmentMapShader = resourceStorage->storeShader(new GLEnvironmentMapShader());
		irradianceShader = resourceStorage->storeShader(new GLIrradianceShader());
		prefilterShader = resourceStorage->storeShader(new GLPrefilterShader());
		brdfIntegrateShader = resourceStorage->storeShader(new GLBRDFIntegrateShader());
		environmentShader = resourceStorage->storeShader(new GLEnvironmentShader());
		
		environmentMapPipeline = resourceStorage->storePipeline(new GLGraphicsPipeline(window, resourceStorage, environmentMapShader, cubeBuffer));
		irradiancePipeline = resourceStorage->storePipeline(new GLGraphicsPipeline(window, resourceStorage, irradianceShader, cubeBuffer));
		prefilterPipeline = resourceStorage->storePipeline(new GLGraphicsPipeline(window, resourceStorage, prefilterShader, cubeBuffer));
		brdfIntegratePipeline = resourceStorage->storePipeline(new GLGraphicsPipeline(window, resourceStorage, brdfIntegrateShader, quad));
		backgroundPipeline = resourceStorage->storePipeline(new GLGraphicsPipeline(window, resourceStorage, environmentShader, cubeBuffer));

		//Create the matrices
		projectionMatrix = Matrix4f::PerspectiveProjection(90, 1, .1f, 100.0f);

		viewMatrices[0] = Matrix4f::View(Vector3f(1, 0, 0), Vector3f(0, -1, 0));
		viewMatrices[1] = Matrix4f::View(Vector3f(-1, 0, 0), Vector3f(0, -1, 0));
		viewMatrices[2] = Matrix4f::View(Vector3f(0, 1, 0), Vector3f(0, 0, -1));
		viewMatrices[3] = Matrix4f::View(Vector3f(0, -1, 0), Vector3f(0, 0, 1));
		viewMatrices[4] = Matrix4f::View(Vector3f(0, 0, -1), Vector3f(0, -1, 0));
		viewMatrices[5] = Matrix4f::View(Vector3f(0, 0, 1), Vector3f(0, -1, 0));

		//Create the original map, and the framebuffer
		equirectangularMap = resourceStorage->loadTexture(EnvironmentMapConfig::environmentMapLocation, Bilinear, ClampToEdge).value();

		framebuffer = new GLFramebuffer(window);

		//Render the brdf map, which is constant between differnet maps
		uint32_t size = EnvironmentMapConfig::environmentMapResolution;
		brdfMap = resourceStorage->storeTexture(GLTexture::Storage2D(size, size, 1, R16G16_LINEAR, Bilinear, ClampToEdge));

		framebuffer->Bind();
		framebuffer->addDepthAttachment(size, size);
		window->getSwapchain()->SetWindowSize(size, size);

		framebuffer->addColourAttachment2D(brdfMap.pointer);
		framebuffer->Clear(0.0f);

		brdfIntegratePipeline->BindPipeline(nullptr);
		brdfIntegratePipeline->RenderPipeline();
		brdfIntegratePipeline->UnbindPipeline();

		framebuffer->Unbind();
		window->getSwapchain()->SetWindowSize(window->getWidth(), window->getHeight());

		EnvironmentMapConfig::brdfLUT = brdfMap.pointer;
		quad->setFrontFace(FrontFace::COUNTER_CLOCKWISE);
	}

	void EnvironmentMapRenderer::GenerateEnvironmentMap()
	{
		framebuffer->Bind();

		// Rendering the cube map
		uint32_t size = EnvironmentMapConfig::environmentMapResolution;
		framebuffer->addDepthAttachment(size, size);
		window->getSwapchain()->SetWindowSize(size, size);

		environmentMap = resourceStorage->storeTexture(GLTexture::Storage3D(size, size, 0, R8G8B8_LINEAR, Trilinear, ClampToEdge, false));

		environmentMapPipeline->BindPipeline(nullptr);
		for (int i = 0; i < 6; ++i)
		{
			framebuffer->addColourAttachment3D(environmentMap.pointer, i);
			framebuffer->Clear(0.0f);

			static_cast<GLEnvironmentShader*>(environmentMapShader.pointer)->UpdateUniforms(projectionMatrix, viewMatrices[i], equirectangularMap.pointer);
			environmentMapPipeline->RenderPipeline();
		}
		environmentMapPipeline->UnbindPipeline();

		environmentMap->Bind();
		environmentMap->GenerateMipmaps();

		//Rendering the diffuse irradiance map
		size = EnvironmentMapConfig::irradianceMapResolution;
		framebuffer->addDepthAttachment(size, size);
		window->getSwapchain()->SetWindowSize(size, size);

		irradianceMap = resourceStorage->storeTexture(GLTexture::Storage3D(size, size, 0, R8G8B8_LINEAR, Bilinear, ClampToEdge, false));

		irradiancePipeline->BindPipeline(nullptr);
		for (int i = 0; i < 6; ++i)
		{
			framebuffer->addColourAttachment3D(irradianceMap.pointer, i);
			framebuffer->Clear(0.0f);

			static_cast<GLIrradianceShader*>(irradianceShader.pointer)->UpdateUniforms(projectionMatrix, viewMatrices[i], environmentMap.pointer);
			irradiancePipeline->RenderPipeline();
		}
		irradiancePipeline->UnbindPipeline();

		irradianceMap->Bind();

		//Rendering the pre-filter enviroment map
		size = EnvironmentMapConfig::prefilterMapResolution;

		prefilterMap = resourceStorage->storeTexture(GLTexture::Storage3D(size, size, 0, R8G8B8_LINEAR, Trilinear, ClampToEdge, true));

		prefilterPipeline->BindPipeline(nullptr);
		for (int level = 0; level < (int)EnvironmentMapConfig::prefilterLevels; ++level)
		{
			int levelSize = (int)(size * pow(0.5f, level));

			framebuffer->addDepthAttachment(levelSize, levelSize);
			window->getSwapchain()->SetWindowSize(levelSize, levelSize);

			float roughness = (float)level / (EnvironmentMapConfig::prefilterLevels - 1);

			for (int i = 0; i < 6; ++i)
			{
				framebuffer->addColourAttachment3D(prefilterMap.pointer, i, 0, level);
				framebuffer->Clear(0.0f);

				static_cast<GLPrefilterShader*>(prefilterShader.pointer)->UpdateUniforms(projectionMatrix, viewMatrices[i], environmentMap.pointer, roughness);
				prefilterPipeline->RenderPipeline();
			}
		}
		prefilterPipeline->UnbindPipeline();

		prefilterMap->Bind();

		framebuffer->Unbind();
		window->getSwapchain()->SetWindowSize(window->getWidth(), window->getHeight());

		EnvironmentMapConfig::environmentMap = environmentMap.pointer;
		EnvironmentMapConfig::irradianceMap = irradianceMap.pointer;
		EnvironmentMapConfig::prefilterMap = prefilterMap.pointer;
	}

	void EnvironmentMapRenderer::RenderCube(Camera* camera)
	{
		backgroundPipeline->BindPipeline(nullptr);

		static_cast<GLEnvironmentShader*>(environmentShader.pointer)->UpdateUniforms(camera->getProjectionMatrix(), camera->getViewMatrix(), environmentMap.pointer);
		backgroundPipeline->RenderPipeline();

		backgroundPipeline->UnbindPipeline();
	}
};
