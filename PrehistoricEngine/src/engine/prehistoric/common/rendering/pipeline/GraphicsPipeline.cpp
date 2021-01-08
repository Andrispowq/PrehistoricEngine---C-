#include "Includes.hpp"
#include "GraphicsPipeline.h"

namespace Prehistoric
{
	GraphicsPipeline::GraphicsPipeline(AssetManager* manager, VertexBufferHandle vbo)
	{
		this->manager = manager;
		this->backfaceCulling = true;

		this->vbo = vbo;
		manager->addReference<VertexBuffer>(vbo.handle);
	}

	GraphicsPipeline::~GraphicsPipeline()
	{
		manager->removeReference<VertexBuffer>(vbo.handle);
	}
};
