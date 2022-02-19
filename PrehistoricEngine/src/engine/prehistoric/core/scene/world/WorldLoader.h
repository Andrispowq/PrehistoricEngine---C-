#ifndef WORLD_LOADER_H
#define WORLD_LOADER_H

#include <fstream>

#include "prehistoric/core/node/GameObject.h"
#include "prehistoric/common/buffer/MeshVertexBuffer.h"
#include "prehistoric/core/model/material/Material.h"
#include "prehistoric/common/rendering/shaders/Shader.h"
#include "prehistoric/core/node/component/renderer/RendererComponent.h"
#include "prehistoric/core/model/obj/OBJLoader.h"

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
		WorldLoader(Window* window, AssembledAssetManager* manager) 
			: window(window), manager(manager) {}
		virtual ~WorldLoader() {}

		void LoadWorld(const std::string& worldFile, GameObject* root);
		void LoadWorldJSON(const std::string& worldFile, GameObject* root);

	private:
		//std::vector<nlohmann::json> LoadObjectJSON(GameObject* root, nlohmann::json& objectData);

	public:
		std::string sceneName;

		std::string directoryModels;
		std::string directoryTextures;

		//Rendering stuff
		std::unordered_map<std::string, TextureHandle> textures;
		std::vector<std::string> textureNames;
		std::unordered_map<std::string, MaterialHandle> materials;

		std::unordered_map<std::string, VertexBufferHandle> models;
		std::vector<std::string> meshNames;
		std::vector<FrontFace> frontFaces;
		std::unordered_map<std::string, ShaderHandle> shaders;
		std::unordered_map<std::string, PipelineHandle> pipelines;

	private:
		Window* window;
		AssembledAssetManager* manager;
	};
};

#endif