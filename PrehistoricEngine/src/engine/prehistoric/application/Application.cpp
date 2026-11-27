#include "Includes.hpp"
#include "Application.h"

#include "prehistoric/core/events/ApplicationEvent.h"

namespace Prehistoric {
    Application *Application::instance = nullptr;

    Application::Application(bool editor) : editorMode(editor) {
        if (instance)
            PR_LOG_RUNTIME_ERROR("The application already exists!\n");

        instance = this;

        engineLayer = new CoreEngine();
        engineLayer->LoadEngines();
        engineLayer->getRenderingEngine()->getWindow()->setEventCallback(
            std::bind(&Application::OnEvent, this, std::placeholders::_1));

        //Input initialisation
        InputInstance.Init(engineLayer->getRenderingEngine()->getWindow());

        PushLayer(engineLayer);

        {
            imGUILayer = new ImGuiLayer();
            PushOverlay(imGUILayer);
            imGUILayer->SetDarkThemeColors();
        }

        frameTime = 1.0 / __FrameworkConfig.windowMaxFPS;
        last_fps = __FrameworkConfig.windowMaxFPS;
    }

    Application::~Application() {
        if (imGUILayer) {
            //Making sure that the ImGUI layer is deleted before the core engine, therefore deleting the GLFWwindow which is needed by ImGUI
            layerStack.PopOverlay(imGUILayer);
            delete imGUILayer;
        }
    }

    void Application::Run() {
        if (running)
            return;

        running = true;

        uint32_t frames = 0;
        double frameCounter = 0;

        long long lastTime = Time::getTimeFromStartNanoseconds();
        double unprocessedTime = 0;

        while (running) {
            bool render = false;

            long long startTime = Time::getTimeFromStartNanoseconds();
            long long passedTime = startTime - lastTime;
            lastTime = startTime;

            //This piece of code is put here so that whenever the window is iconified, the only code that runs is what is absolutely necessary
            //The code before this check is necessary so that the elapsed time is not tracked, but updated (lastTime = startTime)*
            if (InputInstance.IsPause()) {
                //Checks for updates, so that we can restore the window later
                InputInstance.Update();
                engineLayer->getRenderingEngine()->getWindow()->Input();

                //Makes sure that this loop only runs a 1000 times per second at most, avoiding unnecessary CPU load
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(1ms);

                continue;
            }

            unprocessedTime += passedTime / NANOSECOND;
            frameCounter += passedTime;

            while (unprocessedTime > frameTime) {
                render = true;
                unprocessedTime -= frameTime;

                if (engineLayer->getRenderingEngine()->getWindow()->ShouldClose()) {
                    Stop();
                    break;
                }

                for (auto it = layerStack.rbegin(); it != layerStack.rend(); ++it) {
                    double passed = static_cast<double>(passedTime) / NANOSECOND;
                    passed = std::min(passed, frameTime);
                    (*it)->Update(static_cast<float>(passed));
                }

                if (frameCounter >= NANOSECOND) {
                    last_fps = frames;
                    last_frameTime = passedTime / NANOSECOND * 1000.0f;
                    PR_LOG(CYAN, "FPS: %i\n", frames);
                    PR_LOG(CYAN, "Delta time: %f ms\n", last_frameTime);
                    frames = 0;
                    frameCounter = 0;
                }
            }

            // Editor panels may have focus in another platform window. Keep the
            // scene/UI live while unfocused; iconification is handled above.
            if (render && (editorMode || !InputInstance.IsPauseRendering())) {
                engineLayer->BeginRendering();
                auto *renderer = engineLayer->getRenderingEngine()->getRenderer();
                const bool drawUI = imGUILayer && renderer->CanRenderFrame();
                auto buildUI = [&]() {
                    imGUILayer->Begin();
                    for (auto it = layerStack.rbegin(); it != layerStack.rend(); ++it) {
                        (*it)->ImGUIRender();
                    }
                    imGUILayer->End();
                };
                // Apply editor scene mutations before recording commands that reference scene assets.
                if (drawUI && __FrameworkConfig.api == Vulkan) buildUI();

                for (auto it = layerStack.rbegin(); it != layerStack.rend(); ++it) {
                    (*it)->Render();
                }
                if (drawUI && __FrameworkConfig.api == OpenGL) {
                    buildUI();
                    imGUILayer->Draw();
                }
                engineLayer->EndRendering();
                if (drawUI) imGUILayer->RenderPlatformWindows();

                frames++;
            } else {
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(1ms);
            }
        }
    }

    void Application::Stop() {
        if (!running)
            return;

        running = false;
    }

    void Application::OnEvent(Event &e) {
        for (auto it = layerStack.rbegin(); it != layerStack.rend(); ++it) {
            if (e.handled)
                break;

            (*it)->OnEvent(e);
        }
    }

    void Application::PushLayer(Layer *layer) {
        layerStack.PushLayer(layer);
        layer->OnAttach();
    }

    void Application::PushOverlay(Layer *layer) {
        layerStack.PushOverlay(layer);
        layer->OnAttach();
    }
};