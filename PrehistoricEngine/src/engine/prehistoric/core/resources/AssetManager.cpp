#include "Includes.hpp"
#include "AssetManager.h"

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
#include "platform/opengl/rendering/shaders/postProcessing/GLHDRShader.h"
#include "platform/opengl/rendering/shaders/forwardPlus/GLDepthPassShader.h"
#include "platform/opengl/rendering/shaders/forwardPlus/GLLightCullingPassShader.h"

#include "platform/vulkan/rendering/shaders/basic/VKBasicShader.h"
#include "platform/vulkan/rendering/shaders/pbr/VKPBRShader.h"

namespace Prehistoric
{
	AssetManager::AssetManager(Window* window)
	{
		this->window = window;

		textures.reserve(TexturesSize);
		vertexBuffers.reserve(VertexBuffersSize);
		shaders.reserve(ShadersSize);

		textureLoader = new TextureLoader(window);
		vertexBufferLoader = new VertexBufferLoader(window, this);
		shaderLoader = new ShaderLoader(window);
	}

	std::optional<TextureHandle> AssetManager::loadTexture(const std::string& location, SamplerFilter filter, TextureWrapMode wrapMode, BatchSettings settings)
	{
		if (settings == BatchSettings::QueuedLoad)
		{
			/*
				We don't know when the commands will be dispatched, so we can't use a stack variable here
			*/
			TextureLoaderExtra* extra = new TextureLoaderExtra;
			extra->filter = filter;
			extra->wrapMode = wrapMode;

			textureLoader->LoadResource(false, location, extra);
			return std::nullopt;
		}

		auto index = ID_map.find(location);
		if (index != ID_map.end())
		{
			TextureHandle handle;
			handle.handle = index->second;
			handle.pointer = textures.at(handle.handle).first.get();

			return handle;
		}

		TextureLoaderExtra extra;
		extra.filter = filter;
		extra.wrapMode = wrapMode;

		Texture* tex = (Texture*)textureLoader->LoadResource(true, location, &extra).value();
		size_t han = texture_ID++;

		TextureHandle handle;
		handle.pointer = tex;
		handle.handle = han;

		textures.insert(std::make_pair(handle.handle, std::make_pair(tex, 0)));
		ID_map.insert(std::make_pair(location, handle.handle));

		return handle;
	}

	std::optional<VertexBufferHandle> AssetManager::loadVertexBuffer(std::optional<Model> mesh, const std::string& name, BatchSettings settings)
	{
		if (settings == BatchSettings::QueuedLoad)
		{
			/*
				We don't know when the commands will be dispatched, so we can't use a stack variable here
			*/
			vertexBufferLoader->LoadResource(false, name, nullptr);
			return std::nullopt;
		}

		auto index = ID_map.find(name);
		if (index != ID_map.end())
		{
			VertexBufferHandle handle;
			handle.handle = index->second;
			handle.pointer = vertexBuffers.at(handle.handle).first.get();

			return handle;
		}

		VertexBufferLoaderExtra extra;

		if (mesh.has_value())
			extra.mesh = mesh.value();

		VertexBuffer* vb = (VertexBuffer*)vertexBufferLoader->LoadResource(true, name, &extra).value();
		size_t han = vertexBuffer_ID++;

		VertexBufferHandle handle;
		handle.pointer = vb;
		handle.handle = han;

		vertexBuffers.insert(std::make_pair(handle.handle, std::make_pair(vb, 0)));
		ID_map.insert(std::make_pair(name, handle.handle));

		return handle;
	}

	std::optional<ShaderHandle> AssetManager::loadShader(ShaderName type, BatchSettings settings)
	{
		std::string name;
		switch (type)
		{
		case Prehistoric::ShaderName::PBR:
			name = "pbr";
			break;
		case Prehistoric::ShaderName::Basic:
			name = "basic";
			break;
		case Prehistoric::ShaderName::AtmosphereScattering:
			name = "atmosphere_scattering";
			break;
		case Prehistoric::ShaderName::Atmosphere:
			name = "atmosphere";
			break;
		case Prehistoric::ShaderName::TerrainWireframe:
			name = "terrain_wireframe";
			break;
		case Prehistoric::ShaderName::Terrain:
			name = "terrain";
			break;
		case Prehistoric::ShaderName::Gui:
			name = "gui";
			break;
		case Prehistoric::ShaderName::GPGPUNormal:
			name = "gpgpu_normal";
			break;
		case Prehistoric::ShaderName::GPGPUSplat:
			name = "gpgpu_splat";
			break;
		case Prehistoric::ShaderName::GPGPUHeightQuery:
			name = "gpgpu_terrain_heights";
			break;
		case Prehistoric::ShaderName::HDR:
			name = "hdr";
			break;
		case Prehistoric::ShaderName::LightCullingPass:
			name = "light_culling";
			break;
		case Prehistoric::ShaderName::DepthPass:
			name = "depth_pass";
			break;
		case Prehistoric::ShaderName::Gaussian:
			name = "gaussian";
			break;
		case Prehistoric::ShaderName::BloomCombine:
			name = "bloom_combine";
			break;
		case Prehistoric::ShaderName::BloomDecompose:
			name = "bloom_decompose";
			break;
		case Prehistoric::ShaderName::ShadowDepthPass:
			name = "shadow_depth_pass";
			break;
		default:
			name = "invalid";
			break;
		}

		auto index = ID_map.find(name);
		if (index != ID_map.end())
		{
			ShaderHandle handle;
			handle.handle = index->second;
			handle.pointer = shaders.at(handle.handle).first.get();

			return handle;
		}

		Shader* vb = (Shader*) shaderLoader->LoadResource(true, name, nullptr).value();
		size_t han = shader_ID++;

		ShaderHandle handle;
		handle.pointer = vb;
		handle.handle = han;

		shaders.insert(std::make_pair(handle.handle, std::make_pair(vb, 0)));
		ID_map.insert(std::make_pair(name, handle.handle));

		return handle;
	}

	TextureHandle AssetManager::storeTexture(Texture* texture, const std::string& cacheName)
	{
		for (auto& entry : textures)
		{
			if (entry.second.first.get() == texture)
			{
				return { texture, entry.second.second };
			}
		}

		size_t han = texture_ID++;

		TextureHandle handle;
		handle.pointer = texture;
		handle.handle = han;

		textures.insert(std::make_pair(handle.handle, std::make_pair(texture, 0)));

		if (!cacheName.empty())
		{
			ID_map.insert(std::make_pair(cacheName, handle.handle));
		}

		return handle;
	}

	VertexBufferHandle AssetManager::storeVertexBuffer(VertexBuffer* vertexBuffer, const std::string& cacheName)
	{
		for (auto& entry : vertexBuffers)
		{
			if (entry.second.first.get() == vertexBuffer)
			{
				return { vertexBuffer, entry.second.second };
			}
		}

		size_t han = vertexBuffer_ID++;

		VertexBufferHandle handle;
		handle.pointer = vertexBuffer;
		handle.handle = han;

		vertexBuffers.insert(std::make_pair(handle.handle, std::make_pair(vertexBuffer, 0)));

		if (!cacheName.empty())
		{
			ID_map.insert(std::make_pair(cacheName, handle.handle));
		}

		return handle;
	}

	ShaderHandle AssetManager::storeShader(Shader* shader)
	{
		for (auto& entry : shaders)
		{
			if (entry.second.first.get() == shader)
			{
				return { shader, entry.second.second };
			}
		}

		size_t han = shader_ID++;

		ShaderHandle handle;
		handle.pointer = shader;
		handle.handle = han;

		shaders.insert(std::make_pair(handle.handle, std::make_pair(shader, 0)));

		return handle;
	}
};