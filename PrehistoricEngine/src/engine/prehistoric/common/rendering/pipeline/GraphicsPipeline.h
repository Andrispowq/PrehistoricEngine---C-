#ifndef GRAPHICS_PIPELINE_H
#define GRAPHICS_PIPELINE_H

#include "Pipeline.h"

#include "prehistoric/core/resources/AssetManager.h"

namespace Prehistoric
{
	class GraphicsPipeline
	{
	public:
		GraphicsPipeline(AssetManager* manager, size_t vboID);
		GraphicsPipeline() : backfaceCulling(false), vboID(-1) {}

		virtual ~GraphicsPipeline();

		VertexBuffer* getVertexBuffer() const { return manager->getResourceByID<VertexBuffer>(vboID); }

		void setVertexBufferID(size_t vboID);

		bool IsBackfaceCulling() const { return backfaceCulling; }
		void SetBackfaceCulling(bool backfaceCulling) { this->backfaceCulling = backfaceCulling; }
	protected:
		AssetManager* manager;

		size_t vboID;

		bool backfaceCulling;
	};
};

#endif