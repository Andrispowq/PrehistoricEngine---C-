#include "EditorLayer.h"

#include "prehistoric/application/Application.h"
#include "platform/opengl/rendering/GLRenderer.h"
#include "platform/vulkan/rendering/VKRenderer.h"

#include "imgui.h"
#include "prehistoric/core/ImGUI/ImGUILayer.h"
#include "imgui_internal.h"

#include "prehistoric/common/util/PlatformUtils.h"
#include "prehistoric/core/resources/AssembledAssetManager.h"

#include "Prehistoric/core/scene/world/WorldSerialiser.h"

EditorLayer::EditorLayer(std::unique_ptr<Prehistoric::Scene> scene)
    : scene(std::move(scene)), root(this->scene->getSceneRoot()) {
    this->scenePanel = std::make_unique<SceneHierarchyPanel>(this->scene.get());
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
            ImGuiID properties;
            statsDockId = ImGui::DockBuilderSplitNode(right, ImGuiDir_Down, 0.28f, nullptr, &properties);
            ImGui::DockBuilderDockWindow("Scene Hierarchy", left);
            ImGui::DockBuilderDockWindow("Properties", properties);
            ImGui::DockBuilderDockWindow("Stats", statsDockId);
            ImGui::DockBuilderDockWindow("Asset Manager", bottom);
            ImGui::DockBuilderDockWindow("Viewport", center);
            ImGui::DockBuilderFinish(dockspace_id);
        }
        if (!statsDockId) {
            // Reuse the old environment dock and move Properties there. Other panels keep their docks.
            auto *old = ImGui::FindWindowSettings(ImHashStr("Environment###Environment map configurations"));
            if (!old) old = ImGui::FindWindowSettings(ImHashStr("Environment map configurations"));
            if (old && old->DockId) {
                const ImGuiID target = old->DockId;
                old->DockId = 0;
                if (auto *legacy = ImGui::FindWindowSettings(ImHashStr("Environment map configurations")))
                    legacy->DockId = 0;
                if (ImGui::DockBuilderGetNode(target)) ImGui::DockBuilderDockWindow("Properties", target);
            }
            auto *properties = ImGui::FindWindowSettings(ImHashStr("Properties"));
            auto *stats = ImGui::FindWindowSettings(ImHashStr("Stats"));
            if (stats && ImGui::DockBuilderGetNode(stats->DockId)) statsDockId = stats->DockId;
            else if (properties && ImGui::DockBuilderGetNode(properties->DockId)) {
                ImGuiID top;
                statsDockId = ImGui::DockBuilderSplitNode(properties->DockId, ImGuiDir_Down, 0.28f, nullptr, &top);
                ImGui::DockBuilderDockWindow("Properties", top);
                ImGui::DockBuilderDockWindow("Stats", statsDockId);
            }
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

        if (ImGui::BeginMenu("Objects")) {
            scenePanel->DrawCreateMenu();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Render Settings")) {
            DrawRenderSettings(*renderer);
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    scenePanel->ImGuiRender();

    ImGuiWindowClass statsClass;
    statsClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoDockingOverMe | ImGuiDockNodeFlags_NoDockingSplitMe;
    ImGui::SetNextWindowClass(&statsClass);
    if (statsDockId) ImGui::SetNextWindowDockID(statsDockId, ImGuiCond_Always);
    ImGui::SetNextWindowCollapsed(false, ImGuiCond_Always);
    ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    auto stats = Prehistoric::RenderingEngine::getStats();
    ImGui::Text("RenderingEngine statistics:");
    ImGui::Text("Draw Calls: %zu", stats.drawcalls);
    ImGui::Text("Vertices: %zu", stats.vertexCount);
    ImGui::Text("Indices: %zu", stats.indexCount);

    ImGui::Text("Frames per second: %d", Prehistoric::Application::Get().getLastFPS());
    ImGui::Text("Frame time: %f", Prehistoric::Application::Get().getFrameTime());

    ImGui::End();

    auto *ui = Prehistoric::Application::Get().getImGuiLayer();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::Begin("Viewport");

    ImVec2 size = ImGui::GetContentRegionAvail();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    auto origin = ImGui::GetMainViewport()->Pos;
    viewportStart = {pos.x - origin.x, pos.y - origin.y};
    viewportSize = {size.x, size.y};
    if (size.x > 0 && size.y > 0) {
        auto *camera = renderer->getCamera();
        auto *output = renderer->getOutputTexture();
        if (output) {
            ui->DrawImage(output, size.x, size.y);
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsMouseDown(ImGuiMouseButton_Right) &&
                !ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
                const auto mouse = ImGui::GetMousePos();
                const float x = 2.0f * (mouse.x - pos.x) / size.x - 1.0f;
                float y = 1.0f - 2.0f * (mouse.y - pos.y) / size.y;
                if (__FrameworkConfig.api == Prehistoric::Vulkan) y = -y;
                const auto viewPoint = camera->getProjectionMatrix().Invert() * Prehistoric::Vector4f(x, y, 0.5f, 1.0f);
                const auto direction = camera->getViewMatrix().Invert() * Prehistoric::Vector4f(viewPoint.xyz(), 0.0f);
                const auto origin = camera->getPosition();
                const auto hit = scene->Raycast(origin, direction.xyz(), pickDistance);
                auto *object = hit ? hit->object : nullptr;
                std::optional<uint32_t> part;
                if (object && object->GetComponent<Prehistoric::RendererComponent>()->getMeshBuffer()->getSubmeshCount()
                    > 1)
                    part = hit->submesh;
                scenePanel->SelectViewportHit(object, part);
            }
        }
        camera->SetProjection(camera->getFov(), size.x, size.y);
    }

    ImGui::End();
    ImGui::PopStyleVar();

    auto *selected = scenePanel->getSelectedObject();
    auto *component = selected ? selected->GetComponent<Prehistoric::RendererComponent>() : nullptr;
    renderer->SetSelection(component && component->getMeshBuffer() ? selected : nullptr,
                           scenePanel->getSelectedSubmesh());

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

void EditorLayer::DrawRenderSettings(Prehistoric::Renderer &renderer) {
    if (ImGui::BeginMenu("Antialiasing")) {
        DrawAntialiasingSettings(renderer);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Camera & post-processing")) {
        DrawCameraSettings(renderer);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Lighting & shadows")) {
        DrawLightingSettings(renderer);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Voxel GI")) {
        DrawVoxelSettings(renderer);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Environment baking")) {
        DrawEnvironmentSettings(renderer);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Diagnostics")) {
        DrawDiagnostics(renderer);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Editor interaction")) {
        DrawEditorInteraction();
        ImGui::EndMenu();
    }
}

void EditorLayer::DrawAntialiasingSettings(Prehistoric::Renderer &renderer) {
    const unsigned activeSamples = renderer.GetSampleCount();
    const std::string sampleLabel = activeSamples == 1 ? "Off" : std::to_string(activeSamples) + "x";
    if (ImGui::BeginCombo("MSAA", sampleLabel.c_str())) {
        for (unsigned samples: renderer.GetSupportedSampleCounts()) {
            const std::string label = samples == 1 ? "Off" : std::to_string(samples) + "x";
            if (ImGui::Selectable(label.c_str(), samples == activeSamples)) {
                renderer.RequestSampleCount(samples);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::TextDisabled("Applies next frame; higher MSAA uses more GPU memory.");
    ImGui::Separator();
    if (ImGui::Checkbox("Temporal antialiasing (TAA)", &scene->temporal.antialiasing)) {
        renderer.temporal = scene->temporal;
    }
}

void EditorLayer::DrawCameraSettings(Prehistoric::Renderer &renderer) {
    ImGui::SliderFloat("Exposure", &__EngineConfig.rendererExposure, 0.5f, 4.0f, "%.2f");
    ImGui::SliderFloat("Gamma", &__EngineConfig.rendererGamma, 1.0f, 5.0f, "%.2f");
    ImGui::Separator();
    auto &temporal = scene->temporal;
    ImGui::Checkbox("Motion blur", &temporal.motionBlur);
    if (temporal.motionBlur) {
        ImGui::SliderFloat("Shutter angle", &temporal.shutterAngle, 0, 360, "%.0f degrees");
        ImGui::SliderFloat("Maximum blur", &temporal.maxBlurPixels, 0, 64, "%.0f pixels");
        ImGui::SliderInt("Blur samples", &temporal.samples, 4, 32);
    }
    renderer.temporal = temporal;
    ImGui::Separator();
    auto &lens = scene->depthOfField;
    ImGui::Checkbox("Depth of field", &lens.enabled);
    if (lens.enabled) {
        ImGui::DragFloat("Focus distance", &lens.focusDistance, 0.25f, 0.01f, __EngineConfig.rendererFarPlane,
                         "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::DragFloat("Aperture diameter", &lens.aperture, 0.02f, 0.0f, 100.0f, "%.2f",
                         ImGuiSliderFlags_AlwaysClamp);
        ImGui::TextDisabled("Both distances are in world units.");
    }
    renderer.depthOfField = lens;
    ImGui::Separator();
    bool bloom = renderer.isBloomEnabled();
    if (ImGui::Checkbox("Bloom (K)", &bloom)) renderer.setBloomEnabled(bloom);

    if (bloom && ImGui::SliderFloat("Bloom intensity", &scene->bloomIntensity, 0.0f, 2.0f, "%.2f")) {
        renderer.bloomIntensity = scene->bloomIntensity;
    }
    ImGui::Checkbox("Sun lens flare", &renderer.sunFlare);
    if (renderer.sunFlare) {
        ImGui::SliderFloat("Flare intensity", &renderer.sunFlareStrength, 0.0f, 3.0f);
        ImGui::SliderFloat("Flare size", &renderer.sunFlareSize, 0.25f, 2.0f);
    }
}

void EditorLayer::DrawLightingSettings(Prehistoric::Renderer &renderer) {
    if (ImGui::Checkbox("Screen-space reflections", &scene->screenSpaceReflections)) {
        renderer.screenSpaceReflections = scene->screenSpaceReflections;
    }
    auto &ao = scene->ambientOcclusion;
    ImGui::Checkbox("Ambient occlusion", &ao.enabled);
    if (ao.enabled) {
        ImGui::DragFloat("Occlusion radius", &ao.radius, 0.05f, 0.01f, 100.0f, "%.2f world units",
                         ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Occlusion strength", &ao.strength, 0.0f, 2.0f);
    }
    renderer.ambientOcclusion = ao;
    ImGui::Separator();
    ImGui::SliderFloat("Sun Shadow Radius", &__EngineConfig.shadowSunAngularRadius, 0.0f, 0.1f, "%.3f rad");
    ImGui::Checkbox("Sun shafts", &renderer.sunShafts);
    if (renderer.sunShafts) {
        ImGui::SliderFloat("Shaft strength", &renderer.sunShaftStrength, 0.0f, 4.0f);
    }
}

void EditorLayer::DrawVoxelSettings(Prehistoric::Renderer &renderer) {
    auto *gl = dynamic_cast<Prehistoric::GLRenderer *>(&renderer);
    auto *vk = dynamic_cast<Prehistoric::VKRenderer *>(&renderer);
    auto *pass = gl ? gl->getVoxelPass() : nullptr;
    if (vk || (pass && pass->IsSupported())) {
        using VoxelPass = Prehistoric::GLVoxelRendererPass;
        auto *settings = pass ? &pass->GetSettings() : nullptr;
        auto &gi = vk ? vk->voxelGI : settings->globalIllumination;
        auto &width = vk ? vk->voxelExtent : settings->extent;
        auto &center = vk ? vk->voxelCenter : settings->center;
        ImGui::Checkbox("Voxel GI (G)", &gi);
        if (ImGui::Checkbox("Follow camera (snapped)", &renderer.voxelFollowCamera)) renderer.ResetTemporalHistory();
        if (ImGui::Checkbox("Terrain receives GI", &scene->terrainReceivesGI)) {
            renderer.terrainReceivesGI = scene->terrainReceivesGI;
            renderer.ResetTemporalHistory();
        }
        if (ImGui::Checkbox("Voxelise terrain (T)", &scene->voxeliseTerrain)) {
            renderer.voxeliseTerrain = scene->voxeliseTerrain;
            renderer.ResetTemporalHistory();
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip(
                "Include terrain in voxel shadows and GI. Terrain receives GI independently of this setting.");
        ImGui::Checkbox("Half-Resolution GI (H)", vk ? &vk->voxelHalfResolution : &settings->halfResolution);
        ImGui::Checkbox("Directional Voxel Mips (M)", vk ? &vk->voxelDirectional : &settings->directionalMips);
        ImGui::Checkbox("Voxel Sun Shadows (J)", vk ? &vk->voxelSunShadows : &settings->voxelSunShadows);
        int resolution = vk ? vk->voxelResolution : VoxelPass::Resolutions[settings->resolutionIndex];
        const auto label = [](int size) { return std::to_string(size) + " cubed"; };
        if (ImGui::BeginCombo("Voxel Resolution (B)", label(resolution).c_str())) {
            for (int i = 0; i < static_cast<int>(std::size(VoxelPass::Resolutions)); ++i) {
                const int size = VoxelPass::Resolutions[i];
                const bool available = vk ? vk->SupportsVoxelResolution(size) : pass->CanUseResolution(i);
                if (ImGui::Selectable(label(size).c_str(), size == resolution,
                                      available ? ImGuiSelectableFlags_None : ImGuiSelectableFlags_Disabled)) {
                    resolution = size;
                    if (vk) vk->voxelResolution = size;
                    else settings->resolutionIndex = i;
                }
                if (!available && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                    ImGui::SetTooltip("Grid exceeds the GPU budget for the current voxel settings.");
                }
            }
            ImGui::EndCombo();
        }
        if (ImGui::DragFloat3("Volume Center", &center.x, 1.0f)) renderer.voxelFollowCamera = false;
        ImGui::DragFloat("Volume Width", &width, 1.0f, 1.0f, 100000.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("GI Strength", vk ? &vk->voxelStrength : &settings->giStrength, 0.0f, 4.0f);
        ImGui::SliderInt("GI Trace Depth ([ / ])", vk ? &vk->voxelBounces : &settings->traceDepth, 1,
                         VoxelPass::MaxTraceDepth, "%d", ImGuiSliderFlags_AlwaysClamp);
        ImGui::Text("Voxel size %.2f", width / resolution);
        Prehistoric::ImGuiLayer::DrawVoxelCascades(renderer, unsigned(resolution));
    } else {
        ImGui::TextDisabled("Voxel GI is unavailable on this device.");
    }
}

void EditorLayer::DrawEnvironmentSettings(Prehistoric::Renderer &renderer) {
    auto editSize = [](const char *label, uint32_t &value) {
        int size = static_cast<int>(value);
        if (ImGui::InputInt(label, &size)) value = static_cast<uint32_t>(std::max(1, size));
    };
    editSize("Environment map resolution", __EnvironmentMapConfig.environmentMapResolution);
    editSize("Irradiance map resolution", __EnvironmentMapConfig.irradianceMapResolution);
    editSize("Prefilter map resolution", __EnvironmentMapConfig.prefilterMapResolution);
    editSize("Prefilter map mip levels", __EnvironmentMapConfig.prefilterLevels);
    const float maxLod = float(std::max(1u, __EnvironmentMapConfig.prefilterLevels) - 1);
    ImGui::SliderFloat("Environment map LOD", &renderer.environmentPreviewLod, 0.0f, maxLod, "%.1f");
    ImGui::Separator();
    if (ImGui::Button("Regenerate lighting")) renderer.RequestEnvironmentBake();
    ImGui::TextDisabled("Rebakes the world's atmosphere or environment map.");
}

void EditorLayer::DrawDiagnostics(Prehistoric::Renderer &renderer) {
    bool wireframe = renderer.isWireframeMode();
    if (ImGui::Checkbox("Wireframe (E)", &wireframe)) renderer.setWireframeMode(wireframe);
    auto &temporal = scene->temporal;
    if (ImGui::Combo("Temporal debug", &temporal.debug, "Output\0Velocity\0Validity\0Rejection\0History weight\0")) {
        renderer.temporal = temporal;
    }
    if (ImGui::Button("Reset temporal history")) renderer.ResetTemporalHistory();
    if (ImGui::TreeNode("Temporal GPU timings")) {
        const char *stages[] = {"Motion geometry", "Motion resolve", "Temporal AA", "Blur and diagnostics"};
        for (int i = 0; i < 4; ++i) {
            if (renderer.temporalTimings.milliseconds[i] >= 0) {
                ImGui::Text("%s: %.3f ms", stages[i], renderer.temporalTimings.milliseconds[i]);
            }
        }
        ImGui::Text("Temporal targets: %.1f MiB", renderer.temporalBytes / (1024.0 * 1024.0));
        ImGui::TreePop();
    }
    auto *gl = dynamic_cast<Prehistoric::GLRenderer *>(&renderer);
    auto *vk = dynamic_cast<Prehistoric::VKRenderer *>(&renderer);
    auto *pass = gl ? gl->getVoxelPass() : nullptr;
    if (vk || (pass && pass->IsSupported())) {
        using VoxelPass = Prehistoric::GLVoxelRendererPass;
        auto *settings = pass ? &pass->GetSettings() : nullptr;
        const bool gi = vk ? vk->voxelGI : settings->globalIllumination;
        ImGui::Separator();
        ImGui::Checkbox("Voxel View (V)", vk ? &vk->voxelView : &settings->debugView);
        if (gi && ImGui::TreeNode("Voxel GPU Timings")) {
            const char *labels[] = {
                vk ? "Geometry bake" : "Occluder bake",
                vk ? "Occlusion and injection" : "Radiance injection", "Bounce propagation",
                "Final radiance mips", "Cone tracing",
                vk ? "Main shading and GI composite" : "Filter and composite"
            };
            for (unsigned i = 0; i < std::size(labels); ++i) {
                const double ms = vk
                                      ? vk->voxelMilliseconds[i]
                                      : pass->GetGpuMilliseconds(static_cast<VoxelPass::GpuStage>(i));
                if (ms >= 0) ImGui::Text("%s  %.2f ms", labels[i], ms);
            }
            ImGui::TreePop();
        }
    }
}

void EditorLayer::DrawEditorInteraction() {
    if (ImGui::InputFloat("Pick distance", &pickDistance, 1.0f, 10.0f, "%.1f")) {
        pickDistance = std::max(0.0f, pickDistance);
    }
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
    scene->ClearObjects();
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

        std::unique_ptr<Prehistoric::Scene> next;
        try { next = std::make_unique<Prehistoric::Scene>(val, window, camera, manager); } catch (const std::exception &
            error) {
            PR_LOG_WARNING("Could not open world: %s\n", error.what());
            return;
        }
        coreEngine->SetScene(next.get());
        scene = std::move(next);

        root = scene->getSceneRoot();
        scenePanel->SetContext(scene.get());
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