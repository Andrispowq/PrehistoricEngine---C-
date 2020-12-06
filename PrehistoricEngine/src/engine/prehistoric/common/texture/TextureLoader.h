#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include "platform/opengl/texture/GLTexture.h"
#include "platform/vulkan/texture/VKTexture.h"

#include "prehistoric/common/framework/Window.h"

#include "prehistoric/core/util/Util.h"
#include "prehistoric/core/config/FrameworkConfig.h"

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
		ImagePtr ptr;
	};

	namespace TextureLoader
	{
		Texture* LoadTexture(const std::string& path, Window* window, SamplerFilter filter = Anisotropic, TextureWrapMode wrapMode = Repeat);
		ImageData LoadTextureData(const std::string& path);
	};
};

#endif