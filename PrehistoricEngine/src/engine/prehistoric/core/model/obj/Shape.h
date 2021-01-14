#ifndef SHAPE_H
#define SHAPE_H

#include "Includes.hpp"
#include "prehistoric/core/util/math/Math.h"

namespace Prehistoric
{
	struct index
	{
		int vertex_index;
		int normal_index;
		int texcoord_index;
	};

	class Shape
	{
	public:
		Shape(const std::string& name, std::vector<index> indices = {}, std::vector<int> material_ids = {})
			: name(name), indices(indices), material_ids(material_ids) {}
		~Shape() {}

	public:
		std::string name;
		std::vector<index> indices;
		std::vector<int> material_ids;
	};
};

#endif