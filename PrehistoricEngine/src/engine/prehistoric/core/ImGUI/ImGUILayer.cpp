#include "Includes.hpp"
#include "prehistoric/core/resources/EngineResources.h"
#include "ImGUILayer.h"
#include "prehistoric/core/scene/Scene.h"
#include "platform/vulkan/rendering/VKRenderer.h"

#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"

#include "prehistoric/application/Application.h"

#include "platform/vulkan/framework/context/VKContext.h"
#include "platform/vulkan/rendering/VKImGui.h"
#include "platform/opengl/texture/GLTexture.h"
#include "platform/opengl/rendering/GLRenderer.h"

namespace Prehistoric {
    ImGuiLayer::ImGuiLayer() {
    }

    ImGuiLayer::~ImGuiLayer() = default;

    void ImGuiLayer::OnAttach() {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void) io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

        io.Fonts->AddFontFromFileTTF(EngineResourcePath("fonts/opensans/OpenSans-Bold.ttf").c_str(), 18.0f);
        io.FontDefault = io.Fonts->AddFontFromFileTTF(EngineResourcePath("fonts/opensans/OpenSans-Regular.ttf").c_str(),
                                                      18.0f);

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle &style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        SetDarkThemeColors();

        Application &app = Application::Get();
        Window *window = app.getEngineLayer()->getRenderingEngine()->getWindow();
        GLFWwindow *glfw_window = static_cast<GLFWwindow *>(window->getWindowHandle());

        // Setup Platform/Renderer bindings
        if (__FrameworkConfig.api == OpenGL) {
            ImGui_ImplGlfw_InitForOpenGL(glfw_window, true);
            ImGui_ImplOpenGL3_Init("#version 410");
        } else {
            ImGui_ImplGlfw_InitForVulkan(glfw_window, true);
            vulkan = std::make_unique<VKImGui>(static_cast<VKContext *>(window->getContext()),
                                               static_cast<VKSwapchain *>(window->getSwapchain()));
        }
    }

    void ImGuiLayer::OnDetach() {
        if (vulkan) vulkan.reset();
        else ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::Begin() {
        if (vulkan) vulkan->Begin();
        else ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        auto *core = Application::Get().getEngineLayer();
        auto *renderer = core->getRenderingEngine()->getRenderer();
        auto *vk = dynamic_cast<VKRenderer *>(renderer);
        auto *gl = dynamic_cast<GLRenderer *>(renderer);
        auto *pass = gl ? gl->getVoxelPass() : nullptr;
        const bool supported = vk || (pass && pass->IsSupported());
        const auto &io = ImGui::GetIO();
        const bool shortcuts = !io.WantTextInput && !io.KeyCtrl && !io.KeyAlt && !io.KeySuper;
        if (shortcuts && ImGui::IsKeyPressed(ImGuiKey_F8, false)) showRenderingControls = !showRenderingControls;
        if (!supported) return;

        auto *settings = pass ? &pass->GetSettings() : nullptr;
        auto &gi = vk ? vk->voxelGI : settings->globalIllumination;
        auto &view = vk ? vk->voxelView : settings->debugView;
        auto &half = vk ? vk->voxelHalfResolution : settings->halfResolution;
        auto &directional = vk ? vk->voxelDirectional : settings->directionalMips;
        auto &shadows = vk ? vk->voxelSunShadows : settings->voxelSunShadows;
        auto &bounces = vk ? vk->voxelBounces : settings->traceDepth;
        bool changed = false;
        auto toggle = [&](ImGuiKey key, bool &value) {
            if (shortcuts && ImGui::IsKeyPressed(key, false)) {
                value = !value;
                changed = true;
            }
        };
        toggle(ImGuiKey_G, gi);
        toggle(ImGuiKey_V, view);
        toggle(ImGuiKey_T, renderer->voxeliseTerrain);
        toggle(ImGuiKey_H, half);
        toggle(ImGuiKey_M, directional);
        toggle(ImGuiKey_J, shadows);
        if (shortcuts && ImGui::IsKeyPressed(ImGuiKey_C, false)) {
            renderer->voxelFollowCamera = false;
            renderer->FrameVoxelVolume();
            changed = true;
        }
        if (shortcuts && ImGui::IsKeyPressed(ImGuiKey_V, false) && view)
            showRenderingControls = true;
        if (shortcuts && ImGui::IsKeyPressed(ImGuiKey_B, false)) {
            if (vk)
                vk->voxelResolution = vk->SupportsVoxelResolution(vk->voxelResolution * 2)
                                          ? vk->voxelResolution * 2
                                          : 64;
            else {
                const int count = static_cast<int>(std::size(GLVoxelRendererPass::Resolutions));
                for (int step = 1; step <= count; ++step) {
                    const int index = (settings->resolutionIndex + step) % count;
                    if (!pass->CanUseResolution(index)) continue;
                    settings->resolutionIndex = index;
                    break;
                }
            }
            changed = true;
        }
        if (shortcuts) {
            int delta = int(ImGui::IsKeyPressed(ImGuiKey_RightBracket, false)) -
                        int(ImGui::IsKeyPressed(ImGuiKey_LeftBracket, false));
            const int next = std::clamp(bounces + delta, 1, GLVoxelRendererPass::MaxTraceDepth);
            changed |= next != bounces;
            bounces = next;
        }
        const int resolution = vk ? vk->voxelResolution : GLVoxelRendererPass::Resolutions[settings->resolutionIndex];
        if (changed) {
            if (auto *scene = core->getScene()) scene->voxeliseTerrain = renderer->voxeliseTerrain;
            renderer->ResetTemporalHistory();
            PR_LOG_MESSAGE("Rendering: GI=%d terrain=%d view=%d grid=%d half=%d directional=%d shadows=%d depth=%d\n",
                           gi, renderer->voxeliseTerrain, view, resolution, half, directional, shadows, bounces);
        }
        if (showRenderingControls) {
            ImGui::SetNextWindowBgAlpha(0.9f);
            if (ImGui::Begin("Rendering comparison (F8)", &showRenderingControls, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("%d FPS | %.2f ms/frame", Application::Get().getLastFPS(),
                            Application::Get().getFrameTime());
                ImGui::Text("G GI: %s | T Terrain voxels: %s", gi ? "on" : "off",
                            renderer->voxeliseTerrain ? "on" : "off");
                ImGui::Text("V Voxel view: %s | B Grid: %d cubed", view ? "on" : "off", resolution);
                ImGui::Text("H Half resolution: %s | M Directional mips: %s", half ? "on" : "off",
                            directional ? "on" : "off");
                ImGui::Text("J Voxel shadows: %s | [ / ] Trace depth: %d", shadows ? "on" : "off", bounces);
                ImGui::TextUnformatted("E Wireframe | K Bloom");
                if (ImGui::Checkbox("Follow camera (snapped)", &renderer->voxelFollowCamera)) renderer->
                        ResetTemporalHistory();
                if (ImGui::Checkbox("Terrain receives GI", &renderer->terrainReceivesGI)) {
                    if (auto *scene = core->getScene()) scene->terrainReceivesGI = renderer->terrainReceivesGI;
                    renderer->ResetTemporalHistory();
                }
                const char *modes[] = {"Reused", "Scrolled", "Full rebuild"};
                ImGui::Text("Geometry: %s | Revoxelised cells: %llu", modes[renderer->voxelUpdate.mode],
                            static_cast<unsigned long long>(renderer->voxelUpdate.dirtyCells));
                ImGui::TextUnformatted("Coverage changes refresh lighting across the volume.");
                auto bounds = renderer->GetVoxelVolume();
                ImGui::TextUnformatted("C Place voxel volume around the current view");
                bool boundsChanged = ImGui::DragFloat3("Volume centre", &bounds.x, 1.0f);
                if (boundsChanged) renderer->voxelFollowCamera = false;
                boundsChanged |= ImGui::DragFloat("Volume width", &bounds.w, 1.0f, 1.0f, 100000.0f,
                                                  "%.1f", ImGuiSliderFlags_AlwaysClamp);
                if (boundsChanged) renderer->SetVoxelVolume({bounds.x, bounds.y, bounds.z}, bounds.w);
                ImGui::Text("Voxel size: %.3f world units", bounds.w / resolution);
                if (!renderer->IsCameraInsideVoxelVolume())
                    ImGui::TextWrapped(
                        "Camera outside voxel volume: nearby surfaces may receive no GI. Press C to reposition it.");
                if (!renderer->voxeliseTerrain)
                    ImGui::TextWrapped(
                        "Terrain is excluded from voxel coverage. Press T to include it. Water is always excluded.");
                if (view) {
                    ImGui::TextWrapped(
                        "V shows occupied cells only. Empty space has a dark background; press V again for shaded GI comparisons.");
                    ImGui::TextWrapped(
                        "Distant terrain can be outside the volume even when the camera is inside. Increase width or reposition the centre to cover it.");
                }
                ImGui::Separator();
                ImGui::TextUnformatted("Latest available GPU measurements (ms)");
                const char *labels[] = {
                    "Geometry / occluders", "Radiance injection", "Bounces", "Radiance mips", "Cone tracing",
                    "Shading / composite"
                };
                for (unsigned i = 0; i < std::size(labels); ++i) {
                    const double ms = vk
                                          ? vk->voxelMilliseconds[i]
                                          : pass->GetGpuMilliseconds(static_cast<GLVoxelRendererPass::GpuStage>(i));
                    if (ms >= 0) ImGui::Text("%s: %.3f", labels[i], ms);
                }
            }
            ImGui::End();
        }
    }

    void ImGuiLayer::End() {
        ImGui::Render();
    }

    void ImGuiLayer::DrawImage(Texture *texture, float width, float height) {
        if (!texture || texture->getType() != TEXTURE_2D || width <= 0 || height <= 0) return;
        ImTextureID id;
        if (vulkan) id = (ImTextureID) vulkan->BindTexture(static_cast<VKTexture *>(texture));
        else id = (ImTextureID)(uintptr_t)
        static_cast<GLTexture *>(texture)->getTextureID();
        const bool flip = !vulkan;
        ImGui::Image(id, ImVec2(width, height), ImVec2(0, flip ? 1 : 0), ImVec2(1, flip ? 0 : 1));
    }

    void ImGuiLayer::Draw() {
        if (vulkan) {
            auto *renderer = Application::Get().getEngineLayer()->getRenderingEngine()->getRenderer();
            auto *buffer = static_cast<VKCommandBuffer *>(renderer->getDrawCommandBuffer());
            vulkan->Draw(buffer->getCommandBuffer());
        } else ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void ImGuiLayer::RenderPlatformWindows() {
        if (!(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)) return;
        GLFWwindow *previous = vulkan ? nullptr : glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        if (vulkan) vulkan->RenderPlatformWindows();
        else {
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(previous);
        }
    }

    void ImGuiLayer::SetDarkThemeColors() {
        auto &colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_WindowBg] = ImVec4{0.1f, 0.105f, 0.11f, 1.0f};

        // Headers
        colors[ImGuiCol_Header] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
        colors[ImGuiCol_HeaderHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
        colors[ImGuiCol_HeaderActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

        // Buttons
        colors[ImGuiCol_Button] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
        colors[ImGuiCol_ButtonHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
        colors[ImGuiCol_ButtonActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

        // Frame BG
        colors[ImGuiCol_FrameBg] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
        colors[ImGuiCol_FrameBgHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
        colors[ImGuiCol_FrameBgActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
        colors[ImGuiCol_TabHovered] = ImVec4{0.38f, 0.3805f, 0.381f, 1.0f};
        colors[ImGuiCol_TabActive] = ImVec4{0.28f, 0.2805f, 0.281f, 1.0f};
        colors[ImGuiCol_TabUnfocused] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};

        // Title
        colors[ImGuiCol_TitleBg] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
        colors[ImGuiCol_TitleBgActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
    }
};