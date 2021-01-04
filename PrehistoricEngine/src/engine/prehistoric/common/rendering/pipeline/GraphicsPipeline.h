#ifndef GRAPHICS_PIPELINE_H
#define GRAPHICS_PIPELINE_H

#include "Pipeline.h"

#include "prehistoric/core/resources/ResourceStorage.h"

namespace Prehistoric
{
	class GraphicsPipeline
	{
	public:
		GraphicsPipeline(ResourceStorage* resourceStorage, VertexBufferHandle handle);
		GraphicsPipeline() : backfaceCulling(false) {}

		virtual ~GraphicsPipeline();

		VertexBuffer* getVertexBuffer() const { return vbo.pointer; }

		bool isBackfaceCulling() const { return backfaceCulling; }
		void setBackfaceCulling(bool backfaceCulling) { this->backfaceCulling = backfaceCulling; }
	protected:
		ResourceStorage* resourceStorage;

		VertexBufferHandle vbo;
		bool backfaceCulling;
	};
};

#endif