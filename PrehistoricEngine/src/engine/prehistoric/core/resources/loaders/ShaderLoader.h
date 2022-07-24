#ifndef SHADER_LOADER_H
#define SHADER_LOADER_H

#include "prehistoric/common/framework/Window.h"

#include "prehistoric/core/resources/Loader.h"

namespace Prehistoric
{
	enum class ShaderName
	{
		PBR,
		AtmosphereScattering,
		Atmosphere,
		TerrainWireframe,
		TerrainShadow,
		Terrain,
		Gui,
		GPGPUNormal,
		GPGPUSplat,
		GPGPUHeightQuery,
		HDR,
		DepthPass,
		LightCullingPass,
		Gaussian,
		BloomCombine,
		BloomDecompose,
		ShadowDepthPass,
		VolumetricPostProcessing,
		RayTracing,
		Butterfly,
		H0k,
		Hkt,
		Inversion,
		TwiddleFactors,
		Water,
		WaterWireframe
	};

	class ShaderLoader : public Loader
	{
	public:
		ShaderLoader(Window* window) : Loader(window) {}

		virtual void* LoadResourceInternal(const std::string& path, Extra* extra) override;
	};
};

#endif