#ifndef NORMAL_MAP_RENDERER_H
#define NORMAL_MAP_RENDERER_H

#include "engine/platform/opengl/texture/GLTexture.h"
#include "engine/platform/vulkan/texture/VKTexture.h"

#include "engine/platform/opengl/rendering/pipeline/GLComputePipeline.h"
//#include "engine/platform/vulkan/rendering/pipeline/VKComputePipeline.h"

#include "engine/platform/opengl/rendering/shaders/gpgpu/GLNormalMapShader.h"
//#include "engine/platform/vulkan/rendering/shaders/gpgpu/VKNormalMapShader.h"

namespace Prehistoric
{
	class AssembledAssetManager;

	class NormalMapRenderer
	{
	public:
		NormalMapRenderer(Window* window, AssembledAssetManager* manager, float strength, uint32_t N);
		virtual ~NormalMapRenderer();

		void Render(Texture* heightmap);

		Texture* getNormalmap() const { return normalmap; }
	private:
		Window* window;
		AssembledAssetManager* manager;

		size_t pipelineID;
		size_t textureID;

		Pipeline* pipeline;
		Texture* normalmap;

		float strength;
		uint32_t N;
	};
};

#endif