#ifndef GRAPHICS_PIPELINE_H
#define GRAPHICS_PIPELINE_H

#include "Pipeline.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

namespace Prehistoric
{
	class GraphicsPipeline
	{
	public:
		GraphicsPipeline(AssetManager* manager, VertexBufferHandle handle);
		GraphicsPipeline() : backfaceCulling(false) {}

		virtual ~GraphicsPipeline();

		VertexBuffer* getVertexBuffer() const { return vbo.pointer; }
		VertexBufferHandle getVertexBufferHandle() const { return vbo; }

		bool isBackfaceCulling() const { return backfaceCulling; }
		void setBackfaceCulling(bool backfaceCulling) { this->backfaceCulling = backfaceCulling; }
	protected:
		AssetManager* manager;

		VertexBufferHandle vbo;
		bool backfaceCulling;
	};
};

#endif