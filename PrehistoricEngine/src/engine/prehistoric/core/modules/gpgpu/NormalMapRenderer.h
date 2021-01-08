#ifndef NORMAL_MAP_RENDERER_H
#define NORMAL_MAP_RENDERER_H

#include "platform/opengl/texture/GLTexture.h"
#include "platform/vulkan/texture/VKTexture.h"

#include "platform/opengl/rendering/pipeline/GLComputePipeline.h"
//#include "platform/vulkan/rendering/pipeline/VKComputePipeline.h"

#include "platform/opengl/rendering/shaders/gpgpu/GLNormalMapShader.h"
//#include "platform/vulkan/rendering/shaders/gpgpu/VKNormalMapShader.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

namespace Prehistoric
{
	class NormalMapRenderer
	{
	public:
		NormalMapRenderer(Window* window, AssembledAssetManager* manager, float strength, uint32_t N);
		virtual ~NormalMapRenderer();

		void Render(Texture* heightmap);

		TextureHandle getNormalmap() const { return normalmap; }
	private:
		Window* window;
		AssembledAssetManager* manager;

		PipelineHandle pipeline;
		TextureHandle normalmap;

		float strength;
		uint32_t N;
	};
};

#endif