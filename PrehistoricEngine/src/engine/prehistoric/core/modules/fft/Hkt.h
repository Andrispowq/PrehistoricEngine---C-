#ifndef HKT_H
#define HKT_H

#include "prehistoric/common/texture/Texture.h"
#include "prehistoric/common/rendering/shaders/Shader.h"
#include "prehistoric/common/buffer/ShaderStorageBuffer.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

#include "prehistoric/application/Application.h"

namespace Prehistoric
{
	class Hkt
	{
	public:
		Hkt(AssembledAssetManager* manager, Window* window, int N, int L);
		~Hkt();

		void SetHTexture(Texture* h0k, Texture* h0minusK);

		void Render(float delta);

		Texture* getDyCoefficients() const { return imageDyCoefficients.pointer; }
		Texture* getDxCoefficients() const { return imageDxCoefficients.pointer; }
		Texture* getDzCoefficients() const { return imageDzCoefficients.pointer; }

	private:
		Window* window = nullptr;
		AssembledAssetManager* manager = nullptr;

		TextureHandle imageDyCoefficients;
		TextureHandle imageDxCoefficients;
		TextureHandle imageDzCoefficients;

		Texture* imageH0k = nullptr;
		Texture* imageH0minusK = nullptr;

		int N;
		int L;
		
		ShaderHandle shader;
		PipelineHandle pipeline;
	};
}

#endif