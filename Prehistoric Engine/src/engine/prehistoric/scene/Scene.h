#ifndef SCENE_H
#define SCENE_H

#include "engine/prehistoric/component/renderer/RendererComponent.h"
#include "engine/prehistoric/world/WorldLoader.h"

#include "engine/prehistoric/modules/terrain/Terrain.h"

class Scene
{
public:
	Scene(GameObject* root, Window* window, AssembledAssetManager* manager, Camera* camera);
	virtual ~Scene() {}

	Scene(const Scene&) = default;
};

#endif