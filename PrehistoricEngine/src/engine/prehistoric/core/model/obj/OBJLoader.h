#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include "prehistoric/core/model/Model.h"

#include "prehistoric/core/util/Constants.h"
#include "prehistoric/common/framework/Window.h"

namespace Prehistoric
{
	class AssetManager;

	class OBJLoader
	{
	public:
		OBJLoader() {}
		OBJLoader(Window* window, AssetManager* manager)
			: window(window), manager(manager) {}
		~OBJLoader() {}

		Model LoadModel(const std::string& path, const std::string& objectFile, const std::string& materialFile);

	private:
		void CalculateTangents(Vertex& v0, Vertex& v1, Vertex& v2);

	private:
		Window* window;
		AssetManager* manager;
	};
};

#endif