#ifndef VERTEX_H
#define VERTEX_H

#include "engine/prehistoric/core/util/Includes.hpp"
#include "engine/prehistoric/core/util/math/Math.h"

namespace Prehistoric
{
	class Vertex
	{
	public:
		Vertex(const Vector3f& position, const Vector2f& texture, const Vector3f& normal, const Vector3f& tangent) : position(position), texture(texture), normal(normal), tangent(tangent), textureIndex(-1), normalIndex(-1) {}
		Vertex(const Vector3f& position, const Vector2f& texture, const Vector3f& normal) : position(position), texture(texture), normal(normal), textureIndex(-1), normalIndex(-1) {}
		Vertex(const Vector3f& position, const Vector2f& texture) : position(position), texture(texture), normal(Vector3f(0)), textureIndex(-1), normalIndex(-1) {}
		Vertex(const Vector3f& position) : position(position), texture(Vector2f(0)), normal(Vector3f(0)), textureIndex(-1), normalIndex(-1) {}
		Vertex() : position(Vector3f(0)), texture(Vector2f(0)), normal(Vector3f(0)), textureIndex(-1), normalIndex(-1) {}

		Vertex(uint16_t index, Vector3f position) : index(index), position(position), textureIndex(-1), normalIndex(-1) {}

		~Vertex() {}

		constexpr static size_t getNumberOfFloats() { return 11; }
		constexpr static size_t getSize() { return getNumberOfFloats() * sizeof(float); }

		inline bool isSet() const { return textureIndex != -1 && normalIndex != -1; }
		inline bool hasSameTextureAndNormal(int textureIndex, int normalIndex) const { return this->textureIndex == textureIndex && this->normalIndex == normalIndex; }
		inline bool hasTangent() const { return tangent == 0; }

		inline Vector3f getPosition() const { return position; }
		inline Vector2f getTexture() const { return texture; }
		inline Vector3f getNormal() const { return normal; }
		inline Vector3f getTangent() const { return tangent; }

		inline uint16_t getIndex() const { return index; }
		inline int getTextureIndex() const { return textureIndex; }
		inline int getNormalIndex() const { return normalIndex; }

		inline size_t getDuplicateVertexIndex() const { return duplicate_vert; }

		inline void setPosition(const Vector3f& position) { this->position = position; }
		inline void setTexture(const Vector2f& texture) { this->texture = texture; }
		inline void setNormal(const Vector3f& normal) { this->normal = normal; }
		inline void addTangent(const Vector3f& tangent) { this->tang_sum += tangent; }

		inline void setIndex(uint16_t index) { this->index = index; }
		inline void setTextureIndex(int textureIndex) { this->textureIndex = textureIndex; }
		inline void setNormalIndex(int normalIndex) { this->normalIndex = normalIndex; }

		inline void setDuplicateVertexIndex(size_t duplicate_vert) { this->duplicate_vert = duplicate_vert; }

		inline void averageTangents() { tangent = tang_sum.normalise(); }

	public:
		Vector3f position;
		Vector2f texture;
		Vector3f normal;
		Vector3f tangent = 0; //Some default value, might not be used

		Vector3f tang_sum = 0;

		size_t duplicate_vert = -1;

		uint16_t index = -1;
		int textureIndex = -1;
		int normalIndex = -1;
	};
};

#endif