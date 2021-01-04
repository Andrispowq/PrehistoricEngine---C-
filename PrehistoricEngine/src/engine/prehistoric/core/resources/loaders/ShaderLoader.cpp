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
#include "platform/opengl/rendering/shaders/deferred/GLDeferredShader.h"
#include "platform/opengl/rendering/shaders/deferred/GLAlphaCoverageShader.h"
#include "platform/opengl/rendering/shaders/deferred/GLFXAAShader.h"

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
			else if (path == "deferred")
			{
				shader = new GLDeferredShader();
			}
			else if (path == "alpha_coverage")
			{
				shader = new GLAlphaCoverageShader();
			}
			else if (path == "fxaa")
			{
				shader = new GLFXAAShader();
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
