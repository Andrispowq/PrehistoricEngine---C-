#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include "prehistoric/common/framework/Window.h"
#include "prehistoric/common/texture/Texture.h"

#include "prehistoric/core/util/Util.h"
#include "prehistoric/core/config/FrameworkConfig.h"

#include "prehistoric/core/resources/Loader.h"

#include <thread>

namespace Prehistoric
{
	union ImagePtr
	{
		unsigned char* dataUC; 
		float* dataF;
	};

	struct ImageData
	{
		enum class ImageType { LDR, HDR } type;
		int width, height, channels;
		union ImagePtr { unsigned char* dataUC; float* dataF; } ptr;
	};

	struct TextureLoaderExtra : public Extra
	{
		SamplerFilter filter = Anisotropic;
		TextureWrapMode wrapMode = Repeat;
	};

	class TextureLoader : public Loader
	{
	public:
		TextureLoader(Window* window);

		virtual void* LoadResourceInternal(const std::string& path, Extra* extra) override;
		virtual void LoadResources() override;

		static Texture* LoadTexture(Window* window, const std::string& path, SamplerFilter filter = Anisotropic, TextureWrapMode wrapMode = Repeat);
		static ImageData LoadTextureData(const std::string& path);
	};
};

#endif