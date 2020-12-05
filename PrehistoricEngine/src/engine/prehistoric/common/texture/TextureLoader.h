#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include "engine/platform/opengl/texture/GLTexture.h"
#include "engine/platform/vulkan/texture/VKTexture.h"

#include "engine/prehistoric/common/framework/Window.h"

#include "engine/prehistoric/core/util/Util.h"
#include "engine/prehistoric/core/config/FrameworkConfig.h"

namespace Prehistoric
{
	struct ImageData
	{
		int width, height;
		unsigned char* data;
	};

	namespace TextureLoader
	{
		Texture* LoadTexture(const std::string& path, Window* window, SamplerFilter filter = Anisotropic, TextureWrapMode wrapMode = Repeat);
		ImageData LoadTextureData(const std::string& path);
	};
};

#endif