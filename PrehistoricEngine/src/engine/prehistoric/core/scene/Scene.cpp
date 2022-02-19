#include "Includes.hpp"
#include "Scene.h"

namespace Prehistoric
{
	Scene::Scene(const std::string& worldFile, Window* window, Camera* camera, AssembledAssetManager* manager)
		: window(window), camera(camera), manager(manager)
	{
		sceneRoot = new GameObject();

		WorldLoader loader(window, manager);

		if (Util::Split(worldFile, '.')[1] == "json")
		{
			loader.LoadWorldJSON(worldFile, sceneRoot);
		}
		else
		{
			loader.LoadWorld(worldFile, sceneRoot);
		}

		name = loader.sceneName;

		auto tex = loader.textures;
		for (auto elem : tex)
		{
			textures.push_back(elem.second);
		}
		auto mat = loader.materials;
		for (auto elem : mat)
		{
			materials.push_back(elem.second);
		}

		auto model = loader.models;
		for (auto elem : model)
		{
			models.push_back(elem.second);
		}
		auto sha = loader.shaders;
		for (auto elem : sha)
		{
			shaders.push_back(elem.second);
		}
		auto pip = loader.pipelines;
		for (auto elem : pip)
		{
			pipelines.push_back(elem.second);
		}
	}

	Scene::~Scene()
	{
		Node* root = sceneRoot->getParent();
		if (root)
		{
			root->RemoveChild(sceneRoot);
		}
	}
};