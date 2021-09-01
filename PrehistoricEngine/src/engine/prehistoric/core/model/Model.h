#ifndef MODEL_H
#define MODEL_H

#include "Mesh.h"

namespace Prehistoric
{
	class Material;

	class Model
	{
	public:
		Model() {}
		~Model() {}

		void AddMesh(Mesh&& mesh) { meshes.push_back(mesh); }
		void AddMaterial(Material* material) { materials.push_back(material); }

		inline std::vector<Mesh> getMeshes() const { return meshes; }
		inline std::vector<Material*> getMaterials() const { return materials; }

		inline Mesh& getMesh(uint32_t index) { return meshes[index]; }

		inline void setMeshes(const std::vector<Mesh>& meshes) { this->meshes = meshes; }
		inline void setMaterials(const std::vector<Material*>& materials) { this->materials = materials; }

	private:
		std::vector<Mesh> meshes;
		std::vector<Material*> materials;
	};
};

#endif