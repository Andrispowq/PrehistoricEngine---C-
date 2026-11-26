#include "Includes.hpp"
#include "GraphicsPipeline.h"

namespace Prehistoric {
    GraphicsPipeline::GraphicsPipeline(AssetManager *manager, VertexBufferHandle vbo, std::optional<FrontFace> face)
        : frontFace(face.value_or(vbo->getFrontFace())) {
        this->manager = manager;
        this->backfaceCulling = true;

        this->vbo = vbo;
        manager->addReference<VertexBuffer>(vbo.handle);
    }

    GraphicsPipeline::~GraphicsPipeline() {
        if (vbo.pointer != nullptr)
            manager->removeReference<VertexBuffer>(vbo.handle);
    }
};