#ifndef WATER_H
#define WATER_H

#include "prehistoric/core/node/Node.h"

#include "WaterQuadtree.h"

#include "prehistoric/core/modules/fft/FFT.h"
#include "prehistoric/core/modules/gpgpu/NormalMapRenderer.h"

namespace Prehistoric
{
	class Renderer;
	class Framebuffer;

	class Water : public Node
	{
	public:
		Water(Window* window, Camera* camera, AssembledAssetManager* manager, Vector3f position);
		virtual ~Water();

		void PreRender(Renderer* renderer) override;
		void RenderCaptures(Renderer* renderer);
		void UpdateQuadtree();
        bool isCameraUnderwater() const { return cameraUnderwater; }

		Texture* getReflectionTexture() const { return reflectionTexture.pointer; }
		Texture* getRefractionTexture() const { return refractionTexture.pointer; }

		Water(const Water&) = default;
	private:
		Window* window;
		Camera* camera;
		AssembledAssetManager* manager;

		WaterQuadtree* quadtree;

		static constexpr uint32_t refl_size = 1024;
		static constexpr uint32_t refr_size = 1024;

		std::unique_ptr<FFT> fft;
		std::unique_ptr<NormalMapRenderer> normalMapRenderer;

		TextureHandle dudv;
		Vector3f position = 0;
		Vector3f lastLodPosition;

		MaterialHandle material;

		std::unique_ptr<Framebuffer> reflectionFbo;
		std::unique_ptr<Framebuffer> refractionFbo;

		TextureHandle reflectionTexture;
		TextureHandle refractionTexture;

		bool cameraUnderwater = false;
	};
}

#endif