#ifndef WORLD_LOADER_H
#define WORLD_LOADER_H

#include <fstream>

#include "engine/prehistoric/core/node/GameObject.h"
#include "engine/prehistoric/common/buffer/MeshVertexBuffer.h"
#include "engine/prehistoric/core/model/material/Material.h"
#include "engine/prehistoric/common/rendering/shaders/Shader.h"
#include "engine/prehistoric/core/node/component/renderer/RendererComponent.h"
#include "engine/prehistoric/core/model/obj/OBJLoader.h"

#include "engine/platform/opengl/rendering/shaders/basic/GLBasicShader.h"
#include "engine/platform/vulkan/rendering/shaders/basic/VKBasicShader.h"

#include "engine/platform/opengl/rendering/shaders/pbr/GLPBRShader.h"
#include "engine/platform/vulkan/rendering/shaders/pbr/VKPBRShader.h"

#include "engine/platform/opengl/rendering/pipeline/GLGraphicsPipeline.h"
#include "engine/platform/vulkan/rendering/pipeline/VKGraphicsPipeline.h"

#include "engine/prehistoric/core/util/Includes.hpp"

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
		std::unordered_map<std::string, size_t> textures;
		std::unordered_map<std::string, size_t> materials;

		std::unordered_map<std::string, size_t> models;
		std::unordered_map<std::string, size_t> shaders;
		std::unordered_map<std::string, size_t> pipelines;
	};
};

#endif