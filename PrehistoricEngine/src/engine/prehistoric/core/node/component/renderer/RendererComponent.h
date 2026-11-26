#ifndef RENDERER_COMPONENT_H
#define RENDERER_COMPONENT_H

#include "Includes.hpp"

#include "RenderableComponent.h"

namespace Prehistoric {
    class Material;
    class MeshVertexBuffer;

    class RendererComponent : public RenderableComponent {
    public:
        RendererComponent(Window *window, AssembledAssetManager *manager, PipelineHandle pipeline,
                          MaterialHandle material);

        RendererComponent(Window *window, AssembledAssetManager *manager);

        virtual ~RendererComponent();

        std::unique_ptr<Component> Clone() const override;

        void PreRender(Renderer *renderer) override;

        void Render(Renderer *renderer) const override;

        void BatchRender(uint32_t instance_index = 0) const override;

        MeshVertexBuffer *getMeshBuffer() const;

        Material *getSubmeshOverride(uint32_t index) const override;

        Material *EditSubmesh(uint32_t index);

        void ResetSubmesh(uint32_t index) { overrides.erase(index); }
        const auto &getSubmeshOverrides() const { return overrides; }

        Material *getMaterial() const { return material.pointer; }
        MaterialHandle getMaterialHandle() const { return material; }

        void setMaterial(MaterialHandle handle) {
            this->material.handle = material.handle;
            this->material.pointer = material.pointer;
        }

        static ComponentType getStaticComponentType() { return ComponentType::RendererComponent; }

    private:
        MaterialHandle material;
        std::map<uint32_t, std::unique_ptr<Material> > overrides;
    };
};

#endif