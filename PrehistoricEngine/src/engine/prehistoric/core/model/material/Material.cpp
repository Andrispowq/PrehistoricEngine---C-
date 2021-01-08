#include "Includes.hpp"
#include "Material.h"

namespace Prehistoric
{
	Material::Material(AssetManager* manager)
	{
		this->manager = manager;

		TextureHandle def = manager->loadTexture("res/textures/default.png").value();
		manager->addReference<Texture>(def.handle);
		textures.emplace(std::make_pair("DEFAULT_TEX", def));
	}

	Material::~Material()
	{
		for (auto& tex : textures)
		{
			manager->removeReference<Texture>(tex.second.handle);
		}
	}

	void Material::addTexture(const std::string& key, TextureHandle texture)
	{
		manager->addReference<Texture>(texture.handle);
		textures.emplace(std::make_pair(key, texture));
	}

	void Material::addVector4f(const std::string& key, Vector4f value)
	{
		vector4s.emplace(std::make_pair(key, value));
	}

	void Material::addVector3f(const std::string& key, Vector3f value)
	{
		vector3s.emplace(std::make_pair(key, value));
	}

	void Material::addVector2f(const std::string& key, Vector2f value)
	{
		vector2s.emplace(std::make_pair(key, value));
	}

	void Material::addFloat(const std::string& key, float value)
	{
		floats.emplace(std::make_pair(key, value));
	}

	void Material::addInt(const std::string& key, int value)
	{
		ints.emplace(std::make_pair(key, value));
	}

	Texture* Material::getTexture(const std::string& key) const
	{
		auto index = textures.find(key);
		if (index == textures.end())
			return textures.begin()->second.pointer;

		return index->second.pointer;
	}

	Vector4f Material::getVector4f(const std::string& key) const
	{
		auto index = vector4s.find(key);
		if (index == vector4s.end())
			return Vector4f(-1);

		return index->second;
	}

	Vector3f Material::getVector3f(const std::string& key) const
	{
		auto index = vector3s.find(key);
		if (index == vector3s.end())
			return Vector3f(-1);

		return index->second;
	}

	Vector2f Material::getVector2f(const std::string& key) const
	{
		auto index = vector2s.find(key);
		if (index == vector2s.end())
			return Vector2f(-1);

		return index->second;
	}

	float Material::getFloat(const std::string& key) const
	{
		auto index = floats.find(key);
		if (index == floats.end())
			return -1.0;

		return index->second;
	}

	int Material::getInt(const std::string& key) const
	{
		auto index = ints.find(key);
		if (index == ints.end())
			return -1;

		return index->second;
	}

	Texture* Material::getDefault() const
	{
		return textures.at("DEFAULT_TEX").pointer;
	}
};
