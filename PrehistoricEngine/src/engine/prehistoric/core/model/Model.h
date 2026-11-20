#ifndef MODEL_H
#define MODEL_H

#include "Mesh.h"
#include "prehistoric/common/texture/Texture.h"

namespace Prehistoric
{
	class Material;

	class Model
	{
	public:
		Model();
		~Model();

		void AddMesh(Mesh&& mesh) { meshes.push_back(mesh); }
		void AddMaterial(Material* material) { materials.push_back(material); }
		void AddTextureLocation(Texture* texture, std::string location) { texture_locations.emplace(std::make_pair(texture, location)); }

		inline std::vector<Mesh> getMeshes() const { return meshes; }
		inline std::vector<Material*> getMaterials() const { return materials; }
		inline std::map<Texture*, std::string> getTextureLocations() const { return texture_locations; }

		inline Mesh& getMesh(uint32_t index) { return meshes[index]; }

		inline void setMeshes(const std::vector<Mesh>& meshes) { this->meshes = meshes; }
		inline void setMaterials(const std::vector<Material*>& materials) { this->materials = materials; }
		inline void setTextureLocations(const std::map<Texture*, std::string>& texLocations) { this->texture_locations = texLocations; }

	private:
		std::string name;
		uint64_t ID;

		std::vector<Mesh> meshes;
		std::vector<Material*> materials;

		std::map<Texture*, std::string> texture_locations;
	};
};

#endif