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
#include "platform/opengl/rendering/shaders/deferred/GLDeferredShader.h"
#include "platform/opengl/rendering/shaders/deferred/GLAlphaCoverageShader.h"
#include "platform/opengl/rendering/shaders/deferred/GLFXAAShader.h"

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
		vertexBufferLoader = new VertexBufferLoader(window);
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

	std::optional<VertexBufferHandle> AssetManager::loadVertexBuffer(std::optional<Mesh> mesh, const std::string& name, BatchSettings settings)
	{
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

	std::optional<ShaderHandle> AssetManager::loadShader(const std::string& location, BatchSettings settings)
	{
		auto index = ID_map.find(location);
		if (index != ID_map.end())
		{
			ShaderHandle handle;
			handle.handle = index->second;
			handle.pointer = shaders.at(handle.handle).first.get();

			return handle;
		}

		Shader* vb = (Shader*)shaderLoader->LoadResource(true, location, nullptr).value();
		size_t han = shader_ID++;

		ShaderHandle handle;
		handle.pointer = vb;
		handle.handle = han;

		shaders.insert(std::make_pair(handle.handle, std::make_pair(vb, 0)));
		ID_map.insert(std::make_pair(location, handle.handle));

		return handle;
	}

	TextureHandle AssetManager::storeTexture(Texture* texture)
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

		return handle;
	}

	VertexBufferHandle AssetManager::storeVertexBuffer(VertexBuffer* vertexBuffer)
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