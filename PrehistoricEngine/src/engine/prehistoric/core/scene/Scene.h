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
	/*
		TODO: the scene will have a function later on, but it is just some legacy code right now
	*/
	class PR_API Scene
	{
	public:
		Scene(GameObject* root, Window* window, AssembledAssetManager* manager, Camera* camera, const std::string& worldFile);
		virtual ~Scene() {}

		Scene(const Scene&) = default;
	};
};

#endif