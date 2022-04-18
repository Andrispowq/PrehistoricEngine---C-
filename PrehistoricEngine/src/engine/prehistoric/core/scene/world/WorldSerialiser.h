#ifndef WORLD_SERIALISER_H
#define WORLD_SERIALISER_H

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
#include "prehistoric/core/scene/Scene.h"

#include "Includes.hpp"

namespace Prehistoric
{
	class WorldSerialiser
	{
	public:
		WorldSerialiser(Window* window, AssembledAssetManager* manager)
			: window(window), manager(manager) {}
		virtual ~WorldSerialiser() {}

		void SerialiseWorld(const std::string& worldFile, Scene* scene);
		void SerialiseWorldJSON(const std::string& worldFile, Scene* scene);

	private:
		void ListResources(GameObject* root);

	public:
		std::string sceneName;

		std::string directoryModels;
		std::string directoryTextures;

		//Rendering stuff
		std::unordered_map<std::string, VertexBufferHandle> models;

		std::unordered_map<std::string, TextureHandle> textures;
		std::unordered_map<std::string, MaterialHandle> materials;

	private:
		Window* window;
		AssembledAssetManager* manager;
	};
};

#endif