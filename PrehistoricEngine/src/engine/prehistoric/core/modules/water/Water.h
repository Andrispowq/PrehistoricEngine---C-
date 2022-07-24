#ifndef WATER_H
#define WATER_H

#include "prehistoric/core/node/Node.h"

#include "WaterQuadtree.h"

#include "prehistoric/core/modules/fft/FFT.h"
#include "prehistoric/core/modules/gpgpu/NormalMapRenderer.h"

namespace Prehistoric
{
	class Renderer;

	class Water : public Node
	{
	public:
		Water(Window* window, Camera* camera, AssembledAssetManager* manager, Vector3f position);
		virtual ~Water();

		void PreRender(Renderer* renderer) override;
		void UpdateQuadtree();

		Water(const Water&) = default;
	private:
		void RenderObjects(Renderer* renderer) const;
	private:
		Window* window;
		Camera* camera;
		AssembledAssetManager* manager;

		WaterQuadtree* quadtree;

		size_t refl_size = 1024;
		size_t refr_size = 1024;

		std::unique_ptr<FFT> fft;
		std::unique_ptr<NormalMapRenderer> normalMapRenderer;

		Vector4f clipplane = 0.0f;
		int clipOffset = 0;
		float motion = 0.0f;
		float distortionDelta = 0.0f;
		TextureHandle dudv;

		TexturesData* texData;

		std::unique_ptr<Framebuffer> reflectionFbo;
		std::unique_ptr<Framebuffer> refractionFbo;

		TextureHandle reflectionTexture;
		TextureHandle refractionTexture;

		bool cameraUnderwater = false;
	};
}

#endif