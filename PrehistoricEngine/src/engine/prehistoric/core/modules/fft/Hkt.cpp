#include "Includes.hpp"
#include "Hkt.h"

#include "platform/opengl/rendering/shaders/fft/GLHktShader.h"

namespace Prehistoric
{
	Hkt::Hkt(AssembledAssetManager* manager, Window* window, int N, int L)
		: manager(manager), window(window), N(N), L(L)
	{
		AssetManager* man = manager->getAssetManager();
		shader = man->loadShader(ShaderName::Hkt).value();

		imageDyCoefficients = man->storeTexture(GLTexture::Storage2D(N, N, 1, R8G8B8A8_LINEAR, Bilinear));
		imageDxCoefficients = man->storeTexture(GLTexture::Storage2D(N, N, 1, R8G8B8A8_LINEAR, Bilinear));
		imageDzCoefficients = man->storeTexture(GLTexture::Storage2D(N, N, 1, R8G8B8A8_LINEAR, Bilinear));
	
		man->addReference<Texture>(imageDyCoefficients.handle);
		man->addReference<Texture>(imageDxCoefficients.handle);
		man->addReference<Texture>(imageDzCoefficients.handle);

		uint32_t invocation_size = (uint32_t)N / 16;

		pipeline = manager->createPipeline(PipelineTypeHashFlags::Compute, shader);
		manager->addReference<Pipeline>(pipeline.handle);
		dynamic_cast<GLComputePipeline*>(pipeline.pointer)->addTextureBinding(0, imageDyCoefficients.pointer, (AccessMask)ComputeAccessFlags::READ_WRITE);
		dynamic_cast<GLComputePipeline*>(pipeline.pointer)->addTextureBinding(1, imageDxCoefficients.pointer, (AccessMask)ComputeAccessFlags::READ_WRITE);
		dynamic_cast<GLComputePipeline*>(pipeline.pointer)->addTextureBinding(2, imageDzCoefficients.pointer, (AccessMask)ComputeAccessFlags::READ_WRITE);
		dynamic_cast<GLComputePipeline*>(pipeline.pointer)->setInvocationSize({ invocation_size, invocation_size, 1 });
	}

	Hkt::~Hkt()
	{
		manager->removeReference<Pipeline>(pipeline.handle);

		AssetManager* man = manager->getAssetManager();
		man->removeReference<Texture>(imageDyCoefficients.handle);
		man->removeReference<Texture>(imageDxCoefficients.handle);
		man->removeReference<Texture>(imageDzCoefficients.handle);
	}

	void Hkt::SetHTexture(Texture* h0k, Texture* h0minusK)
	{
		imageH0k = h0k;
		imageH0minusK = h0minusK;

		dynamic_cast<GLComputePipeline*>(pipeline.pointer)->addTextureBinding(3, imageH0k, (AccessMask)ComputeAccessFlags::READ_ONLY);
		dynamic_cast<GLComputePipeline*>(pipeline.pointer)->addTextureBinding(4, imageH0minusK, (AccessMask)ComputeAccessFlags::READ_ONLY);
	}

	void Hkt::Render(float delta)
	{
		pipeline->BindPipeline(nullptr);
		dynamic_cast<GLHktShader*>(shader.pointer)->UpdateUniforms(N, L, delta);
		pipeline->RenderPipeline();
		pipeline->UnbindPipeline();
	}
}