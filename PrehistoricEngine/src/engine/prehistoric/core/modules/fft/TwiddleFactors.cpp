#include "Includes.hpp"
#include "TwiddleFactors.h"

#include "platform/opengl/buffer/GLShaderStorageBuffer.h"

#include "platform/opengl/rendering/shaders/fft/GLTwiddleFactorsShader.h"

namespace Prehistoric
{
	template<typename T>
	T binary_reverse(T number)
	{
		T reverse = 0;
		size_t bytes = sizeof(T);

		for (size_t i = bytes * 8; i-- > 0; number >>= 1)
		{
			reverse = (reverse << 1) | (number & 1);
		}

		return reverse;
	}

	TwiddleFactors::TwiddleFactors(AssembledAssetManager* manager, Window* window, int N)
		: manager(manager), window(window), N(N)
	{
		log_2_N = (int)(std::log(N) / std::log(2));

		std::vector<uint32_t> data_vec = InitBitReversedIndices();
		data_size = data_vec.size();
		data = new uint32_t[data_size];
		memcpy(data, &data_vec[0], data_size * sizeof(uint32_t));

		if (__FrameworkConfig.api == API::OpenGL)
		{
			bitReversedSSBO = new GLShaderStorageBuffer(window, data, data_size * sizeof(uint32_t));
		}

		AssetManager* man = manager->getAssetManager();
		shader = man->loadShader(ShaderName::TwiddleFactors).value();
		texture = man->storeTexture(GLTexture::Storage2D(log_2_N, N, 1, R8G8B8A8_LINEAR, Bilinear));

		uint32_t invocation_size = (uint32_t)N / 16;

		pipeline = manager->createPipeline(PipelineTypeHashFlags::Compute, shader);
		manager->addReference<Pipeline>(pipeline.handle);
		dynamic_cast<GLComputePipeline*>(pipeline.pointer)->addSSBOBinding(1, bitReversedSSBO, (AccessMask)ComputeAccessFlags::READ_ONLY);
		dynamic_cast<GLComputePipeline*>(pipeline.pointer)->addTextureBinding(0, texture.pointer, (AccessMask)ComputeAccessFlags::WRITE_ONLY);
		dynamic_cast<GLComputePipeline*>(pipeline.pointer)->setInvocationSize({ (uint32_t)log_2_N, invocation_size, 1 });
	}

	TwiddleFactors::~TwiddleFactors()
	{
		delete[] data;

		manager->removeReference<Pipeline>(pipeline.handle);
	}

	void TwiddleFactors::Render()
	{
		pipeline->BindPipeline(nullptr);
		dynamic_cast<GLTwiddleFactorsShader*>(shader.pointer)->UpdateUniforms(N);
		pipeline->RenderPipeline();
		pipeline->UnbindPipeline();
	}

	std::vector<uint32_t> TwiddleFactors::InitBitReversedIndices()
	{
		std::vector<uint32_t> bitReversedIndices;
		uint32_t bits = (uint32_t)(std::log(N) / std::log(2));

		for (uint32_t i = 0; i < N; i++)
		{
			uint32_t x = binary_reverse(i);
			uint32_t rot = (x << bits) | (x >> (32 - bits));
			bitReversedIndices.push_back(rot);
		}

		return bitReversedIndices;
	}
}
