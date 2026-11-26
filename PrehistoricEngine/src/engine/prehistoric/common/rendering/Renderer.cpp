#include "Includes.hpp"
#include "Renderer.h"
#include "ShadowCascades.h"
#include "prehistoric/core/modules/atmosphere/Atmosphere.h"
#include "prehistoric/core/resources/loaders/TextureLoader.h"

#include "prehistoric/core/node/component/renderer/RenderableComponent.h"
#include "prehistoric/core/node/component/renderer/RendererComponent.h"

#include "prehistoric/core/node/component/light/Light.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"
#include "prehistoric/core/config/EngineSettings.h"

#include "prehistoric/application/Application.h"

namespace Prehistoric {
    Renderer::Renderer(Window *window, Camera *camera, AssembledAssetManager *manager)
        : window(window), camera(camera), manager(manager), wireframeMode(false) {
        flareAtlas.reset(TextureLoader::LoadTexture(window, "engine://textures/sun_flare.png", Bilinear, ClampToEdge));
    }

    Vector4f Renderer::GetSunFlare() const {
        if (!sunFlare || sunFlareStrength <= 0.0f) return {};
        const bool skyDisk = atmosphere && __AtmosphereConfig.scatteringEnabled;
        const Light *sun = skyDisk ? atmosphere->getSun() : ShadowCascades::FindSun(*this);
        if (!sun || sun->getIntensity() <= 0 || sun->getColour().lengthSquared() == 0) return {};
        auto direction = sun->getParent()->getWorldTransform().getPosition();
        if (direction.lengthSquared() == 0) return {};
        direction = direction.normalise();
        if (skyDisk) {
            const float radius = __AtmosphereConfig.planetRadius;
            const float horizon = radius / (radius + 1000.0f);
            if (__AtmosphereConfig.sunIntensity <= 0 || direction.y < -std::sqrt(1.0f - horizon * horizon))
                return {};
            // Invert the sky shader's normalize(viewRay + verticalShift).
            const float shift = __AtmosphereConfig.horizontalVerticalShift;
            const float discriminant = 1 - shift * shift * (1 - direction.y * direction.y);
            if (discriminant <= 0) return {};
            direction *= shift * direction.y + std::sqrt(discriminant);
            direction.y -= shift;
        }
        const auto clip = camera->getProjectionMatrix() * camera->getViewMatrix() * Vector4f(direction, 0);
        if (clip.w <= 0 || std::abs(clip.x) >= clip.w || std::abs(clip.y) >= clip.w) return {};
        return {0.5f + 0.5f * clip.x / clip.w, 0.5f + 0.5f * clip.y / clip.w, sunFlareStrength, sunFlareSize};
    }

    Vector2f Renderer::GetSunFlareFootprint() const {
        const auto &projection = camera->getProjectionMatrix();
        // Sample the bright core of the atmospheric disk, or a solar angular radius for a directional light.
        const float angle = atmosphere && __AtmosphereConfig.scatteringEnabled
                                ? std::max(0.001f, __AtmosphereConfig.sunRadius * 0.1f)
                                : 0.00465f;
        return {0.5f * angle * std::abs(projection.m[0]), 0.5f * angle * std::abs(projection.m[5])};
    }

    void register_model(ModelBatches &batches, RenderableComponent *renderable) {
        auto *component = dynamic_cast<RendererComponent *>(renderable);
        if (!component) return;
        batches[renderable->getPipeline()][component->getMaterial()].push_back(renderable);
    }

    void unregister_model(ModelBatches &batches, RenderableComponent *renderable) {
        auto group = batches.find(renderable->getPipeline());
        if (group == batches.end()) return;
        auto *component = dynamic_cast<RendererComponent *>(renderable);
        if (!component) return;
        auto batch = group->second.find(component->getMaterial());
        if (batch == group->second.end()) return;
        auto &objects = batch->second;
        auto object = std::find(objects.begin(), objects.end(), renderable);
        if (object != objects.end()) objects.erase(object);
        if (objects.empty()) group->second.erase(batch);
        if (group->second.empty()) batches.erase(group);
    }

    void register_2d_model(std::unordered_map<Pipeline *, std::vector<RenderableComponent *> > &map,
                           RenderableComponent *renderable) {
        map[renderable->getPipeline()].push_back(renderable);
    }

    void unregister_2d_model(std::unordered_map<Pipeline *, std::vector<RenderableComponent *> > &map,
                             RenderableComponent *renderable) {
        Pipeline *pipeline = renderable->getPipeline();
        auto pipeIdx = map.find(pipeline);

        if (pipeIdx != map.end()) {
            auto &renderables = pipeIdx->second;
            auto rendIdx = std::find(renderables.begin(), renderables.end(), renderable);

            if (rendIdx != renderables.end()) renderables.erase(rendIdx);
        }
    }

    void Renderer::Render() {
        temporalRendered = false;
        if (!CanRenderFrame()) {
            ResetTemporalHistory();
            return;
        }
        UpdateEnvironmentLighting();
        const bool useTemporal = UseTemporalEffects();
        if (useTemporal) {
            temporalHistory.Begin(camera->getProjectionMatrix(), camera->getViewMatrix(),
                                  window->getWidth(), window->getHeight(), temporal.antialiasing);
            for (const auto *batches: {&models_3d, &models_transparency})
                VisitBatches(*batches, [](Pipeline *) {
                             }, [](Pipeline *, Material *) {
                             },
                             [&](Pipeline *, RenderableComponent *object) {
                                 temporalHistory.pendingModels.emplace(object->getTemporalIdentity(),
                                                                       object->getParent()->getWorldTransform().
                                                                       getTransformationMatrix());
                             }, [](Pipeline *) {
                             });
            camera->setProjectionMatrix(temporalHistory.JitteredProjection());
            camera->setViewProjectionMatrix(camera->getProjectionMatrix() * camera->getViewMatrix());
        } else ResetTemporalHistory();
        RenderScene();
        RenderHDR();
        if (useTemporal) {
            camera->setProjectionMatrix(temporalHistory.projection);
            camera->setViewProjectionMatrix(temporalHistory.current);
        }
        FinishFrame();
    }

    Renderer::TemporalParameters Renderer::GetTemporalParameters() const {
        const auto &h = temporalHistory;
        auto rotation = h.view;
        auto previousRotation = h.previousView;
        rotation.m[12] = rotation.m[13] = rotation.m[14] = 0;
        previousRotation.m[12] = previousRotation.m[13] = previousRotation.m[14] = 0;
        TemporalParameters result;
        const auto reprojection = h.projection * previousRotation * (h.projection * rotation).Invert();
        std::copy_n(reprojection.m, 16, result.reprojection);
        static_assert(sizeof(TemporalParameters) == 128);
        result.jitter = {h.jitter.x, h.jitter.y, h.previousJitter.x, h.previousJitter.y};
        result.settings = {
            temporal.motionBlur ? std::clamp(temporal.shutterAngle, 0.0f, 360.0f) / 360.0f : 0.0f,
            std::clamp(temporal.maxBlurPixels, 0.0f, 64.0f),
            float(std::clamp(temporal.samples, 4, 32)), float(h.valid)
        };
        result.options = {
            float(window->getWidth()), float(window->getHeight()),
            float(temporal.antialiasing), float(temporal.debug)
        };
        result.depthRow = {-h.view.m[2], -h.view.m[6], -h.view.m[10], -h.view.m[14]};
        return result;
    }

    void Renderer::CommitTemporalFrame() {
        if (temporalRendered) temporalHistory.Commit();
        else temporalHistory.Reset();
    }

    void Renderer::ClearFrameSubmissions() {
        models_3d.clear();
        models_transparency.clear();
        models_water.clear();
        models_2d.clear();
        lights.clear();
        volumetricSpheres.clear();
        atmosphere = nullptr;
    }

    void Renderer::ResizePipelines(uint32_t width, uint32_t height) {
        for (Pipeline *pipeline: manager->get<Pipeline>()) {
            pipeline->setViewportSize({float(width), float(height)});
            pipeline->setScissorSize({width, height});
            pipeline->RecreatePipeline();
        }
    }

    void Renderer::RegisterModel(RenderableComponent *renderable) {
        switch (renderable->getPriority()) {
            case RenderPriority::_3D:
                register_model(models_3d, renderable);
                if (renderable->hasReflection()) {
                    register_model(models_water, renderable);
                }
                break;
            case RenderPriority::_TRANSPARENCY:
                register_model(models_transparency, renderable);
                break;
            case RenderPriority::_2D:
                register_2d_model(models_2d, renderable);
                break;
            default:
                break;
        }
    }

    void Renderer::RegisterLight(Light *light) {
        if (__FrameworkConfig.api == Vulkan && !light->illuminates() && !light->castShadows()) return;
        lights.push_back(light);
    }

    void Renderer::UnregisterModel(RenderableComponent *renderable) {
        switch (renderable->getPriority()) {
            case RenderPriority::_3D:
                unregister_model(models_3d, renderable);
                unregister_model(models_water, renderable);
                break;
            case RenderPriority::_TRANSPARENCY:
                unregister_model(models_transparency, renderable);
                break;
            case RenderPriority::_2D:
                unregister_2d_model(models_2d, renderable);
                break;
            default:
                break;
        }
    }

    void Renderer::UnregisterLight(Light *light) {
        auto idx = std::find(lights.begin(), lights.end(), light);
        if (idx != lights.end()) {
            lights.erase(idx);
        }
    }
};