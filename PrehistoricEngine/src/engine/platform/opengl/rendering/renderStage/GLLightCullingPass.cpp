#include "Includes.hpp"
#include "GLLightCullingPass.h"

#include "platform/opengl/rendering/pipeline/GLComputePipeline.h"
#include "platform/opengl/rendering/shaders/forwardPlus/GLLightCullingPassShader.h"
#include "platform/opengl/rendering/GLRenderer.h"

#include "prehistoric/core/node/component/light/Light.h"

namespace Prehistoric
{
	GLLightCullingPass::GLLightCullingPass(Renderer* renderer)
		: RenderStage(renderer), lightBuffer{ nullptr }, visibleLightIndicesBuffer{ nullptr }
	{
		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		uint32_t workGroupsX = (width + (width % 16)) / 16;
		uint32_t workGroupsY = (height + (height % 16)) / 16;

		uint32_t numberOfTiles = workGroupsX * workGroupsY;

		AssetManager* man = manager->getAssetManager();
		lightCullingShader = man->loadShader(ShaderName::LightCullingPass).value();
		lightCullingPipeline = manager->storePipeline(new GLComputePipeline(window, man, lightCullingShader));
		manager->addReference<Pipeline>(lightCullingPipeline.handle);

		lightBuffer = std::make_unique<GLShaderStorageBuffer>(window, nullptr, EngineConfig::lightsMaxNumber * sizeof(InternalLight));
		visibleLightIndicesBuffer = std::make_unique<GLShaderStorageBuffer>(window, nullptr, numberOfTiles * sizeof(VisibleIndex) * 1024);

		static_cast<GLComputePipeline*>(lightCullingPipeline.pointer)->setInvocationSize({ workGroupsX, workGroupsY, 1 });
		static_cast<GLComputePipeline*>(lightCullingPipeline.pointer)->addSSBOBinding(0, (ShaderStorageBuffer*)lightBuffer.get(), READ_ONLY);
		static_cast<GLComputePipeline*>(lightCullingPipeline.pointer)->addSSBOBinding(1, (ShaderStorageBuffer*)visibleLightIndicesBuffer.get(), WRITE_ONLY);
	}

	GLLightCullingPass::~GLLightCullingPass()
	{
		manager->removeReference<Pipeline>(lightCullingPipeline.handle);
	}

	void GLLightCullingPass::OnResized()
	{
		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		uint32_t workGroupsX = (width + (width % 16)) / 16;
		uint32_t workGroupsY = (height + (height % 16)) / 16;

		uint32_t numberOfTiles = workGroupsX * workGroupsY;

		visibleLightIndicesBuffer = std::make_unique<GLShaderStorageBuffer>(window, nullptr, numberOfTiles * sizeof(VisibleIndex) * 1024);

		static_cast<GLComputePipeline*>(lightCullingPipeline.pointer)->removeSSBOBinding(0);
		static_cast<GLComputePipeline*>(lightCullingPipeline.pointer)->removeSSBOBinding(1);
		static_cast<GLComputePipeline*>(lightCullingPipeline.pointer)->setInvocationSize({ workGroupsX, workGroupsY, 1 });
		static_cast<GLComputePipeline*>(lightCullingPipeline.pointer)->addSSBOBinding(0, (ShaderStorageBuffer*)lightBuffer.get(), READ_ONLY);
		static_cast<GLComputePipeline*>(lightCullingPipeline.pointer)->addSSBOBinding(1, (ShaderStorageBuffer*)visibleLightIndicesBuffer.get(), WRITE_ONLY);
	}

	void GLLightCullingPass::Render()
	{
		GLRenderer* rend = (GLRenderer*)renderer;

		lightCullingPipeline->BindPipeline(nullptr);
		static_cast<GLLightCullingPassShader*>(lightCullingPipeline->getShader())->UpdateUniforms(camera, rend->getLights(), rend->getDepthPass()->getDepthTexture().pointer);
		lightCullingPipeline->RenderPipeline();
		lightCullingPipeline->UnbindPipeline();
	}
};
