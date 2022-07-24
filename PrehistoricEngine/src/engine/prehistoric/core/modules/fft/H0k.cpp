#include "Includes.hpp"
#include "H0k.h"

#include "platform/opengl/rendering/shaders/fft/GLH0kShader.h"

namespace Prehistoric
{
	H0k::H0k(AssembledAssetManager* manager, Window* window, int N, int L, float amplitude, Vector2f direction, float alignment, float intensity, float capillarSupressFactor)
		: manager(manager), window(window), N(N), L(L), direction(direction),
		amplitude(amplitude), intensity(intensity), capillarSupressFactor(capillarSupressFactor),
		alignment(alignment)
	{
		AssetManager* man = manager->getAssetManager();
		shader = man->loadShader(ShaderName::H0k).value();

		imageH0k = man->storeTexture(GLTexture::Storage2D(N, N, 1, R8G8B8A8_LINEAR, Bilinear));
		imageH0minusK = man->storeTexture(GLTexture::Storage2D(N, N, 1, R8G8B8A8_LINEAR, Bilinear));
	
		noise0 = man->loadTexture("res/textures/noise/noise256_0.jpg").value();
		noise1 = man->loadTexture("res/textures/noise/noise256_1.jpg").value();
		noise2 = man->loadTexture("res/textures/noise/noise256_2.jpg").value();
		noise3 = man->loadTexture("res/textures/noise/noise256_3.jpg").value();

		man->addReference<Texture>(imageH0k.handle);
		man->addReference<Texture>(imageH0minusK.handle);

		man->addReference<Texture>(noise0.handle);
		man->addReference<Texture>(noise1.handle);
		man->addReference<Texture>(noise2.handle);
		man->addReference<Texture>(noise3.handle);

		uint32_t invocation_size = (uint32_t)N / 16;

		pipeline = manager->createPipeline(PipelineTypeHashFlags::Compute, shader);
		manager->addReference<Pipeline>(pipeline.handle);
		dynamic_cast<GLComputePipeline*>(pipeline.pointer)->addTextureBinding(0, imageH0k.pointer, (AccessMask)ComputeAccessFlags::WRITE_ONLY);
		dynamic_cast<GLComputePipeline*>(pipeline.pointer)->addTextureBinding(1, imageH0minusK.pointer, (AccessMask)ComputeAccessFlags::WRITE_ONLY);
		dynamic_cast<GLComputePipeline*>(pipeline.pointer)->setInvocationSize({ invocation_size, invocation_size, 1 });
	}

	H0k::~H0k()
	{
		manager->removeReference<Pipeline>(pipeline.handle);

		AssetManager* man = manager->getAssetManager();
		man->removeReference<Texture>(imageH0k.handle);
		man->removeReference<Texture>(imageH0minusK.handle);

		man->removeReference<Texture>(noise0.handle);
		man->removeReference<Texture>(noise1.handle);
		man->removeReference<Texture>(noise2.handle);
		man->removeReference<Texture>(noise3.handle);
	}

	void H0k::Render()
	{
		pipeline->BindPipeline(nullptr);
		
		noise0->Bind(0);
		noise1->Bind(1);
		noise2->Bind(2);
		noise3->Bind(3);

		dynamic_cast<GLH0kShader*>(shader.pointer)->UpdateUniforms(N, L, amplitude, direction, alignment, intensity, capillarSupressFactor);
		dynamic_cast<GLH0kShader*>(shader.pointer)->UpdateUniformsTextures(0, 1, 2, 3);

		pipeline->RenderPipeline();
		pipeline->UnbindPipeline();
	}
}