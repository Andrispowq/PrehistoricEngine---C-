#ifndef H0K_H
#define H0K_H

#include "prehistoric/common/texture/Texture.h"
#include "prehistoric/common/rendering/shaders/Shader.h"
#include "prehistoric/common/buffer/ShaderStorageBuffer.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

#include "prehistoric/application/Application.h"

namespace Prehistoric
{
	class H0k
	{
	public:
		H0k(AssembledAssetManager* manager, Window* window, int N, int L,
			float amplitude, Vector2f direction, float alignment, float intensity,
			float capillarSupressFactor);
		~H0k();

		void Render();

		Texture* getH0k() const { return imageH0k.pointer; }
		Texture* getH0minusK() const { return imageH0minusK.pointer; }

	private:
		Window* window = nullptr;
		AssembledAssetManager* manager = nullptr;

		TextureHandle imageH0k;
		TextureHandle imageH0minusK;

		Vector2f direction;
		float intensity;
		float amplitude;
		float alignment;
		float capillarSupressFactor;
		int N;
		int L;

		TextureHandle noise0;
		TextureHandle noise1;
		TextureHandle noise2;
		TextureHandle noise3;

		ShaderHandle shader;
		PipelineHandle pipeline;
	};
}

#endif