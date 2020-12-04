#include "engine/prehistoric/core/util/Includes.hpp"
#include "Material.h"

#include "engine/prehistoric/resources/AssetManager.h"

Material::Material(AssetManager* manager, Window* window)
{
	this->manager = manager;

	size_t def_ID = manager->getResource<Texture>("res/textures/default.png");
	manager->addReference<Texture>(def_ID);
	textureIDs.insert(std::make_pair("DEFAULT_TEX", def_ID));
}

Material::~Material()
{
	for (auto& tex : textureIDs)
	{
		manager->removeReference<Texture>(tex.second);
	}
}

void Material::addTexture(const std::string& key, size_t textureID)
{
	manager->addReference<Texture>(textureID);
	textureIDs.insert(std::make_pair(key, textureID));
}

void Material::addVector4f(const std::string& key, Vector4f value)
{
	vector4s.insert(std::make_pair(key, value));
}

void Material::addVector3f(const std::string& key, Vector3f value)
{
	vector3s.insert(std::make_pair(key, value));
}

void Material::addVector2f(const std::string& key, Vector2f value)
{
	vector2s.insert(std::make_pair(key, value));
}

void Material::addFloat(const std::string& key, float value)
{
	floats.insert(std::make_pair(key, value));
}

void Material::addInt(const std::string& key, int value)
{
	ints.insert(std::make_pair(key, value));
}

Texture* Material::getTexture(const std::string& key) const
{
	auto index = textureIDs.find(key);
	if (index == textureIDs.end())
		 return manager->getResourceByID<Texture>(textureIDs.begin()->second);
	
	return manager->getResourceByID<Texture>(index->second);
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
	return manager->getResourceByID<Texture>(textureIDs.at("DEFAULT_TEX"));
}
