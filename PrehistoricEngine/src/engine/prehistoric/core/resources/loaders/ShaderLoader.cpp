#include "Includes.hpp"
#include "ShaderLoader.h"

#include "platform/opengl/rendering/shaders/atmosphere/GLAtmosphereScatteringShader.h"
#include "platform/opengl/rendering/shaders/atmosphere/GLAtmosphereShader.h"
#include "platform/opengl/rendering/shaders/basic/GLBasicShader.h"
#include "platform/opengl/rendering/shaders/gpgpu/GLNormalmapShader.h"
#include "platform/opengl/rendering/shaders/gpgpu/GLSplatmapShader.h"
#include "platform/opengl/rendering/shaders/gpgpu/GLTerrainHeightsShader.h"
#include "platform/opengl/rendering/shaders/gui/GLGUIShader.h"
#include "platform/opengl/rendering/shaders/pbr/GLPBRShader.h"
#include "platform/opengl/rendering/shaders/terrain/GLTerrainShader.h"
#include "platform/opengl/rendering/shaders/terrain/GLTerrainWireframeShader.h"
#include "platform/opengl/rendering/shaders/forwardPlus/GLDepthPassShader.h"
#include "platform/opengl/rendering/shaders/forwardPlus/GLLightCullingPassShader.h"
#include "platform/opengl/rendering/shaders/postProcessing/GLGaussianShader.h"
#include "platform/opengl/rendering/shaders/postProcessing/GLBloomCombineShader.h"
#include "platform/opengl/rendering/shaders/postProcessing/GLHDRShader.h"
#include "platform/opengl/rendering/shaders/postProcessing/GLBloomDecomposeShader.h"
#include "platform/opengl/rendering/shaders/shadow/GLShadowDepthPassShader.h"

#include "platform/vulkan/rendering/shaders/basic/VKBasicShader.h"
#include "platform/vulkan/rendering/shaders/pbr/VKPBRShader.h"

namespace Prehistoric
{
    void* ShaderLoader::LoadResourceInternal(const std::string& path, Extra* extra)
    {
		Shader* shader;
		if (FrameworkConfig::api == OpenGL)
		{
			if (path == "pbr")
			{
				shader = new GLPBRShader();
			}
			else if (path == "basic")
			{
				shader = new GLBasicShader();
			}
			else if (path == "atmosphere_scattering")
			{
				shader = new GLAtmosphereScatteringShader();
			}
			else if (path == "atmosphere")
			{
				shader = new GLAtmosphereShader();
			}
			else if (path == "terrain_wireframe")
			{
				shader = new GLTerrainWireframeShader();
			}
			else if (path == "terrain")
			{
				shader = new GLTerrainShader();
			}
			else if (path == "gui")
			{
				shader = new GLGUIShader();
			}
			else if (path == "gpgpu_normal")
			{
				shader = new GLNormalMapShader();
			}
			else if (path == "gpgpu_splat")
			{
				shader = new GLSplatMapShader();
			}
			else if (path == "gpgpu_terrain_heights")
			{
				shader = new GLTerrainHeightsShader();
			}
			else if (path == "hdr")
			{
				shader = new GLHDRShader();
			}
			else if (path == "depth_pass")
			{
				shader = new GLDepthPassShader();
			}
			else if (path == "light_culling")
			{
				shader = new GLLightCullingPassShader();
			}
			else if (path == "gaussian")
			{
				shader = new GLGaussianShader();
			}
			else if (path == "bloom_combine")
			{
				shader = new GLBloomCombineShader();
			}
			else if (path == "bloom_decompose")
			{
				shader = new GLBloomDecomposeShader();
			}
			else if (path == "shadow_depth_pass")
			{
				shader = new GLShadowDepthPassShader();
			}
		}
		else if (FrameworkConfig::api == Vulkan)
		{
			if (path == "pbr")
			{
				shader = new VKPBRShader(window);
			}
			else if (path == "basic")
			{
				shader = new VKBasicShader(window);
			}
		}

        return shader;
    }
};
