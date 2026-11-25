#include "Includes.hpp"
#include "RenderableComponent.h"
#include "prehistoric/core/model/material/Material.h"
#include "prehistoric/common/rendering/pipeline/Pipeline.h"
#include "prehistoric/common/rendering/pipeline/GraphicsPipeline.h"
#include "prehistoric/common/rendering/pipeline/ComputePipeline.h"

#include "platform/vulkan/rendering/shaders/VKShader.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

namespace Prehistoric {
    Material *RenderableComponent::ResolveSubmeshMaterial(const Model &model, uint32_t index,
                                                          Material *fallback) const {
        if (auto *overrideMaterial = getSubmeshOverride(index)) return overrideMaterial;
        const int slot = model.getMesh(index).getMaterialIndex();
        const auto &materials = model.getMaterials();
        return slot >= 0 && size_t(slot) < materials.size() && materials[slot] ? materials[slot] : fallback;
    }

    RenderableComponent::RenderableComponent(Window *window, AssembledAssetManager *manager, PipelineHandle pipeline)
        : priority(RenderPriority::_3D), window(window), manager(manager) {
        type = ComponentType::RenderableComponent;

        this->window = window;
        this->manager = manager;

        this->pipeline = pipeline;
        manager->addReference<Pipeline>(pipeline.handle);
    }

    RenderableComponent::RenderableComponent(Window *window, AssembledAssetManager *manager)
        : priority(RenderPriority::_3D), window(window), manager(manager) {
        type = ComponentType::RendererComponent;
    }

    RenderableComponent::~RenderableComponent() {
        if (pipeline.pointer != nullptr)
            manager->removeReference<Pipeline>(pipeline.handle);
    }
};