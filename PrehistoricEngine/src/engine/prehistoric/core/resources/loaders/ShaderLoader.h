#ifndef SHADER_LOADER_H
#define SHADER_LOADER_H

#include "prehistoric/common/framework/Window.h"

#include "prehistoric/core/resources/Loader.h"

namespace Prehistoric
{
	class ShaderLoader : public Loader
	{
	public:
		ShaderLoader(Window* window) : Loader(window) {}

		virtual void* LoadResourceInternal(const std::string& path, Extra* extra) override;
	};
};

#endif