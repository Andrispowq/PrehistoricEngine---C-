#ifndef APPLICATION_H
#define APPLICATION_H

#include "prehistoric/core/events/Event.h"

#include "prehistoric/core/layers/LayerStack.h"

#include "prehistoric/core/CoreEngine.h"
#include "prehistoric/core/ImGUI/ImGUILayer.h"

#define __EngineSettings ::Prehistoric::Application::Get().getEngineLayer()->getEngineSettings()

#define __FrameworkConfig __EngineSettings->frameworkConfig
#define __EngineConfig __EngineSettings->engineConfig
#define __TerrainConfig __EngineSettings->terrainConfig
#define __AtmosphereConfig __EngineSettings->atmosphereConfig
#define __EnvironmentMapConfig __EngineSettings->environmentMapConfig
#define __WaterConfig __EngineSettings->waterConfig

namespace Prehistoric {
    class Application {
    public:
        explicit Application(bool editor = false);

        virtual ~Application();

        void Run();

        void Stop();

        void OnEvent(Event &e);

        void PushLayer(Layer *layer);

        void PushOverlay(Layer *layer);

        bool isEditor() const { return editorMode; }
        CoreEngine *getEngineLayer() { return engineLayer; }
        ImGuiLayer *getImGuiLayer() { return imGUILayer; }

        int getLastFPS() const { return last_fps; }
        double getFrameTime() const { return last_frameTime; }

        static Application &Get() { return *instance; }

    protected:
        bool editorMode;
        LayerStack layerStack;
        bool running = false;
        double frameTime;
        double last_frameTime;
        int last_fps;

        std::unique_ptr<Window> window;

        CoreEngine *engineLayer;
        ImGuiLayer *imGUILayer = nullptr;

        static Application *instance;
    };

    //To be defined in client
    Application *CreateApplication();
};

#endif