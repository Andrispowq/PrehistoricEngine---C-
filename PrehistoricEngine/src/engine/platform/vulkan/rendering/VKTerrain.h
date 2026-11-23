#pragma once

namespace Prehistoric
{
	class GameObject;
	class TerrainMaps;
	class Window;
	class AssembledAssetManager;

	void CreateVulkanTerrainSurface(GameObject& terrain, Window* window, TerrainMaps& maps, AssembledAssetManager* manager);
}
