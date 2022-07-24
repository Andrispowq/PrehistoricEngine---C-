#include "Includes.hpp"
#include "FFT.h"

#include "platform/opengl/rendering/shaders/fft/GLButterflyShader.h"
#include "platform/opengl/rendering/shaders/fft/GLInversionShader.h"

namespace Prehistoric
{
	FFT::FFT(AssembledAssetManager* manager, Window* window, WaterConfig* config)
		: manager(manager), window(window)
	{
		this->N = config->fftResolution;
		this->choppy = config->choppy;
		this->t_delta = config->tDelta;

		systemTime = Time::getTimeFromStartNanoseconds() / 1000000;

		log_2_N = (int)(std::log(N) / std::log(2));
		twiddleFactors = new TwiddleFactors(manager, window, N);
		h0k = new H0k(manager, window, N, config->fftL, config->fftAmplitude, config->windDirection, 
			config->alignment, config->windSpeed, config->fftCapillarWavesSupression);
		hkt = new Hkt(manager, window, N, config->fftL);

		AssetManager* man = manager->getAssetManager();
		butterflyShader = man->loadShader(ShaderName::Butterfly).value();
		inversionShader = man->loadShader(ShaderName::Inversion).value();

		pingpongTexture = man->storeTexture(GLTexture::Storage2D(N, N, 1, R8G8B8A8_LINEAR, Bilinear));
		Dy = man->storeTexture(GLTexture::Storage2D(N, N, 1, R8G8B8A8_LINEAR, Bilinear));
		Dx = man->storeTexture(GLTexture::Storage2D(N, N, 1, R8G8B8A8_LINEAR, Bilinear));
		Dz = man->storeTexture(GLTexture::Storage2D(N, N, 1, R8G8B8A8_LINEAR, Bilinear));
	
		man->addReference<Texture>(pingpongTexture.handle);
		man->addReference<Texture>(Dy.handle);
		man->addReference<Texture>(Dx.handle);
		man->addReference<Texture>(Dz.handle);

		butterflyPipeline = manager->createPipeline(PipelineTypeHashFlags::Compute, butterflyShader);
		manager->addReference<Pipeline>(butterflyPipeline.handle);
		inversionPipeline = manager->createPipeline(PipelineTypeHashFlags::Compute, inversionShader);
		manager->addReference<Pipeline>(inversionPipeline.handle);

		h0k->Render();
		twiddleFactors->Render();
		hkt->SetHTexture(h0k->getH0k(), h0k->getH0minusK());
	}

	FFT::~FFT()
	{
		manager->removeReference<Pipeline>(butterflyPipeline.handle);
		manager->removeReference<Pipeline>(inversionPipeline.handle);

		AssetManager* man = manager->getAssetManager();
		man->removeReference<Texture>(pingpongTexture.handle);
		man->removeReference<Texture>(Dy.handle);
		man->removeReference<Texture>(Dx.handle);
		man->removeReference<Texture>(Dz.handle);
	}

	void FFT::Render()
	{
		hkt->Render(t);

		uint32_t invSize = (uint32_t)N / 16;

		//Dy FFT

		pingpong = 0;
		dynamic_cast<GLComputePipeline*>(butterflyPipeline.pointer)->ResetBindings();
		dynamic_cast<GLComputePipeline*>(butterflyPipeline.pointer)->addTextureBinding(0, twiddleFactors->getTexture(), (AccessMask)ComputeAccessFlags::READ_WRITE);
		dynamic_cast<GLComputePipeline*>(butterflyPipeline.pointer)->addTextureBinding(1, hkt->getDyCoefficients(), (AccessMask)ComputeAccessFlags::READ_WRITE);
		dynamic_cast<GLComputePipeline*>(butterflyPipeline.pointer)->addTextureBinding(2, pingpongTexture.pointer, (AccessMask)ComputeAccessFlags::READ_WRITE);
		dynamic_cast<GLComputePipeline*>(butterflyPipeline.pointer)->setInvocationSize({ invSize, invSize, 1 });
		butterflyPipeline->BindPipeline(nullptr);

		// 1D FFT horizontal
		for (int i = 0; i < log_2_N; i++)
		{
			dynamic_cast<GLButterflyShader*>(butterflyShader.pointer)->UpdateUniforms(pingpong, 0, i);
			butterflyPipeline->RenderPipeline();
			pingpong++;
			pingpong %= 2;
		}

		// 1D FFT vertical
		for (int j = 0; j < log_2_N; j++)
		{
			dynamic_cast<GLButterflyShader*>(butterflyShader.pointer)->UpdateUniforms(pingpong, 1, j);
			butterflyPipeline->RenderPipeline();
			pingpong++;
			pingpong %= 2;
		}

		butterflyPipeline->UnbindPipeline();

		dynamic_cast<GLComputePipeline*>(inversionPipeline.pointer)->ResetBindings();
		dynamic_cast<GLComputePipeline*>(inversionPipeline.pointer)->addTextureBinding(0, Dy.pointer, (AccessMask)ComputeAccessFlags::READ_WRITE);
		dynamic_cast<GLComputePipeline*>(inversionPipeline.pointer)->addTextureBinding(1, hkt->getDyCoefficients(), (AccessMask)ComputeAccessFlags::READ_WRITE);
		dynamic_cast<GLComputePipeline*>(inversionPipeline.pointer)->addTextureBinding(2, pingpongTexture.pointer, (AccessMask)ComputeAccessFlags::READ_WRITE);
		dynamic_cast<GLComputePipeline*>(inversionPipeline.pointer)->setInvocationSize({ invSize, invSize, 1 });
		inversionPipeline->BindPipeline(nullptr);
		dynamic_cast<GLInversionShader*>(inversionShader.pointer)->UpdateUniforms(N, pingpong);
		inversionPipeline->RenderPipeline();
		inversionPipeline->UnbindPipeline();

		if (choppy)
		{
			// Dx-FFT

			pingpong = 0;
			dynamic_cast<GLComputePipeline*>(butterflyPipeline.pointer)->ResetBindings();
			dynamic_cast<GLComputePipeline*>(butterflyPipeline.pointer)->addTextureBinding(0, twiddleFactors->getTexture(), (AccessMask)ComputeAccessFlags::READ_WRITE);
			dynamic_cast<GLComputePipeline*>(butterflyPipeline.pointer)->addTextureBinding(1, hkt->getDxCoefficients(), (AccessMask)ComputeAccessFlags::READ_WRITE);
			dynamic_cast<GLComputePipeline*>(butterflyPipeline.pointer)->addTextureBinding(2, pingpongTexture.pointer, (AccessMask)ComputeAccessFlags::READ_WRITE);
			dynamic_cast<GLComputePipeline*>(butterflyPipeline.pointer)->setInvocationSize({ invSize, invSize, 1 });
			butterflyPipeline->BindPipeline(nullptr);

			// 1D FFT horizontal
			for (int i = 0; i < log_2_N; i++)
			{
				dynamic_cast<GLButterflyShader*>(butterflyShader.pointer)->UpdateUniforms(pingpong, 0, i);
				butterflyPipeline->RenderPipeline();
				pingpong++;
				pingpong %= 2;
			}

			// 1D FFT vertical
			for (int j = 0; j < log_2_N; j++)
			{
				dynamic_cast<GLButterflyShader*>(butterflyShader.pointer)->UpdateUniforms(pingpong, 1, j);
				butterflyPipeline->RenderPipeline();
				pingpong++;
				pingpong %= 2;
			}

			butterflyPipeline->UnbindPipeline();

			dynamic_cast<GLComputePipeline*>(inversionPipeline.pointer)->ResetBindings();
			dynamic_cast<GLComputePipeline*>(inversionPipeline.pointer)->addTextureBinding(0, Dx.pointer, (AccessMask)ComputeAccessFlags::READ_WRITE);
			dynamic_cast<GLComputePipeline*>(inversionPipeline.pointer)->addTextureBinding(1, hkt->getDyCoefficients(), (AccessMask)ComputeAccessFlags::READ_WRITE);
			dynamic_cast<GLComputePipeline*>(inversionPipeline.pointer)->addTextureBinding(2, pingpongTexture.pointer, (AccessMask)ComputeAccessFlags::READ_WRITE);
			dynamic_cast<GLComputePipeline*>(inversionPipeline.pointer)->setInvocationSize({ invSize, invSize, 1 });
			inversionPipeline->BindPipeline(nullptr);
			dynamic_cast<GLInversionShader*>(inversionShader.pointer)->UpdateUniforms(N, pingpong);
			inversionPipeline->RenderPipeline();
			inversionPipeline->UnbindPipeline();

			// Dz-FFT

			pingpong = 0;
			dynamic_cast<GLComputePipeline*>(butterflyPipeline.pointer)->ResetBindings();
			dynamic_cast<GLComputePipeline*>(butterflyPipeline.pointer)->addTextureBinding(0, twiddleFactors->getTexture(), (AccessMask)ComputeAccessFlags::READ_WRITE);
			dynamic_cast<GLComputePipeline*>(butterflyPipeline.pointer)->addTextureBinding(1, hkt->getDzCoefficients(), (AccessMask)ComputeAccessFlags::READ_WRITE);
			dynamic_cast<GLComputePipeline*>(butterflyPipeline.pointer)->addTextureBinding(2, pingpongTexture.pointer, (AccessMask)ComputeAccessFlags::READ_WRITE);
			dynamic_cast<GLComputePipeline*>(butterflyPipeline.pointer)->setInvocationSize({ invSize, invSize, 1 });
			butterflyPipeline->BindPipeline(nullptr);

			// 1D FFT horizontal
			for (int i = 0; i < log_2_N; i++)
			{
				dynamic_cast<GLButterflyShader*>(butterflyShader.pointer)->UpdateUniforms(pingpong, 0, i);
				butterflyPipeline->RenderPipeline();
				pingpong++;
				pingpong %= 2;
			}

			// 1D FFT vertical
			for (int j = 0; j < log_2_N; j++)
			{
				dynamic_cast<GLButterflyShader*>(butterflyShader.pointer)->UpdateUniforms(pingpong, 1, j);
				butterflyPipeline->RenderPipeline();
				pingpong++;
				pingpong %= 2;
			}

			butterflyPipeline->UnbindPipeline();

			dynamic_cast<GLComputePipeline*>(inversionPipeline.pointer)->ResetBindings();
			dynamic_cast<GLComputePipeline*>(inversionPipeline.pointer)->addTextureBinding(0, Dz.pointer, (AccessMask)ComputeAccessFlags::READ_WRITE);
			dynamic_cast<GLComputePipeline*>(inversionPipeline.pointer)->addTextureBinding(1, hkt->getDyCoefficients(), (AccessMask)ComputeAccessFlags::READ_WRITE);
			dynamic_cast<GLComputePipeline*>(inversionPipeline.pointer)->addTextureBinding(2, pingpongTexture.pointer, (AccessMask)ComputeAccessFlags::READ_WRITE);
			dynamic_cast<GLComputePipeline*>(inversionPipeline.pointer)->setInvocationSize({ invSize, invSize, 1 });
			inversionPipeline->BindPipeline(nullptr);
			dynamic_cast<GLInversionShader*>(inversionShader.pointer)->UpdateUniforms(N, pingpong);
			inversionPipeline->RenderPipeline();
			inversionPipeline->UnbindPipeline();
		}

		t += (Time::getTimeFromStartNanoseconds() / 1000000 - systemTime) * t_delta;
		systemTime = Time::getTimeFromStartNanoseconds() / 1000000;
	}
}