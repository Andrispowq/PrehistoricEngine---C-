#ifndef MATERIAL_H
#define MATERIAL_H

#include "Includes.hpp"
#include "prehistoric/core/util/math/Math.h"
#include "prehistoric/common/texture/TextureLoader.h"

namespace Prehistoric
{
	class AssetManager;

	class Material
	{
	public:
		Material(AssetManager* manager);
		~Material();

		void addTexture(const std::string& key, size_t textureID);
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
		bool exists(const std::string& key) const { return textureIDs.find(key) != textureIDs.end(); }
	private:
		AssetManager* manager;

		std::unordered_map<std::string, size_t> textureIDs;
		std::unordered_map<std::string, Texture*> textures; //Used for faster lookups
		std::unordered_map<std::string, Vector4f> vector4s;
		std::unordered_map<std::string, Vector3f> vector3s;
		std::unordered_map<std::string, Vector2f> vector2s;
		std::unordered_map<std::string, float> floats;
		std::unordered_map<std::string, int> ints;
	};
};

#endif