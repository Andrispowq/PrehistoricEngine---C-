#include "Includes.hpp"
#include "VKTerrain.h"
#include "platform/vulkan/buffer/VKMeshVertexBuffer.h"
#include "prehistoric/application/Application.h"
#include "prehistoric/core/modules/terrain/TerrainMaps.h"
#include "prehistoric/core/node/component/renderer/RendererComponent.h"

namespace Prehistoric
{
	void CreateVulkanTerrainSurface(GameObject& terrain, Window* window, TerrainMaps& maps, AssembledAssetManager* manager)
	{
		VkPhysicalDeviceFeatures features{};
		vkGetPhysicalDeviceFeatures(*static_cast<VkPhysicalDevice*>(window->getContext()->getNativePhysicalDevice()), &features);
		if (!features.tessellationShader)
		{
			PR_LOG_ERROR("Vulkan terrain requires tessellationShader.\n");
			return;
		}

		// Equal-size triangle patches share both edge endpoints at every LOD.
		constexpr unsigned cells = 32;
		Mesh mesh;
		mesh.setMaterialIndex(-1);
		for (unsigned z = 0; z <= cells; ++z)
		{
			for (unsigned x = 0; x <= cells; ++x)
			{
				Vector2f uv(float(x) / cells, float(z) / cells);
				mesh.AddVertex(Vertex(Vector3f(uv.x - 0.5f, 0, uv.y - 0.5f), uv));
				if (x == cells || z == cells) continue;
				unsigned a = z * (cells + 1) + x;
				unsigned b = a + cells + 1;
				for (unsigned index : {a, b, a + 1, a + 1, b, b + 1}) mesh.AddIndex(index);
			}
		}

		Model model;
		model.AddMesh(std::move(mesh));
		auto* assets = manager->getAssetManager();
		auto vertices = assets->storeVertexBuffer(new VKMeshVertexBuffer(window, model));
		auto shader = assets->loadShader(ShaderName::Terrain).value();
		auto pipeline = manager->createPipeline(PipelineTypeHashFlags::Graphics, shader, vertices);
		auto layers = maps.getMaterials();
		if (layers.empty())
		{
			PR_LOG_ERROR("Vulkan terrain requires at least one surface material.\n");
			return;
		}

		auto* material = new Material(assets, "Terrain");
		material->addTexture("heightMap", maps.getHeightmapHandle());
		for (size_t i = 0; i < 3; ++i)
		{
			auto layer = layers[(std::min)(i, layers.size() - 1)];
			const auto& textures = layer->getTextures();
			const std::string suffix = std::to_string(i);
			for (const char* name : {"albedoMap", "normalMap", "mrotMap"})
			{
				auto found = textures.find(name);
				material->addTexture(name + suffix, found == textures.end() ? textures.at("DEFAULT_TEX") : found->second);
			}

			material->addFloat("tiling" + suffix, layer->getFloat("horizontalScale"));
			material->addVector4f("mrot" + suffix, layer->getVector4f("mrot"));
		}

		terrain.SetPosition(maps.getPosition());
		terrain.SetScale({__TerrainConfig.scaleXZ, __TerrainConfig.scaleY, __TerrainConfig.scaleXZ});
		terrain.AddComponent(RENDERER_COMPONENT, new RendererComponent(window, manager, pipeline, manager->storeMaterial(material)));
	}
}
