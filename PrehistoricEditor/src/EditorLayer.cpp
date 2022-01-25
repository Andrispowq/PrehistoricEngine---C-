#include "EditorLayer.h"

#include "prehistoric/application/Application.h"

#include "imgui.h"

#include "prehistoric/common/util/PlatformUtils.h"

EditorLayer::EditorLayer()
	: scenePanel{nullptr}
{
	scenePanel = std::make_unique<SceneHierarchyPanel>(Prehistoric::Application::Get().getEngineLayer()->getRootObject());
}

void EditorLayer::OnAttach()
{
}

void EditorLayer::OnDetach()
{
}

void EditorLayer::Update(float delta)
{
}

#include "SpotifyInterface.h"
extern SpotifyInterface* sIF;
char songNameBuffer[256] = { 0 };
int offset = 0;

void EditorLayer::ImGUIRender()
{
	static bool dockspaceOpen = true;
	static bool opt_fullscreen_persistant = true;
	bool opt_fullscreen = opt_fullscreen_persistant;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
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
	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();
	float minWinSizeX = style.WindowMinSize.x;
	style.WindowMinSize.x = 370.0f;
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	style.WindowMinSize.x = minWinSizeX;

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			// Disabling fullscreen would allow the window to be moved to the front of other windows, 
			// which we can't undo at the moment without finer window depth/z control.
			//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);1
			if (ImGui::MenuItem("New", "Ctrl+N"))
				PR_LOG_MESSAGE("Clicked New!\n");

			if (ImGui::MenuItem("Open...", "Ctrl+O"))
				PR_LOG_MESSAGE("Clicked Open!\n");

			if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
				PR_LOG_MESSAGE("Clicked Save as!\n");

			if (ImGui::MenuItem("Exit")) Prehistoric::Application::Get().Stop();

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	scenePanel->ImGuiRender();

	ImGui::Begin("Stats");

	auto stats = Prehistoric::RenderingEngine::getStats();
	ImGui::Text("RenderingEngine statistics:");
	ImGui::Text("Draw Calls: %d", stats.drawcalls);
	ImGui::Text("Vertices: %d", stats.vertexCount);
	ImGui::Text("Indices: %d", stats.indexCount);

	ImGui::Text("Frames per second: %d", Prehistoric::Application::Get().getLastFPS());
	ImGui::Text("Frame time: %f", Prehistoric::Application::Get().getFrameTime());

	ImGui::End();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
	ImGui::Begin("Environment map configurations");

	if (ImGui::Button("Load environment map"))
	{
		std::optional<std::string> path = Prehistoric::FileDialogues::Get()->OpenFile("Environment Map (*.hdr)\0*.hdr\0");
		if (path)
		{
			Prehistoric::EnvironmentMapConfig::environmentMapLocation = *path;
			Prehistoric::EnvironmentMapRenderer::instance->GenerateEnvironmentMap();
		}
	}

	if (ImGui::Button("Regenerate environment map"))
	{
		Prehistoric::EnvironmentMapRenderer::instance->GenerateEnvironmentMap();
	}

	float max = (float)Prehistoric::EnvironmentMapConfig::prefilterLevels;
	ImGui::SliderFloat("Environment map LOD", &Prehistoric::EnvironmentMapRenderer::instance->lodRenderedMap, 0.0f, max - 1.0f, "Environment map LOD");

	ImGui::InputInt("Environment map resolution", (int*)&Prehistoric::EnvironmentMapConfig::environmentMapResolution);
	ImGui::InputInt("Irradiance map resolution", (int*)&Prehistoric::EnvironmentMapConfig::irradianceMapResolution);
	ImGui::InputInt("Prefilter map resolution", (int*)&Prehistoric::EnvironmentMapConfig::prefilterMapResolution);
	ImGui::InputInt("Prefilter map mip levels", (int*)&Prehistoric::EnvironmentMapConfig::prefilterLevels);

	ImGui::SliderFloat("Exposure", &Prehistoric::EngineConfig::rendererExposure, 0.5f, 4.0f, "Exposure");
	ImGui::SliderFloat("Gamma", &Prehistoric::EngineConfig::rendererGamma, 1.0f, 5.0f, "Gamma");

	ImGui::InputText("Song to play", songNameBuffer, 256);
	ImGui::InputInt("Start offset (ms)", &offset);
	if (ImGui::Button("Play song"))
	{
		std::string name = songNameBuffer;
		if (name.empty())
		{
			return;
		}

		sIF->PlayTrack(name, float(offset) / 1000.0f);

		//memset(songNameBuffer, 0, 256);
		//offset = 0;
	}

	ImGui::End();
	ImGui::PopStyleVar();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
	ImGui::Begin("Viewport");

	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	Prehistoric::Vector2f viewportSize = { viewportPanelSize.x, viewportPanelSize.y };

	Prehistoric::Texture* texture = Prehistoric::Application::Get().getEngineLayer()->getRenderingEngine()->getRenderer()->getOutputTexture();
	ImGui::Image((void*)(dynamic_cast<Prehistoric::GLTexture*>(texture)->getTextureID()), ImVec2{ viewportSize.x, viewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

	ImGui::End();
	ImGui::PopStyleVar();

	ImGui::End();
}

void EditorLayer::OnEvent(Prehistoric::Event& e)
{
}
