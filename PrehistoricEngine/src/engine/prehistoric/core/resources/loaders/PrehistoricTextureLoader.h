#ifndef PREHISTORIC_TEXTURE_LOADER_H
#define PREHISTORIC_TEXTURE_LOADER_H

#include "prehistoric/common/framework/Window.h"
#include "prehistoric/common/texture/Texture.h"

#include "prehistoric/core/util/Util.h"

#include "prehistoric/core/resources/Loader.h"

#include <thread>

namespace Prehistoric
{
	struct RawTextureData
	{
		uint32_t width, height, channels, imageType, size;
		char data[];
	};

	class PrehistoricTextureLoader : public Loader
	{
	public:
		PrehistoricTextureLoader(Window* window);

		virtual void* LoadResourceInternal(const std::string& path, Extra* extra) override;
		virtual void LoadResources() override;

		static Texture* LoadTexture(Window* window, const std::string& path, SamplerFilter filter = Anisotropic, TextureWrapMode wrapMode = Repeat);
		static ImageData LoadTextureData(const std::string& path);

		static bool SavePrehistoricTexture(const std::string& path, RawTextureData* data);
	};
};

#endif