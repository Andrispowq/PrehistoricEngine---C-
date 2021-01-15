#include "Includes.hpp"
#include "EnvironmentMapRenderer.h"

#include "prehistoric/core/config/EnvironmentMapConfig.h"
#include "prehistoric/core/modules/atmosphere/Atmosphere.h"

#include "platform/opengl/texture/GLTexture.h"
//#include "platform/vulkan/texture/VKTexture.h"

#include "platform/opengl/rendering/framebuffer/GLFramebuffer.h"
//#include "platform/vulkan/rendering/framebuffer/VKFramebuffer.h"

#include "platform/opengl/rendering/pipeline/GLComputePipeline.h"
//#include "platform/vulkan/rendering/pipeline/VKComputePipeline.h"

namespace Prehistoric
{
	EnvironmentMapRenderer* EnvironmentMapRenderer::instance;

	EnvironmentMapRenderer::EnvironmentMapRenderer(Window* window, AssembledAssetManager* manager)
		: window(window), manager(manager)
	{
		AssetManager* man = manager->getAssetManager();

		//Create the cube mesh
		cubeBuffer = man->loadVertexBuffer(std::nullopt, "res/models/cube.obj").value();
		cubeBuffer.pointer->setFrontFace(FrontFace::DOUBLE_SIDED);

		//Load the shaders and the pipelines
		environmentMapShader = man->storeShader(new GLEnvironmentMapShader());
		irradianceShader = man->storeShader(new GLIrradianceShader());
		prefilterShader = man->storeShader(new GLPrefilterShader());
		brdfIntegrateShader = man->storeShader(new GLBRDFIntegrateShader());
		environmentShader = man->storeShader(new GLEnvironmentShader());
		
		environmentMapPipeline = manager->storePipeline(new GLComputePipeline(window, man, environmentMapShader));
		irradiancePipeline = manager->storePipeline(new GLComputePipeline(window, man, irradianceShader));
		prefilterPipeline = manager->storePipeline(new GLComputePipeline(window, man, prefilterShader));
		brdfIntegratePipeline = manager->storePipeline(new GLComputePipeline(window, man, brdfIntegrateShader));
		backgroundPipeline = manager->storePipeline(new GLGraphicsPipeline(window, man, environmentShader, cubeBuffer));

		uint32_t size = 512;
		brdfMap = man->storeTexture(GLTexture::Storage2D(size, size, 1, R16G16_LINEAR, Bilinear, ClampToEdge));
		manager->addReference<Texture>(brdfMap.handle);
		static_cast<GLComputePipeline*>(brdfIntegratePipeline.pointer)->setInvocationSize({ size / 16, size / 16, 1 });
		static_cast<GLComputePipeline*>(brdfIntegratePipeline.pointer)->addTextureBinding(0, brdfMap.pointer, WRITE_ONLY);

		//Render the brdf map, which is constant between differnet maps
		brdfIntegratePipeline->BindPipeline(nullptr);
		brdfIntegratePipeline->RenderPipeline();
		brdfIntegratePipeline->UnbindPipeline();

		brdfMap->Bind();

		EnvironmentMapConfig::brdfLUT = brdfMap.pointer;
	}

	void EnvironmentMapRenderer::GenerateEnvironmentMap()
	{
		AssetManager* man = manager->getAssetManager();

		if (equirectangularMap.pointer != nullptr)
		{
			man->removeReference<Texture>(equirectangularMap.handle);
			man->removeReference<Texture>(environmentMap.handle);
			man->removeReference<Texture>(irradianceMap.handle);
			man->removeReference<Texture>(prefilterMap.handle);

			static_cast<GLComputePipeline*>(environmentMapPipeline.pointer)->removeTextureBinding(0);
			static_cast<GLComputePipeline*>(environmentMapPipeline.pointer)->removeTextureBinding(1);

			static_cast<GLComputePipeline*>(irradiancePipeline.pointer)->removeTextureBinding(0);
			static_cast<GLComputePipeline*>(prefilterPipeline.pointer)->removeTextureBinding(0);
		}

		equirectangularMap = man->loadTexture(EnvironmentMapConfig::environmentMapLocation, Bilinear, ClampToEdge).value();

		uint32_t size = EnvironmentMapConfig::environmentMapResolution;
		environmentMap = man->storeTexture(GLTexture::Storage3D(size, size, 1, R8G8B8A8_LINEAR, Trilinear, ClampToEdge, true));
		manager->addReference<Texture>(environmentMap.handle);
		static_cast<GLComputePipeline*>(environmentMapPipeline.pointer)->setInvocationSize({ size / 16, size / 16, 6 });
		static_cast<GLComputePipeline*>(environmentMapPipeline.pointer)->addTextureBinding(0, environmentMap.pointer, WRITE_ONLY);
		static_cast<GLComputePipeline*>(environmentMapPipeline.pointer)->addTextureBinding(1, equirectangularMap.pointer, READ_ONLY);

		size = EnvironmentMapConfig::irradianceMapResolution;
		irradianceMap = man->storeTexture(GLTexture::Storage3D(size, size, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false));
		manager->addReference<Texture>(irradianceMap.handle);
		static_cast<GLComputePipeline*>(irradiancePipeline.pointer)->setInvocationSize({ size / 16, size / 16, 6 });
		static_cast<GLComputePipeline*>(irradiancePipeline.pointer)->addTextureBinding(0, irradianceMap.pointer, WRITE_ONLY);

		size = EnvironmentMapConfig::prefilterMapResolution;
		prefilterMap = man->storeTexture(GLTexture::Storage3D(size, size, EnvironmentMapConfig::prefilterLevels, R8G8B8A8_LINEAR, Trilinear, ClampToEdge, true));
		manager->addReference<Texture>(prefilterMap.handle);

		// Rendering the cube map
		environmentMapPipeline->BindPipeline(nullptr);
		static_cast<GLEnvironmentMapShader*>(environmentMapShader.pointer)->UpdateUniforms(Vector2f((float)equirectangularMap->getWidth(), (float)equirectangularMap->getHeight()));
		environmentMapPipeline->RenderPipeline();
		environmentMapPipeline->UnbindPipeline();

		environmentMap->Bind();
		environmentMap->GenerateMipmaps();

		//Rendering the diffuse irradiance map
		irradiancePipeline->BindPipeline(nullptr);
		static_cast<GLIrradianceShader*>(irradianceShader.pointer)->UpdateUniforms(environmentMap.pointer);
		irradiancePipeline->RenderPipeline();
		irradiancePipeline->UnbindPipeline();

		irradianceMap->Bind();

		//Rendering the pre-filter enviroment map
		for (int level = 0; level < (int)EnvironmentMapConfig::prefilterLevels; ++level)
		{
			int levelSize = (int)(EnvironmentMapConfig::prefilterMapResolution * pow(0.5f, level));
			float roughness = (float)level / (EnvironmentMapConfig::prefilterLevels - 1);

			if(level > 0)
				static_cast<GLComputePipeline*>(prefilterPipeline.pointer)->removeTextureBinding(0);

			static_cast<GLComputePipeline*>(prefilterPipeline.pointer)->setInvocationSize({ (uint32_t)levelSize / 16, (uint32_t)levelSize / 16, 6 });
			static_cast<GLComputePipeline*>(prefilterPipeline.pointer)->addTextureBinding(0, prefilterMap.pointer, WRITE_ONLY, (size_t)level);

			prefilterPipeline->BindPipeline(nullptr);
			static_cast<GLPrefilterShader*>(prefilterShader.pointer)->UpdateUniforms(environmentMap.pointer, roughness, (float)levelSize);
			prefilterPipeline->RenderPipeline();
		}
		prefilterPipeline->UnbindPipeline();

		prefilterMap->Bind();

		EnvironmentMapConfig::environmentMap = environmentMap.pointer;
		EnvironmentMapConfig::irradianceMap = irradianceMap.pointer;
		EnvironmentMapConfig::prefilterMap = prefilterMap.pointer;
	}

	void EnvironmentMapRenderer::RenderCube(Camera* camera)
	{
		backgroundPipeline->BindPipeline(nullptr);
		static_cast<GLEnvironmentShader*>(environmentShader.pointer)->UpdateUniforms(camera->getProjectionMatrix(), camera->getViewMatrix(), prefilterMap.pointer, lodRenderedMap);
		backgroundPipeline->RenderPipeline();
		backgroundPipeline->UnbindPipeline();
	}
};
