#include "EditorLayer.h"

#include "prehistoric/application/Application.h"
#include "platform/opengl/rendering/GLRenderer.h"

#include "imgui.h"
#include "imgui_internal.h"

#include "prehistoric/common/util/PlatformUtils.h"
#include "prehistoric/core/resources/AssembledAssetManager.h"

#include "Prehistoric/core/scene/world/WorldSerialiser.h"

EditorLayer::EditorLayer(std::unique_ptr<Prehistoric::Scene> scene)
    : scene(std::move(scene)), root(this->scene->getSceneRoot()) {
    this->scenePanel = std::make_unique<SceneHierarchyPanel>(root);
}

void EditorLayer::OnAttach() {
}

void EditorLayer::OnDetach() {
}

void EditorLayer::Update(float delta) {
}

void EditorLayer::ImGUIRender() {
    static bool dockspaceOpen = true;
    static bool opt_fullscreen_persistant = true;
    bool opt_fullscreen = opt_fullscreen_persistant;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen) {
        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
    ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // DockSpace
    ImGuiIO &io = ImGui::GetIO();
    ImGuiStyle &style = ImGui::GetStyle();
    float minWinSizeX = style.WindowMinSize.x;
    style.WindowMinSize.x = 370.0f;
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        if (!ImGui::DockBuilderGetNode(dockspace_id)) {
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);
            ImGuiID center = dockspace_id;
            ImGuiID left = ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.22f, nullptr, &center);
            ImGuiID right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.29f, nullptr, &center);
            ImGuiID bottom = ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.25f, nullptr, &center);
            ImGui::DockBuilderDockWindow("Scene Hierarchy", left);
            ImGui::DockBuilderDockWindow("Properties", right);
            ImGui::DockBuilderDockWindow("Environment map configurations", right);
            ImGui::DockBuilderDockWindow("Stats", bottom);
            ImGui::DockBuilderDockWindow("Asset Manager", bottom);
            ImGui::DockBuilderDockWindow("Viewport", center);
            ImGui::DockBuilderFinish(dockspace_id);
        }
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    style.WindowMinSize.x = minWinSizeX;

    auto *renderer = Prehistoric::Application::Get().getEngineLayer()->getRenderingEngine()->getRenderer();
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            // Disabling fullscreen would allow the window to be moved to the front of other windows, 
            // which we can't undo at the moment without finer window depth/z control.
            //ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);1
            if (ImGui::MenuItem("New", "Ctrl+N")) NewButton();
            if (ImGui::MenuItem("Open...", "Ctrl+O")) OpenButton();
            if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) SaveButton();
            if (ImGui::MenuItem("Exit")) QuitButton();

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Render Settings")) {
            bool wireframe = renderer->isWireframeMode();
            if (ImGui::Checkbox("Wireframe (E)", &wireframe)) renderer->setWireframeMode(wireframe);
            bool bloom = renderer->isBloomEnabled();
            if (ImGui::Checkbox("Bloom (K)", &bloom)) renderer->setBloomEnabled(bloom);

            auto *gl = dynamic_cast<Prehistoric::GLRenderer *>(renderer);
            if (gl && gl->getVoxelPass()->IsSupported()) {
                auto &settings = gl->getVoxelPass()->GetSettings();
                ImGui::Separator();
                ImGui::Checkbox("Voxel View (V)", &settings.debugView);
                ImGui::Checkbox("Voxel GI (G)", &settings.globalIllumination);
                ImGui::Checkbox("Half-Resolution GI", &settings.halfResolution);
                ImGui::Checkbox("Directional Voxel Mips", &settings.directionalMips);
                ImGui::Checkbox("Voxel Sun Shadows", &settings.voxelSunShadows);
                const auto &resolutions = Prehistoric::GLVoxelRendererPass::Resolutions;
                const auto resolutionLabel = [](int resolution) { return std::to_string(resolution) + " cubed"; };
                if (ImGui::BeginCombo("Voxel Resolution (B)",
                                      resolutionLabel(resolutions[settings.resolutionIndex]).c_str())) {
                    for (int i = 0; i < static_cast<int>(std::size(resolutions)); ++i) {
                        const bool available = gl->getVoxelPass()->CanUseResolution(i);
                        if (ImGui::Selectable(resolutionLabel(resolutions[i]).c_str(), i == settings.resolutionIndex,
                                              available ? ImGuiSelectableFlags_None : ImGuiSelectableFlags_Disabled))
                            settings.resolutionIndex = i;
                        if (!available && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                            ImGui::SetTooltip("Grid exceeds the GPU budget for the current voxel settings.");
                    }
                    ImGui::EndCombo();
                }
                ImGui::DragFloat3("Volume Center", &settings.center.x, 1.0f);
                ImGui::DragFloat("Volume Width", &settings.extent, 1.0f, 1.0f, 100000.0f, "%.1f",
                                 ImGuiSliderFlags_AlwaysClamp);
                ImGui::SliderFloat("GI Strength", &settings.giStrength, 0.0f, 4.0f);
                ImGui::SliderInt("GI Trace Depth", &settings.traceDepth, 1,
                                 Prehistoric::GLVoxelRendererPass::MaxTraceDepth, "%d", ImGuiSliderFlags_AlwaysClamp);
                ImGui::Text("Voxel size %.2f",
                            settings.extent / Prehistoric::GLVoxelRendererPass::Resolutions[settings.resolutionIndex]);
                if (settings.globalIllumination && ImGui::TreeNode("Voxel GPU Timings")) {
                    using Stage = Prehistoric::GLVoxelRendererPass::GpuStage;
                    const char *labels[] = {
                        "Occluder bake", "Radiance injection", "Bounce propagation",
                        "Final radiance mips", "Cone tracing", "Filter and composite"
                    };
                    for (int i = 0; i < static_cast<int>(Stage::Count); ++i) {
                        const double ms = gl->getVoxelPass()->GetGpuMilliseconds(static_cast<Stage>(i));
                        if (ms >= 0.0) ImGui::Text("%s  %.2f ms", labels[i], ms);
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    scenePanel->ImGuiRender();

    ImGui::Begin("Stats");

    auto stats = Prehistoric::RenderingEngine::getStats();
    ImGui::Text("RenderingEngine statistics:");
    ImGui::Text("Draw Calls: %zu", stats.drawcalls);
    ImGui::Text("Vertices: %zu", stats.vertexCount);
    ImGui::Text("Indices: %zu", stats.indexCount);

    ImGui::Text("Frames per second: %d", Prehistoric::Application::Get().getLastFPS());
    ImGui::Text("Frame time: %f", Prehistoric::Application::Get().getFrameTime());

    ImGui::End();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::Begin("Environment map configurations");

    auto *ui = Prehistoric::Application::Get().getImGuiLayer();
    if (ImGui::Button("Load environment map")) {
        std::optional<std::string> path = Prehistoric::FileDialogues::Get()->OpenFile(
            "Environment Map (*.hdr)\0*.hdr\0");
        if (path) {
            __EnvironmentMapConfig.environmentMapLocation = *path;
            renderer->RequestEnvironmentBake(true);
        }
    }

    if (ImGui::Button("Regenerate environment map")) {
        renderer->RequestEnvironmentBake(false);
    }

    float max = (float) __EnvironmentMapConfig.prefilterLevels;
    ImGui::SliderFloat("Environment map LOD", &renderer->environmentPreviewLod, 0.0f, max - 1.0f, "%.1f");

    auto editSize = [](const char *label, uint32_t &value) {
        int size = static_cast<int>(value);
        if (ImGui::InputInt(label, &size)) value = static_cast<uint32_t>(std::max(1, size));
    };
    editSize("Environment map resolution", __EnvironmentMapConfig.environmentMapResolution);
    editSize("Irradiance map resolution", __EnvironmentMapConfig.irradianceMapResolution);
    editSize("Prefilter map resolution", __EnvironmentMapConfig.prefilterMapResolution);
    editSize("Prefilter map mip levels", __EnvironmentMapConfig.prefilterLevels);

    ImGui::SliderFloat("Exposure", &__EngineConfig.rendererExposure, 0.5f, 4.0f, "%.2f");
    ImGui::SliderFloat("Sun Shadow Radius", &__EngineConfig.shadowSunAngularRadius, 0.0f, 0.1f, "%.3f rad");
    ImGui::SliderFloat("Gamma", &__EngineConfig.rendererGamma, 1.0f, 5.0f, "%.2f");

    ImGui::End();
    ImGui::PopStyleVar();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::Begin("Viewport");

    ImVec2 size = ImGui::GetContentRegionAvail();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    auto origin = ImGui::GetMainViewport()->Pos;
    viewportStart = {pos.x - origin.x, pos.y - origin.y};
    viewportSize = {size.x, size.y};
    if (size.x > 0 && size.y > 0) {
        auto *camera = renderer->getCamera();
        camera->SetProjection(camera->getFov(), size.x, size.y);
        ui->DrawImage(renderer->getOutputTexture(), size.x, size.y);
    }

    ImGui::End();
    ImGui::PopStyleVar();

    //Asset manager
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::Begin("Asset Manager");

    {
        using namespace ::Prehistoric;
        AssembledAssetManager *manager = Prehistoric::Application::Get().getEngineLayer()->getAssetManager();
        AssetManager *man = manager->getAssetManager();

        std::unordered_map<std::string, Resource> ID_map = man->getIDMap();

        float rows = 3;
        float coloumns = 6;
        int index = 0;

        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        Vector2f viewportSize = {viewportPanelSize.x, viewportPanelSize.y};
        Vector2f imageSize = viewportSize / Vector2f(coloumns, rows);

        int flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders;
        if (ImGui::BeginTable("Asset Manager", coloumns, flags)) {
            for (size_t i = 0; i < coloumns; i++) {
                ImGui::TableSetupColumn("");
            }
            ImGui::TableHeadersRow();

            for (auto id: ID_map) {
                if (id.second.type == ResourceType::Texture) {
                    TextureHandle tex = man->loadTexture(id.first).value();
                    ImGui::TableNextColumn();
                    ui->DrawImage(tex.pointer, imageSize.x, imageSize.y);
                }
            }

            ImGui::EndTable();
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();

    ImGui::End();
}

void EditorLayer::OnEvent(Prehistoric::Event &e) {
    if (e.getCategoryFlags() & (Prehistoric::EventCategoryKeyboard | Prehistoric::EventCategoryJoystick |
                                Prehistoric::EventCategoryMouseButton)) {
        Prehistoric::Vector2f start = viewportStart;
        Prehistoric::Vector2f end = viewportStart + viewportSize;
        Prehistoric::Vector2f cursor = InputInstance.getCursorPosition();

        if (!(cursor >= start && cursor <= end) && (InputInstance.getKeysHolding().size() == 0)) {
            e.handled = true;
        }
    }
}

void EditorLayer::NewButton() {
    std::unordered_map<Prehistoric::GUID, Prehistoric::Node *> children = root->getChildrenByID();
    for (auto &elem: children) {
        root->RemoveChild(elem.second);
    }

    scenePanel->InvalidateSelectionContext();
}

void EditorLayer::OpenButton() {
    std::optional<std::string> scene_string = Prehistoric::FileDialogues::Get()->OpenFile(
        "World file(*.json)\0*.json\0");
    if (scene_string.has_value()) {
        std::string val = scene_string.value();

        Prehistoric::CoreEngine *coreEngine = Prehistoric::Application::Get().getEngineLayer();
        Prehistoric::RenderingEngine *renderingEngine = coreEngine->getRenderingEngine();

        Prehistoric::Window *window = renderingEngine->getWindow();
        Prehistoric::Camera *camera = renderingEngine->getCamera();
        Prehistoric::AssembledAssetManager *manager = coreEngine->getAssetManager();

        scene = std::make_unique<Prehistoric::Scene>(val, window, camera, manager);

        coreEngine->SetScene(scene.get());

        root = scene->getSceneRoot();
        scenePanel->SetContext(root);
        scenePanel->InvalidateSelectionContext();
    }
}

void EditorLayer::SaveButton() {
    Prehistoric::CoreEngine *coreEngine = Prehistoric::Application::Get().getEngineLayer();
    Prehistoric::RenderingEngine *renderingEngine = coreEngine->getRenderingEngine();

    Prehistoric::Window *window = renderingEngine->getWindow();
    Prehistoric::Camera *camera = renderingEngine->getCamera();
    Prehistoric::AssembledAssetManager *manager = coreEngine->getAssetManager();

    std::optional<std::string> new_file_sel = Prehistoric::FileDialogues::Get()->SaveFile(
        "World file(*.json)\0*.json\0");
    if (!new_file_sel) return;

    Prehistoric::WorldSerialiser serialiser(window, manager);
    serialiser.SerialiseWorldJSON(new_file_sel.value(), scene.get());
}

void EditorLayer::QuitButton() {
    Prehistoric::Application::Get().Stop();
}