#include "Includes.hpp"
#include "VKRenderer.h"
#include "VKShadowPass.h"
#include "VKVoxelView.h"
#include "VKWaterSimulation.h"
#include "prehistoric/core/modules/water/Water.h"
#include "prehistoric/core/modules/water/WaterCapture.h"
#include "platform/vulkan/rendering/pipeline/VKGraphicsPipeline.h"
#include "VKEnvironmentBackground.h"
#include "VKAtmosphereParameters.h"
#include "VKEnvironmentLighting.h"
#include "platform/vulkan/texture/VKTexture.h"
#include "prehistoric/application/Application.h"
#include "prehistoric/core/resources/loaders/TextureLoader.h"

#include "prehistoric/core/node/component/renderer/RenderableComponent.h"

#include "platform/vulkan/framework/context/VKContext.h"
#include "platform/vulkan/framework/swapchain/VKSwapchain.h"
#include "platform/vulkan/rendering/shaders/VkShader.h"

#include "prehistoric/core/resources/AssetManager.h"

namespace Prehistoric {
    VKRenderer::VKRenderer(Window *window, Camera *camera, AssembledAssetManager *manager)
        : Renderer(window, camera, manager), commandPool(nullptr), renderpass(nullptr) {
        VKContext *context = (VKContext *) window->getContext();
        VKSwapchain *swapchain = (VKSwapchain *) window->getSwapchain();

        VKDevice *device = (VKDevice *) context->getDevices();

        shadows = std::make_unique<VKShadowPass>(device);
        renderpass = std::make_unique<VKRenderpass>(device, swapchain,
                                                    static_cast<VKTexture *>(getFlareAtlas())->getImage()->
                                                    getImageView());

        size_t NumImages = swapchain->getSwapchainImages().size();

        //Create command pool
        commandPool = std::make_unique<VKCommandPool>(device);

        for (size_t i = 0; i < NumImages; i++) {
            commandPool->AddCommandBuffer();
        }

        swapchain->setRenderpass(renderpass.get());
        //A nastly little cheat to avoid modifying everything in the Pipeline system because the Vulkan Graphics Pipelines require the renderpass
        background = std::make_unique<VKEnvironmentBackground>(window);
    }

    void VKRenderer::RequestEnvironmentBake() {
        bakeRequested = true;
    }

    Texture *VKRenderer::getOutputTexture() const {
        return renderpass->getEditorOutput();
    }

    void VKRenderer::UpdateEnvironmentLighting() {
        auto capture = EnvironmentCaptureSource::FromScene(
            atmosphere, 0, environmentObject);
        capture.includeSunDisk = !capture.scattering || ShadowCascades::FindSun(*this) != atmosphere->getSun();
        if (!bakeRequested && !environmentCapture.NeedsBake(capture, __EnvironmentMapConfig.atmosphereRefreshSeconds))
            return;
        ResetTemporalHistory();
        bool captureSky = capture.scattering;
        auto *device = static_cast<VKDevice *>(window->getContext()->getDevices());
        VkDescriptorImageInfo source{};
        std::unique_ptr<VKTexture> fallback;
        if (!captureSky) {
            source = background->LoadSource(window, capture.panorama);
            if (!source.imageView) {
                fallback.reset(static_cast<VKTexture *>(TextureLoader::LoadTexture(window,
                    "engine://textures/default.png", Bilinear, ClampToEdge)));
                source = {
                    fallback->getSampler(), fallback->getImage()->getImageView(),
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                };
            }
        }

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device->getPhysicalDevice(), &properties);
        const auto limit = properties.limits.maxImageDimensionCube;
        const bool editor = Application::Get().isEditor();
        // The sample caps startup bake cost. The editor exposes these sizes to the user.
        uint32_t irradianceSize = std::clamp(__EnvironmentMapConfig.irradianceMapResolution, 1u, editor ? limit : 64u);
        uint32_t prefilterSize = std::clamp(__EnvironmentMapConfig.prefilterMapResolution, editor ? 1u : 2u,
                                            editor ? limit : 256u);
        uint32_t levels = editor
                              ? std::max(1u, __EnvironmentMapConfig.prefilterLevels)
                              : std::clamp(__EnvironmentMapConfig.prefilterLevels, 2u, 9u);
        const char *captureShader = captureSky ? "vulkan/atmosphere/capture_CS.spv" : "vulkan/ibl/convert_CS.spv";
        environmentLighting = std::make_unique<VKEnvironmentLighting>(
            device->getPhysicalDevice(), device->getDevice(), device->getGraphicsQueue().getQueue(),
            device->getQueueFamilyIndices().graphicsFamily, source, irradianceSize, prefilterSize, levels,
            std::clamp(__EnvironmentMapConfig.environmentMapResolution, 1u, limit),
            ResourceLoader::LoadShaderVK(captureShader),
            ResourceLoader::LoadShaderVK("vulkan/ibl/ibl_CS.spv"), VKEnvironmentLighting::DefaultLutSize,
            VKEnvironmentLighting::DefaultSamples, captureSky ? &capture.sky : nullptr, environmentLighting.get(),
            capture.includeSunDisk, capture.worldToEnvironment.m);
        environmentCapture.Complete(capture);
        bakeRequested = false;
        if (!captureSky && !fallback) background->SetPrefilter(environmentLighting->getPrefilter());
    }

    VKRenderer::~VKRenderer() {
        vkDeviceWaitIdle(((VKDevice *) window->getContext()->getDevices())->getDevice());
    }

    RenderExtent VKRenderer::GetRenderExtent() const {
        const auto extent = static_cast<VKSwapchain *>(window->getSwapchain())->getSwapchainExtent();
        return {extent.width, extent.height};
    }

    std::vector<unsigned> VKRenderer::GetSupportedSampleCounts() const {
        return static_cast<VKSwapchain *>(window->getSwapchain())->GetSupportedSampleCounts();
    }

    unsigned VKRenderer::GetSampleCount() const {
        return unsigned(static_cast<VKSwapchain *>(window->getSwapchain())->getSceneSamples());
    }

    void VKRenderer::PrepareRendering() {
        VKSwapchain *swapchain = (VKSwapchain *) window->getSwapchain();
        frameReady = false;
        int framebufferWidth, framebufferHeight;
        glfwGetFramebufferSize((GLFWwindow *) window->getWindowHandle(), &framebufferWidth, &framebufferHeight);
        if (framebufferWidth == 0 || framebufferHeight == 0)
            return;

        if (ApplyPendingSampleCount()) window->setResized(true);

        if (window->isResized()) {
            PR_PROFILE("Resizing window");

            VKContext *context = (VKContext *) window->getContext();
            VKDevice *device = (VKDevice *) context->getDevices();
            vkDeviceWaitIdle(device->getDevice());

            ResetTemporalHistory();

            renderpass.reset();

            swapchain->SetWindowSize((uint32_t) framebufferWidth, (uint32_t) framebufferHeight);
            VkExtent2D extent = swapchain->getSwapchainExtent();
            uint32_t width = extent.width;
            uint32_t height = extent.height;
            camera->SetProjection(camera->getFov(), (float) width, (float) height);

            renderpass = std::make_unique<VKRenderpass>(device, swapchain,
                                                        static_cast<VKTexture *>(getFlareAtlas())->getImage()->
                                                        getImageView());
            swapchain->setRenderpass(renderpass.get());

            size_t NumImages = swapchain->getSwapchainImages().size();

            //Create command pool
            commandPool->DeleteCommandBuffers();
            for (size_t i = 0; i < NumImages; i++) {
                commandPool->AddCommandBuffer();
            }

            ResizePipelines(width, height);

            background->RecreatePipeline();

            window->setResized(false);
        }

        if (!swapchain->GetNextImageIndex())
            return;
        frameReady = true;
    }

    void VKRenderer::EndRendering() {
        if (frameReady) {
            PR_PROFILE("Submitting");
            window->Render(getDrawCommandBuffer());
            CommitTemporalFrame();
        }

        waters.clear();
        ClearFrameSubmissions();
    }

    bool VKRenderer::SupportsVoxelResolution(int size) const {
        auto *gpu = static_cast<VKDevice *>(window->getContext()->getDevices());
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(gpu->getPhysicalDevice(), &properties);
        VkPhysicalDeviceMemoryProperties memory;
        vkGetPhysicalDeviceMemoryProperties(gpu->getPhysicalDevice(), &memory);
        VkDeviceSize heap = 0;
        for (unsigned i = 0; i < memory.memoryHeapCount; ++i)
            if (memory.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                heap = std::max(heap, memory.memoryHeaps[i].size);
        auto bytes = VKVoxelView::GridBytes(size, voxelGI || voxelSunShadows);
        return bytes <= properties.limits.maxStorageBufferRange && bytes <= heap / 4 && bytes <= 768ull * 1024 * 1024;
    }

    VkDescriptorImageInfo VKRenderer::GetVoxelScreenInput(int index) const {
        return (index >= 3
                    ? screenSpaceReflections
                    : voxelGI || (index < 2 && (ambientOcclusion.active() || screenSpaceReflections)))
                   ? renderpass->VoxelScreenInput(index, voxelHalfResolution)
                   : environmentLighting->getBRDF();
    }

    VkBuffer VKRenderer::GetVoxelBuffer() const {
        return voxelGI || voxelSunShadows ? renderpass->Voxels(voxelResolution, true).Buffer() : VK_NULL_HANDLE;
    }

    void VKRenderer::RenderScene() {
        while ((voxelView || voxelGI || voxelSunShadows) && voxelResolution > 64 && !SupportsVoxelResolution(
                   voxelResolution))
            voxelResolution /= 2;
        if (voxelView) {
            getDrawCommandBuffer()->BindBuffer();
            return;
        }
        renderpass->RetainWaterTargets(waters);
        for (auto *water: waters) {
            renderpass->GetWaterTargets(water);
            water->captureDepthScale = MakeWaterCapture(camera->getViewMatrix(),
                                                        UseTemporalEffects()
                                                            ? temporalHistory.projection
                                                            : camera->getProjectionMatrix(),
                                                        camera->getPosition(), water->height(), false, true).depthScale;
        }
        const auto extent = static_cast<VKSwapchain *>(window->getSwapchain())->getSwapchainExtent();
        shadows->Prepare(*this, float(extent.width) / extent.height);
        const bool needReceivers = voxelGI || ambientOcclusion.active() || screenSpaceReflections;
        if (needReceivers) renderpass->PrepareVoxelScreen(voxelGI, screenSpaceReflections);
        PrepareDescriptors();
        ProcessDraws(nullptr);

        auto *swapchain = static_cast<VKSwapchain *>(window->getSwapchain());
        uint32_t index = swapchain->getAquiredImageIndex();
        VKCommandBuffer *buffer = commandPool->getCommandBuffer(index);
        buffer->BindBuffer();
        if (voxelGI || voxelSunShadows)
            renderpass->Voxels(voxelResolution, true).Prepare(buffer->getCommandBuffer(), *this);
        shadows->Record(*this, buffer);
        if (needReceivers) {
            renderpass->BeginVoxelReceivers(buffer, screenSpaceReflections);
            ProcessDraws(buffer, nullptr, true);
            vkCmdEndRenderPass(buffer->getCommandBuffer());
            if (screenSpaceReflections) renderpass->BoundReflectionDepth(buffer, *this);
            if (voxelGI) renderpass->TraceVoxelScreen(buffer, *this);
        }
        for (auto *water: waters) {
            water->simulation().Record(buffer->getCommandBuffer(), float(Time::getTimeFromStartNanoseconds() * 1.0e-9));
            auto &targets = renderpass->GetWaterTargets(water);
            for (bool reflected: {true, false}) {
                const Matrix4f view = camera->getViewMatrix();
                const Matrix4f projection = camera->getProjectionMatrix();
                const auto pass = MakeWaterCapture(view, UseTemporalEffects() ? temporalHistory.projection : projection,
                                                   camera->getPosition(), water->height(), reflected,
                                                   true);
                const auto target = reflected ? targets.reflectionFrame.get() : targets.refractionFrame.get();
                renderpass->BeginRenderpass(buffer, Vector4f(0, 0, 0, 1), target);
                if (pass.sky) {
                    camera->setViewMatrix(pass.view);
                    camera->setProjectionMatrix(pass.projection);
                    background->Draw(buffer->getCommandBuffer(), camera,
                                     atmosphere,
                                     environmentPreviewLod);
                    camera->setViewMatrix(view);
                    camera->setProjectionMatrix(projection);
                }
                if (pass.visible) {
                    const Matrix4f worldToClip = pass.clippedProjection * pass.view;
                    CaptureConstants constants{{}, {pass.eye.x, pass.eye.y, pass.eye.z, 1.0f}};
                    std::copy_n(worldToClip.m, 16, constants.worldToClip);
                    ProcessDraws(buffer, &constants);
                }
                vkCmdEndRenderPass(buffer->getCommandBuffer());
            }
        }
        if (voxelGI)
            renderpass->Voxels(voxelResolution, true).Mark(buffer->getCommandBuffer(), VKVoxelView::Shading, false);
        renderpass->BeginRenderpass(buffer, swapchain->getClearColour(), nullptr,
                                    UseTemporalEffects() || depthOfField.active() || !getVolumetricSpheres().empty());
        background->Draw(buffer->getCommandBuffer(), camera,
                         atmosphere, environmentPreviewLod);
        ProcessDraws(buffer);
        if (voxelGI)
            renderpass->Voxels(voxelResolution, true).Mark(buffer->getCommandBuffer(), VKVoxelView::Shading, true);
    }

    void VKRenderer::RenderHDR() {
        const bool underwater = __WaterConfig.underwaterPostProcessing && std::any_of(waters.begin(), waters.end(),
                                    [](const Water *water) { return water->isCameraUnderwater(); });
        renderpass->RenderHDR(getDrawCommandBuffer(), *this, bloomEnabled && bloomIntensity > 0.0f, underwater);
    }

    void VKRenderer::FinishFrame() {
        VKCommandBuffer *buffer = getDrawCommandBuffer();
        if (auto *ui = Application::Get().getImGuiLayer()) ui->Draw();
        vkCmdEndRenderPass(buffer->getCommandBuffer());
        buffer->UnbindBuffer();
    }

    void VKRenderer::PrepareDescriptors() {
        frameDescriptors.clear();
        for (const auto *batches: {&models_3d, &models_transparency}) {
            for (const auto &[pipeline, groups]: *batches) {
                auto &frame = frameDescriptors[static_cast<VKShader *>(pipeline->getShader())];
                auto *vertices = static_cast<VKGraphicsPipeline *>(pipeline)->getVertexBuffer();
                if (vertices->getType() == VertexBufferType::MESH) {
                    for (auto *material: static_cast<MeshVertexBuffer *>(vertices)->getModel().getMaterials())
                        if (material) frame.materials.try_emplace(material, frame.materials.size());
                }
                for (const auto &[material, objects]: groups) {
                    frame.materials.try_emplace(material, frame.materials.size());
                    for (auto *object: objects) {
                        auto *component = dynamic_cast<RendererComponent *>(object);
                        if (!component) continue;
                        for (const auto &[index, overrideMaterial]: component->getSubmeshOverrides())
                            frame.materials.try_emplace(overrideMaterial.get(), frame.materials.size());
                    }
                    frame.objects += static_cast<uint32_t>(objects.size());
                }
            }
        }
        for (const auto &[pipeline, objects]: models_2d)
            frameDescriptors[static_cast<VKShader *>(pipeline->getShader())].objects += static_cast<uint32_t>(objects.
                size());
        for (const auto &[shader, frame]: frameDescriptors) {
            shader->EnsureDescriptorCapacity(std::max(frame.objects, static_cast<uint32_t>(frame.materials.size())));
            for (const auto &[material, slot]: frame.materials)
                shader->UpdateMaterialUniforms(material, slot);
        }
    }

    void VKRenderer::ProcessDraws(VKCommandBuffer *buffer, const CaptureConstants *capture, bool receivers) {
        // Each shader owns its descriptor pool. Both traversals assign the same slots.
        std::unordered_map<VKShader *, uint32_t> objectSlots;
        Material *fallback = nullptr;
        MeshVertexBuffer *mesh = nullptr;
        VKShader *shader = nullptr;
        auto begin = [&](Pipeline *pipeline, bool wireframe) {
            shader = static_cast<VKShader *>(pipeline->getShader());
            mesh = nullptr;
            auto *graphics = static_cast<VKGraphicsPipeline *>(pipeline);
            if (receivers && !graphics->HasVoxelReceiver()) {
                shader = nullptr;
                return;
            }
            if (buffer) graphics->BindPipeline(buffer, wireframe, receivers, screenSpaceReflections);
        };
        uint32_t lastSelected = 0;
        auto beginScene = [&](Pipeline *pipeline) {
            begin(pipeline, isWireframeMode());
            if (!shader) return;
            if (buffer) {
                auto *vertices = static_cast<VKGraphicsPipeline *>(pipeline)->getVertexBuffer();
                if (vertices->getType() == VertexBufferType::MESH)
                    mesh = static_cast<MeshVertexBuffer *>(vertices);
                lastSelected = 0;
                constexpr CaptureConstants mainView{};
                vkCmdPushConstants(buffer->getCommandBuffer(), shader->GetPipelineLayout(),
                                   VK_SHADER_STAGE_ALL_GRAPHICS,
                                   0, sizeof(mainView), capture ? capture : &mainView);
                shader->BindGlobalSets();
            } else shader->UpdateGlobalUniforms(this);
        };
        auto material = [&](Pipeline *, Material *material) {
            if (!shader) return;
            fallback = material;
            if (buffer && !mesh) shader->BindTextureSets(frameDescriptors.at(shader).materials.at(material));
        };
        auto object = [&](Pipeline *pipeline, RenderableComponent *renderable) {
            if (!shader) return;
            if (buffer) {
                auto slot = objectSlots[shader]++;
                if (capture && !renderable->hasReflection()) return;
                shader->BindObjectSets(slot);
                if (!mesh) {
                    pipeline->RenderPipeline();
                    return;
                }
                const auto &materials = frameDescriptors.at(shader).materials;
                Material *bound = nullptr;
                for (uint32_t i = 0; i < mesh->getSubmeshCount(); ++i) {
                    const uint32_t selectedPart = !capture && !receivers && isSelected(renderable->getParent(), i);
                    if (selectedPart != lastSelected) {
                        vkCmdPushConstants(buffer->getCommandBuffer(), shader->GetPipelineLayout(),
                                           VK_SHADER_STAGE_ALL_GRAPHICS, offsetof(CaptureConstants, selected),
                                           sizeof(selectedPart), &selectedPart);
                        lastSelected = selectedPart;
                    }
                    auto *selected = renderable->ResolveSubmeshMaterial(mesh->getModel(), i, fallback);
                    if (selected != bound) {
                        shader->BindTextureSets(materials.at(selected));
                        bound = selected;
                    }
                    mesh->Draw(buffer, i);
                }
            } else {
                shader->UpdateObjectUniforms(renderable->getParent(), objectSlots[shader]++);
            }
        };
        auto end = [&](Pipeline *pipeline) {
            if (buffer && shader) pipeline->UnbindPipeline();
        };
        VisitBatches(models_3d, beginScene, material, object, end);
        if (capture || receivers) return;
        VisitBatches(models_transparency, beginScene, material, object, end);
        for (const auto &[pipeline, objects]: models_2d) {
            begin(pipeline, false);
            for (RenderableComponent *renderable: objects) {
                object(pipeline, renderable);
            }
            end(pipeline);
        }
    }
}