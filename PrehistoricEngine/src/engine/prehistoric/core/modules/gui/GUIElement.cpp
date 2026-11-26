#include "Includes.hpp"
#include "GUIElement.h"

#include "prehistoric/application/Application.h"

#include "platform/opengl/rendering/shaders/gui/GLGUIShader.h"
//#include "platform/vulkan/rendering/shaders/basic/VKGUIShader.h"

namespace Prehistoric {
    VertexBufferHandle GUIElement::vbo = {nullptr, static_cast<size_t>(-1)};
    PipelineHandle GUIElement::pipeline = {nullptr, static_cast<size_t>(-1)};

    GUIElement::GUIElement(Window *window, AssembledAssetManager *manager, Vector3f colour, void *data, size_t dataSize,
                           bool visible)
        : window(window), manager(manager), colour(colour), data(data), dataSize(dataSize), visible(visible),
          type(GUIType::Element) {
        if (vbo.handle == -1) {
            ShaderHandle shader;

            vbo = manager->getAssetManager()->storeVertexBuffer(ModelFabricator::CreateQuad(window));
            vbo->setFrontFace(FrontFace::DOUBLE_SIDED);

            if (__FrameworkConfig.api == OpenGL) {
                shader = manager->getAssetManager()->loadShader(ShaderName::Gui).value();
                pipeline = manager->storePipeline(
                    new GLGraphicsPipeline(window, manager->getAssetManager(), shader, vbo));
            } else if (__FrameworkConfig.api == Vulkan) {
                //shader = manager->getAssetManager()->loadShader("gui").value();
                //pipeline = manager->storePipeline(new VKGraphicsPipeline(window, manager->getAssetManager(), shader, vbo));
            }
        }

        if (!pipeline.pointer) return;
        RendererComponent *renderer = new RendererComponent(window, manager, pipeline, manager->storeMaterial(nullptr));
        renderer->SetGenerated(true);
        renderer->setPriority(RenderPriority::_2D);

        AddComponent(RENDERER_COMPONENT, renderer);
    }

    GUIElement::~GUIElement() {
        if (retainedTexture) manager->getAssetManager()->removeReference<Texture>(retainedTexture->handle);
    }

    void GUIElement::setTexture(Texture *value) {
        auto *assets = manager->getAssetManager();
        auto handle = value ? assets->FindTexture(value) : std::nullopt;
        // Existing runtime widgets may borrow renderer-owned debug textures.
        // Duplication rejects those references because their lifetime cannot be retained.
        if (handle) assets->addReference<Texture>(handle->handle);
        if (retainedTexture) assets->removeReference<Texture>(retainedTexture->handle);
        retainedTexture = handle;
        texture = value;
    }

    void GUIElement::OwnData(const void *source, size_t bytes) {
        if (!source || !bytes) {
            ownedData.reset();
            data = nullptr;
            dataSize = 0;
            return;
        }
        ownedData = std::shared_ptr<void>(::operator new(bytes), [](void *p) { ::operator delete(p); });
        std::memcpy(ownedData.get(), source, bytes);
        data = ownedData.get();
        dataSize = bytes;
    }

    void GUIElement::CopyGUIStateTo(GUIElement &copy, size_t payloadSize) const {
        if (data && !payloadSize) throw std::runtime_error("GUI data needs an explicit byte size to be copied");
        if (texture && !retainedTexture) throw std::runtime_error("Cannot duplicate a borrowed runtime GUI texture");
        CopyAuthoredStateTo(copy);
        copy.OwnData(data, payloadSize);
        copy.setTexture(texture);
        copy.statusChanged = false;
    }

    std::unique_ptr<GameObject> GUIElement::Clone() const {
        auto copy = std::make_unique < GUIElement > (window, manager, colour, nullptr, 0, visible);
        CopyGUIStateTo(*copy, dataSize);
        return copy;
    }

    void GUIElement::PreUpdate(CoreEngine *engine) {
        GameObject::PreUpdate(engine);
    }

    void GUIElement::PreRender(Renderer *renderer) {
        if (!visible)
            return;

        GameObject::PreRender(renderer);
    }

    bool GUIElement::inside(Vector2f cursor, Vector2f _start, Vector2f _end) {
        Vector2f pos = worldTransform.getPosition().xy();
        Vector2f scale = worldTransform.getScaling().xy();

        Vector2f start = pos - scale;
        Vector2f end = pos + scale;

        if (_start != -1 && _end != -1) {
            start = _start;
            end = _end;
        }

        cursor.y = window->getHeight() - cursor.y;
        cursor /= {(float) window->getWidth(), (float) window->getHeight()};
        cursor *= 2;
        cursor -= 1;

        if ((start <= cursor) && (end >= cursor)) {
            return true;
        }

        return false;
    }
};