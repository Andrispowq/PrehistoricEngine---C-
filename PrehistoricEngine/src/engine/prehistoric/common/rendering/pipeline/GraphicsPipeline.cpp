#include "engine/prehistoric/core/util/Includes.hpp"
#include "GraphicsPipeline.h"

namespace Prehistoric
{
	GraphicsPipeline::GraphicsPipeline(AssetManager* manager, size_t vboID)
	{
		this->manager = manager;
		this->backfaceCulling = true;

		this->vboID = vboID;
		manager->addReference<VertexBuffer>(vboID);
	}

	GraphicsPipeline::~GraphicsPipeline()
	{
		manager->removeReference<VertexBuffer>(vboID);
		vboID = -1;
	}

	void GraphicsPipeline::setVertexBufferID(size_t vboID)
	{
		this->vboID = vboID;
	}
};
