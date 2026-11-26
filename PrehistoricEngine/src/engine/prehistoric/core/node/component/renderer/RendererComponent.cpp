#include "Includes.hpp"
#include "RendererComponent.h"

#include "prehistoric/application/Application.h"

#include "prehistoric/common/rendering/pipeline/Pipeline.h"
#include "prehistoric/core/engines/RenderingEngine.h"
#include "prehistoric/core/node/GameObject.h"
#include "prehistoric/core/model/material/Material.h"

namespace Prehistoric {
    static std::unique_ptr<Material> CloneMaterial(Material &source, AssembledAssetManager &manager) {
        auto copy = std::make_unique<Material>(manager.getAssetManager(), source.getName());
        for (const auto &[key, texture]: source.getTextures()) {
            if (key != "DEFAULT_TEX") copy->addTexture(key, texture);
        }
        copy->getVector4fs() = source.getVector4fs();
        copy->getVector3fs() = source.getVector3fs();
        copy->getVector2fs() = source.getVector2fs();
        copy->getFloats() = source.getFloats();
        copy->getInts() = source.getInts();
        return copy;
    }

    RendererComponent::RendererComponent(Window *window, AssembledAssetManager *manager, PipelineHandle pipeline,
                                         MaterialHandle material)
        : RenderableComponent(window, manager, pipeline), material(material) {
        type = ComponentType::RendererComponent;

        manager->addReference<Material>(material.handle);
    }

    RendererComponent::RendererComponent(Window *window, AssembledAssetManager *manager)
        : RenderableComponent(window, manager) {
        type = ComponentType::RendererComponent;
    }

    RendererComponent::~RendererComponent() {
        //We can actually have null materials in here, hence the check against the handle not the pointer
        if (material.handle != -1)
            manager->removeReference<Material>(material.handle);
    }

    MeshVertexBuffer *RendererComponent::getMeshBuffer() const {
        auto *graphics = dynamic_cast<GraphicsPipeline *>(getPipeline());
        return graphics ? dynamic_cast<MeshVertexBuffer *>(graphics->getVertexBuffer()) : nullptr;
    }

    Material *RendererComponent::getSubmeshOverride(uint32_t index) const {
        const auto it = overrides.find(index);
        return it == overrides.end() ? nullptr : it->second.get();
    }

    Material *RendererComponent::EditSubmesh(uint32_t index) {
        if (auto *existing = getSubmeshOverride(index)) return existing;
        auto *mesh = getMeshBuffer();
        if (!mesh || index >= mesh->getSubmeshCount()) return nullptr;
        auto *source = ResolveSubmeshMaterial(mesh->getModel(), index, getMaterial());
        if (!source) return nullptr;
        return overrides.emplace(index, CloneMaterial(*source, *manager)).first->second.get();
    }

    std::unique_ptr<Component> RendererComponent::Clone() const {
        auto copy = std::make_unique < RendererComponent > (window, manager);
        copy->pipeline = pipeline;
        copy->material = material;
        if (pipeline.pointer) manager->addReference<Pipeline>(pipeline.handle);
        if (material.handle != -1) manager->addReference<Material>(material.handle);
        copy->setPriority(priority);
        copy->setReflection(reflect_water);
        for (const auto &[index, source]: overrides) {
            copy->overrides.emplace(index, CloneMaterial(*source, *manager));
        }
        return copy;
    }

    void RendererComponent::PreRender(Renderer *renderer) {
        if (pipeline.pointer == nullptr) {
            return;
        }

        renderer->RegisterModel(this);
    }

    void RendererComponent::Render(Renderer *renderer) const {
        pipeline->BindPipeline(renderer->getDrawCommandBuffer());
        pipeline->getShader()->UpdateGlobalUniforms(renderer);
        pipeline->getShader()->UpdateMaterialUniforms(material.pointer, 0);
        pipeline->getShader()->UpdateObjectUniforms(parent);

        pipeline->RenderObject(*this);
        pipeline->UnbindPipeline();
    }

    void RendererComponent::BatchRender(uint32_t instance_index) const {
        pipeline->getShader()->UpdateObjectUniforms(parent);
        pipeline->RenderObject(*this);
    }
};