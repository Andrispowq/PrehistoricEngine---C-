#ifndef WORLD_LOADER_H
#define WORLD_LOADER_H

#include <fstream>

#include "prehistoric/core/node/GameObject.h"
#include "prehistoric/common/buffer/MeshVertexBuffer.h"
#include "prehistoric/core/model/material/Material.h"
#include "prehistoric/common/rendering/shaders/Shader.h"
#include "prehistoric/core/node/component/renderer/RendererComponent.h"
#include "prehistoric/core/model/obj/OBJLoader.h"

#include "platform/opengl/rendering/shaders/basic/GLBasicShader.h"
#include "platform/vulkan/rendering/shaders/basic/VKBasicShader.h"

#include "platform/opengl/rendering/shaders/pbr/GLPBRShader.h"
#include "platform/vulkan/rendering/shaders/pbr/VKPBRShader.h"

#include "platform/opengl/rendering/pipeline/GLGraphicsPipeline.h"
#include "platform/vulkan/rendering/pipeline/VKGraphicsPipeline.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

#include "Includes.hpp"

namespace Prehistoric
{
	class WorldLoader
	{
	public:
		WorldLoader() {}
		virtual ~WorldLoader() {}

		void LoadWorld(const std::string& worldFile, GameObject* root, Window* window, AssembledAssetManager* manager);
	private:
		std::string directoryModels;
		std::string directoryTextures;

		//Rendering stuff
		std::unordered_map<std::string, TextureHandle> textures;
		std::unordered_map<std::string, MaterialHandle> materials;

		std::unordered_map<std::string, VertexBufferHandle> models;
		std::unordered_map<std::string, ShaderHandle> shaders;
		std::unordered_map<std::string, PipelineHandle> pipelines;
	};
};

#endif