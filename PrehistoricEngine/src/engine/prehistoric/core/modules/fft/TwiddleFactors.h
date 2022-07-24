#ifndef TWIDDLE_FACTORS_H
#define TWIDDLE_FACTORS_H

#include "prehistoric/common/texture/Texture.h"
#include "prehistoric/common/rendering/shaders/Shader.h"
#include "prehistoric/common/buffer/ShaderStorageBuffer.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

#include "prehistoric/application/Application.h"

namespace Prehistoric
{
	class TwiddleFactors
	{
	public:
		TwiddleFactors(AssembledAssetManager* manager, Window* window, int N);
		~TwiddleFactors();

		void Render();

		Texture* getTexture() const { return texture.pointer; }

	private:
		std::vector<uint32_t> InitBitReversedIndices();

	private:
		Window* window = nullptr;
		AssembledAssetManager* manager = nullptr;

		ShaderHandle shader;
		TextureHandle texture;
		PipelineHandle pipeline;

		ShaderStorageBuffer* bitReversedSSBO = nullptr;
		int N;
		int log_2_N;

		uint32_t* data = nullptr;
		size_t data_size = 0;
	};
}

#endif