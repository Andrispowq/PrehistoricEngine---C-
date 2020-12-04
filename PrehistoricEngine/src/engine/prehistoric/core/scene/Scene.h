#ifndef SCENE_H
#define SCENE_H

#include "engine/prehistoric/core/node/component/renderer/RendererComponent.h"
#include "world/WorldLoader.h"

#include "engine/prehistoric/core/modules/terrain/Terrain.h"

#include "engine/platform/vulkan/rendering/pipeline/VKPipeline.h"
#include "engine/platform/vulkan/buffer/VKMeshVertexBuffer.h"
#include "engine/platform/vulkan/texture/VkTexture.h"
#include "engine/platform/vulkan/rendering/shaders/pbr/VKPBRShader.h"

#include "engine/prehistoric/core/modules/gui/GUIElement.h"
#include "engine/prehistoric/core/modules/gui/button/GUIButton.h"
#include "engine/prehistoric/core/modules/gui/slider/GUISlider.h"

#include "engine/prehistoric/core/modules/atmosphere/Atmosphere.h"

#include "engine/prehistoric/core/resources/AssembledAssetManager.h"

#include "engine/prehistoric/core/modules/environmentMapRenderer/EnvironmentMapRenderer.h"
#include "engine/prehistoric/core/node/component/audio/AudioComponent.h"

/*
	TODO: the scene will have a function later on, but it is just some legacy code right now
*/
class Scene
{
public:
	Scene(GameObject* root, Window* window, AssembledAssetManager* manager, Camera* camera);
	virtual ~Scene() {}

	Scene(const Scene&) = default;
};

#endif