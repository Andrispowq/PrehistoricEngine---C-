#include "Includes.hpp"
#include "ResourceStorage.h"

namespace Prehistoric
{
	ResourceStorage::ResourceStorage(Window* window)
	{
		this->window = window;

		textures.reserve(TexturesSize);
		vertexBuffers.reserve(VertexBuffersSize);
		shaders.reserve(ShadersSize);
		pipelines.reserve(PipelinesSize);
		materials.reserve(MaterialsSize);

		textureLoader = new TextureLoader(window);
		vertexBufferLoader = new VertexBufferLoader(window);
		shaderLoader = new ShaderLoader(window);
	}

	std::optional<TextureHandle> ResourceStorage::loadTexture(const std::string& location, SamplerFilter filter, TextureWrapMode wrapMode, BatchSettings settings)
	{
		if (settings == BatchSettings::QueuedLoad)
			return std::nullopt;

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

	std::optional<VertexBufferHandle> ResourceStorage::loadVertexBuffer(std::optional<Mesh> mesh, const std::string& name, BatchSettings settings)
	{
		if (settings == BatchSettings::QueuedLoad)
			return std::nullopt;

		VertexBufferLoaderExtra extra;

		if(mesh.has_value())
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

	std::optional<ShaderHandle> ResourceStorage::loadShader(const std::string& location, BatchSettings settings)
	{
		if (settings == BatchSettings::QueuedLoad)
			return std::nullopt;

		Shader* vb = (Shader*)shaderLoader->LoadResource(true, location, nullptr).value();
		size_t han = shader_ID++;

		ShaderHandle handle;
		handle.pointer = vb;
		handle.handle = han;

		shaders.insert(std::make_pair(handle.handle, std::make_pair(vb, 0)));
		ID_map.insert(std::make_pair(location, handle.handle));

		return handle;
	}

	TextureHandle ResourceStorage::storeTexture(Texture* texture)
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

	VertexBufferHandle ResourceStorage::storeVertexBuffer(VertexBuffer* vertexBuffer)
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

	ShaderHandle ResourceStorage::storeShader(Shader* shader)
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

	PipelineHandle ResourceStorage::storePipeline(Pipeline* pipeline)
	{
		for (auto& entry : pipelines)
		{
			if (entry.second.first.get() == pipeline)
			{
				return { pipeline, entry.second.second };
			}
		}

		size_t han = pipeline_ID++;

		PipelineHandle handle;
		handle.pointer = pipeline;
		handle.handle = han;

		pipelines.insert(std::make_pair(handle.handle, std::make_pair(pipeline, 0)));

		return handle;
	}

	MaterialHandle ResourceStorage::storeMaterial(Material* material)
	{
		for (auto& entry : materials)
		{
			if (entry.second.first.get() == material)
			{
				return { material, entry.second.second };
			}
		}
		
		size_t han = material_ID++;

		MaterialHandle handle;
		handle.pointer = material;
		handle.handle = han;

		materials.insert(std::make_pair(handle.handle, std::make_pair(material, 0)));

		return handle;
	}
};
