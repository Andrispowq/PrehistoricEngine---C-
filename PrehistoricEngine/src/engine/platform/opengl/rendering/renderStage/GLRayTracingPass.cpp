#include "Includes.hpp"
#include "GLRayTracingPass.h"

#include "platform/opengl/rendering/GLRenderer.h"
#include "platform/opengl/rendering/shaders/rayTracing/GLRayTracingShader.h"

#include "prehistoric/application/Application.h"
#include "prehistoric/core/modules/environmentMapRenderer/EnvironmentMapRenderer.h"

namespace Prehistoric
{
	GLRayTracingPass::GLRayTracingPass(Renderer* renderer)
		: RenderStage(renderer)
	{
		GLRenderer* rend = (GLRenderer*)renderer;

		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		uint32_t workGroupsX = (width + (width % 16)) / 16;
		uint32_t workGroupsY = (height + (height % 16)) / 16;

		uint32_t numberOfTiles = workGroupsX * workGroupsY;

		AssetManager* man = manager->getAssetManager();
		outputImage = man->storeTexture(GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false));
		man->addReference<Texture>(outputImage.handle);

		rayTracingShader = man->loadShader(ShaderName::RayTracing).value();

		rayTracingPipeline = manager->createPipeline(PipelineTypeHashFlags::RayTracing, rayTracingShader);
		manager->addReference<Pipeline>(rayTracingPipeline.handle);

		static_cast<GLRayTracingPipeline*>(rayTracingPipeline.pointer)->setInvocationSize({ workGroupsX, workGroupsY, 1 });
		static_cast<GLRayTracingPipeline*>(rayTracingPipeline.pointer)->addTextureBinding(0, /*outputImage.pointer*/rend->getMainPass()->getColourImage().pointer, (AccessMask)RayTracingAccessFlags::READ_WRITE);
	}

	GLRayTracingPass::~GLRayTracingPass()
	{
		AssetManager* man = manager->getAssetManager();
		manager->removeReference<Pipeline>(rayTracingPipeline.handle);

		man->removeReference<Texture>(outputImage.handle);
	}

	void GLRayTracingPass::OnResized()
	{
		GLRenderer* rend = (GLRenderer*)renderer;

		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		uint32_t workGroupsX = (width + (width % 16)) / 16;
		uint32_t workGroupsY = (height + (height % 16)) / 16;

		uint32_t numberOfTiles = workGroupsX * workGroupsY;

		AssetManager* man = manager->getAssetManager();
		man->removeReference<Texture>(outputImage.handle);
		outputImage = man->storeTexture(GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false));
		man->addReference<Texture>(outputImage.handle);

		static_cast<GLRayTracingPipeline*>(rayTracingPipeline.pointer)->removeTextureBinding(0);
		static_cast<GLRayTracingPipeline*>(rayTracingPipeline.pointer)->setInvocationSize({ workGroupsX, workGroupsY, 1 });
		static_cast<GLRayTracingPipeline*>(rayTracingPipeline.pointer)->addTextureBinding(0, /*outputImage.pointer*/rend->getMainPass()->getColourImage().pointer, (AccessMask)RayTracingAccessFlags::READ_WRITE);
	}

	void GLRayTracingPass::Render()
	{
		PR_PROFILE("Main pass");

		if (camera->isChanged())
		{
			current_spp = 1;
		}

		rayTracingPipeline->BindPipeline(nullptr);
		static_cast<GLRayTracingShader*>(rayTracingPipeline->getShader())->UpdateUniforms(renderer, current_spp++);
		rayTracingPipeline->RenderPipeline();
		rayTracingPipeline->UnbindPipeline();
	}
}
