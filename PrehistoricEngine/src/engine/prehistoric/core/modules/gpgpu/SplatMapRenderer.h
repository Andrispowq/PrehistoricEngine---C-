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
		SplatMapRenderer(Window* window, AssembledAssetManager* manager, uint32_t N);
		virtual ~SplatMapRenderer();

		void Render(Texture* normalmap);

		Texture* getSplatmap() { return splatmap; }
	private:
		Window* window;
		AssembledAssetManager* manager;

		size_t pipelineID;
		size_t textureID;

		Pipeline* pipeline;
		Texture* splatmap;

		uint32_t N;
	};
};

#endif