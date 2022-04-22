#ifndef VERTEX_BUFFER_LOADER_H
#define VERTEX_BUFFER_LOADER_H

#include "prehistoric/common/framework/Window.h"
#include "prehistoric/common/buffer/MeshVertexBuffer.h"

#include "platform/opengl/buffer/GLMeshVertexBuffer.h"
#include "platform/vulkan/buffer/VKMeshVertexBuffer.h"
#include "platform/opengl/buffer/GLPatchVertexBuffer.h"
//#include "platform/vulkan/buffer/VKPatchVertexBuffer.h"

#include "prehistoric/core/resources/Loader.h"
#include "prehistoric/core/model/obj/OBJLoader.h"
#include "prehistoric/core/model/Mesh.h"

#include <thread>

namespace Prehistoric
{
	enum class Type
	{
		Mesh, Patch
	};

	struct VertexBufferLoaderExtra : public Extra
	{
		Type type = Type::Mesh;
		std::optional<Model> mesh;
	};

	class AssetManager;

	class VertexBufferLoader : public Loader
	{
	public:
		VertexBufferLoader(Window* window, AssetManager* manager);

		virtual void* LoadResourceInternal(const std::string& path, Extra* extra) override;
		virtual void LoadResources() override;

		static Model LoadModel(const std::string& path);

	private:
		AssetManager* manager;
	};
};

#endif