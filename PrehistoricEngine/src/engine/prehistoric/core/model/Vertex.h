#ifndef VERTEX_H
#define VERTEX_H

#include "Includes.hpp"
#include "prehistoric/core/util/math/Math.h"

#include <unordered_map>

namespace Prehistoric
{
	class Vertex
	{
	public:
		Vertex(const Vector3f& position, const Vector2f& texture, const Vector3f& normal, const Vector3f& tangent) : position(position), texture(texture), normal(normal), tangent(tangent) {}
		Vertex(const Vector3f& position, const Vector2f& texture, const Vector3f& normal) : position(position), texture(texture), normal(normal) {}
		Vertex(const Vector3f& position, const Vector2f& texture) : position(position), texture(texture), normal(Vector3f(0)) {}
		Vertex(const Vector3f& position) : position(position), texture(Vector2f(0)), normal(Vector3f(0)) {}
		Vertex() : position(Vector3f(0)), texture(Vector2f(0)), normal(Vector3f(0)) {}

		~Vertex() {}

		bool operator==(const Vertex& other) const 
		{
			return position == other.position && texture == other.texture && normal == other.normal;
		}

		constexpr static size_t getNumberOfFloats() { return 11; }
		constexpr static size_t getSize() { return getNumberOfFloats() * sizeof(float); }

		inline bool hasTangent() const { return tangent == 0; }

		inline Vector3f getPosition() const { return position; }
		inline Vector2f getTexture() const { return texture; }
		inline Vector3f getNormal() const { return normal; }
		inline Vector3f getTangent() const { return tangent; }

		inline size_t getID() const { return Id; }

		inline void setPosition(const Vector3f& position) { this->position = position; }
		inline void setTexture(const Vector2f& texture) { this->texture = texture; }
		inline void setNormal(const Vector3f& normal) { this->normal = normal; }
		inline void addTangent(const Vector3f& tangent) { this->tang_sum += tangent; }

		inline void setID(size_t id) { this->Id = id; }

		inline void averageTangents() { tangent = tang_sum.normalise(); }

	public:
		Vector3f position;
		Vector2f texture;
		Vector3f normal;
		Vector3f tangent = 0; //Some default value, might not be used

		Vector3f tang_sum = 0;
		size_t Id;
	};
};

#endif