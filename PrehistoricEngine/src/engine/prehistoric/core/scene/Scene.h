#ifndef SCENE_H
#define SCENE_H

#include "prehistoric/core/node/component/renderer/RendererComponent.h"
#include "world/WorldLoader.h"

#include "prehistoric/core/modules/terrain/Terrain.h"

#include "platform/vulkan/rendering/pipeline/VKPipeline.h"
#include "platform/vulkan/buffer/VKMeshVertexBuffer.h"
#include "platform/vulkan/texture/VkTexture.h"
#include "platform/vulkan/rendering/shaders/pbr/VKPBRShader.h"

#include "prehistoric/core/modules/gui/GUIElement.h"
#include "prehistoric/core/modules/gui/button/GUIButton.h"
#include "prehistoric/core/modules/gui/slider/GUISlider.h"

#include "prehistoric/core/modules/atmosphere/Atmosphere.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

#include "prehistoric/core/modules/environmentMapRenderer/EnvironmentMapRenderer.h"
#include "prehistoric/core/node/component/audio/AudioComponent.h"

namespace Prehistoric
{
	class Scene
	{
	public:
		Scene(const std::string& worldFile, Window* window, Camera* camera, AssembledAssetManager* manager);
		virtual ~Scene();

		void AddGameObject(const std::string& name, GameObject* object);
		void RemoveGameObject(GameObject* object);

		std::string getName() const { return name; }
		GameObject* getSceneRoot() const { return sceneRoot; }

	protected:
		std::string name = "UnnamedScene";
		
		GameObject* sceneRoot;
		Window* window;
		Camera* camera;
		AssembledAssetManager* manager;

		std::vector<TextureHandle> textures;
		std::vector<MaterialHandle> materials;

		std::vector<VertexBufferHandle> models;
		std::vector<ShaderHandle> shaders;
		std::vector<PipelineHandle> pipelines;
	};
};

#endif