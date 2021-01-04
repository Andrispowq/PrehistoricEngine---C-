#ifndef SPLAT_MAP_RENDERER_H
#define SPLAT_MAP_RENDERER_H

#include "platform/opengl/texture/GLTexture.h"
#include "platform/vulkan/texture/VKTexture.h"

#include "platform/opengl/rendering/pipeline/GLComputePipeline.h"
//#include "platform/vulkan/rendering/pipeline/VKComputePipeline.h"

#include "platform/opengl/rendering/shaders/gpgpu/GLSplatMapShader.h"
//#include "platform/vulkan/rendering/shaders/gpgpu/VKSplatMapShader.h"

namespace Prehistoric
{
	class AssembledAssetManager;

	class SplatMapRenderer
	{
	public:
		SplatMapRenderer(Window* window, ResourceStorage* resourceStorage, uint32_t N);
		virtual ~SplatMapRenderer();

		void Render(Texture* normalmap);

		TextureHandle getSplatmap() { return splatmap; }
	private:
		Window* window;
		ResourceStorage* resourceStorage;

		PipelineHandle pipeline;
		TextureHandle splatmap;

		uint32_t N;
	};
};

#endif