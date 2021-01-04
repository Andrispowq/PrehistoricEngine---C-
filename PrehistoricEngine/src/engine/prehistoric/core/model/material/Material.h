#ifndef MATERIAL_H
#define MATERIAL_H

#include "Includes.hpp"
#include "prehistoric/core/util/math/Math.h"
#include "prehistoric/core/resources/loaders/TextureLoader.h"

#if !defined(HANDLE_OF)
#define HANDLE_OF(type) typedef struct type##_handle_t { type* pointer = nullptr; size_t handle = -1; type* operator->() { return pointer;}; type* const operator->() const { return pointer;} } type##Handle
#endif

namespace Prehistoric
{
	class ResourceStorage;

	class Material
	{
	public:
		Material(ResourceStorage* resourceStorage);
		~Material();

		void addTexture(const std::string& key, TextureHandle texture);
		void addVector4f(const std::string& key, Vector4f value);
		void addVector3f(const std::string& key, Vector3f value);
		void addVector2f(const std::string& key, Vector2f value);
		void addFloat(const std::string& key, float value);
		void addInt(const std::string& key, int value);

		Texture* getTexture(const std::string& key) const;

		Vector4f getVector4f(const std::string& key) const;
		Vector3f getVector3f(const std::string& key) const;
		Vector2f getVector2f(const std::string& key) const;

		float getFloat(const std::string& key) const;
		int getInt(const std::string& key) const;

		Texture* getDefault() const;
		bool exists(const std::string& key) const { return textures.find(key) != textures.end(); }
	private:
		ResourceStorage* resourceStorage;

		std::unordered_map<std::string, TextureHandle> textures;
		std::unordered_map<std::string, Vector4f> vector4s;
		std::unordered_map<std::string, Vector3f> vector3s;
		std::unordered_map<std::string, Vector2f> vector2s;
		std::unordered_map<std::string, float> floats;
		std::unordered_map<std::string, int> ints;
	};

	HANDLE_OF(Material);
};

#endif