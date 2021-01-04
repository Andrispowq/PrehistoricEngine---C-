#include "Includes.hpp"
#include "GraphicsPipeline.h"

namespace Prehistoric
{
	GraphicsPipeline::GraphicsPipeline(ResourceStorage* resourceStorage, VertexBufferHandle vbo)
	{
		this->resourceStorage = resourceStorage;
		this->backfaceCulling = true;

		this->vbo = vbo;
		resourceStorage->addReference<VertexBuffer>(vbo.handle);
	}

	GraphicsPipeline::~GraphicsPipeline()
	{
		resourceStorage->removeReference<VertexBuffer>(vbo.handle);
	}
};
