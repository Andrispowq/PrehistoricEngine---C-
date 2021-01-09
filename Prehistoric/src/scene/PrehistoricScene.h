#ifndef PREHISTORIC_SCENE_H
#define PREHISTORIC_SCENE_H

#include "prehistoric/core/scene/Scene.h"

class PrehistoricScene : public Prehistoric::Scene
{
public:
	PrehistoricScene(const std::string& name, Prehistoric::GameObject* root, Prehistoric::Window* window, Prehistoric::Camera* camera, 
		Prehistoric::AssembledAssetManager* manager, const std::string& worldFile);
	virtual ~PrehistoricScene();
};

#endif