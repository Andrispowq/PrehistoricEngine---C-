#include "Includes.hpp"
#include "prehistoric/core/resources/EngineResources.h"
#include "ImGUILayer.h"

#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"

#include "prehistoric/application/Application.h"

#include "platform/vulkan/framework/context/VKContext.h"
#include "platform/vulkan/rendering/VKImGui.h"
#include "platform/opengl/texture/GLTexture.h"

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