#include "prehistoric/core/scene/Scene.h"
#include "prehistoric/common/util/PlatformUtils.h"
#include "SceneHierarchyPanel.h"

#include "imgui.h"
#include "imgui_internal.h"

#include "prehistoric/core/node/component/renderer/RendererComponent.h"
#include "prehistoric/core/node/component/light/Light.h"
#include "prehistoric/core/node/component/VolumetricSphereComponent.h"
#include "prehistoric/application/Application.h"


SceneHierarchyPanel::SceneHierarchyPanel(Prehistoric::Scene *scene) {
    SetContext(scene);
}

void SceneHierarchyPanel::SetContext(Prehistoric::Scene *scene) {
    this->scene = scene;
    this->root = scene->getSceneRoot();
    InvalidateSelectionContext();
}

void SceneHierarchyPanel::SelectViewportHit(Prehistoric::GameObject *object, std::optional<uint32_t> submesh) {
    revealSelection = object != nullptr;
    selectedSubmesh = selectionContext == object ? submesh : std::nullopt;
    selectionContext = object;
}

void SceneHierarchyPanel::ImGuiRender() {
    ImGui::Begin("Scene Hierarchy");

    for (const auto &child: root->getChildrenByID()) {
        if (auto *object = dynamic_cast<Prehistoric::GameObject *>(child.second)) DrawGameObjectNode(object);
    }

    revealSelection = false;

    // Right-click on blank space
    if (ImGui::BeginPopupContextWindow(0, 1, false)) {
        if (ImGui::MenuItem("Create Empty Entity"))
            root->AddChild("Empty Entity", new Prehistoric::GameObject());

        if (ImGui::MenuItem("Create Volumetric Sphere")) {
            const auto *camera = Prehistoric::Application::Get().getEngineLayer()->getRenderingEngine()->getCamera();
            auto *object = new Prehistoric::GameObject();
            object->SetPosition(camera->getPosition() - camera->getForward() * 15.0f);
            object->AddComponent(Prehistoric::VolumetricSphereComponent::Name,
                                 new Prehistoric::VolumetricSphereComponent());
            root->AddChild("Volumetric Sphere", object);
            selectionContext = object;
            selectedSubmesh.reset();
        }

        ImGui::EndPopup();
    }

    ImGui::End();

    ImGui::Begin("Properties");
    if (!actionError.empty()) ImGui::TextWrapped("%s", actionError.c_str());
    if (selectionContext) {
        auto *component = selectionContext->GetComponent<Prehistoric::RendererComponent>();
        if (selectedSubmesh && component) DrawSubmesh(*component, *selectedSubmesh);
        else DrawComponents(selectionContext);
    }

    ImGui::End();
}

void SceneHierarchyPanel::DrawGameObjectNode(Prehistoric::GameObject *object) {
    std::string tag = object->getName();

    ImGuiTreeNodeFlags flags = ((selectionContext == object && !selectedSubmesh) ? ImGuiTreeNodeFlags_Selected : 0) |
                               ImGuiTreeNodeFlags_OpenOnArrow;
    flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
    if (revealSelection && selectionContext) {
        Prehistoric::Node *ancestor = selectedSubmesh ? selectionContext : selectionContext->getParent();
        for (; ancestor; ancestor = ancestor->getParent()) {
            if (ancestor == object) {
                ImGui::SetNextItemOpen(true);
                break;
            }
        }
    }
    bool opened = ImGui::TreeNodeEx((void *) object, flags, "%s", tag.c_str());
    if (revealSelection &&selectionContext
    
    ==
    object && !selectedSubmesh
    )
    ImGui::SetScrollHereY();
    if (ImGui::IsItemClicked()) {
        selectionContext = object;
        selectedSubmesh.reset();
    }

    bool entityDeleted = false;
    bool entityDuplicated = false;
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Duplicate Object", nullptr, false, !Prehistoric::Scene::IsSky(object)))
            entityDuplicated = true;
        if (ImGui::MenuItem("Delete Entity", nullptr, false, !Prehistoric::Scene::IsSky(object)))
            entityDeleted = true;

        ImGui::EndPopup();
    }

    if (opened) {
        auto *renderer = object->GetComponent<Prehistoric::RendererComponent>();
        auto *mesh = renderer ? renderer->getMeshBuffer() : nullptr;
        if (mesh && mesh->getSubmeshCount() > 1) {
            for (uint32_t i = 0; i < mesh->getSubmeshCount(); ++i) {
                const auto &part = mesh->getModel().getMesh(i);
                std::string label = part.getName();
                if (label.empty()) {
                    auto *mat = renderer->ResolveSubmeshMaterial(mesh->getModel(), i, renderer->getMaterial());
                    label = (mat ? mat->getName() : "Submesh") + " [" + std::to_string(i + 1) + "]";
                }
                ImGui::PushID(static_cast<int>(i));
                const bool selected = selectionContext == object && selectedSubmesh == i;
                ImGui::TreeNodeEx("part", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen |
                                          ImGuiTreeNodeFlags_SpanAvailWidth | (selected
                                                                                   ? ImGuiTreeNodeFlags_Selected
                                                                                   : 0), "%s", label.c_str());
                if (revealSelection && selected) ImGui::SetScrollHereY();
                if (ImGui::IsItemClicked()) {
                    selectionContext = object;
                    selectedSubmesh = i;
                }
                ImGui::PopID();
            }
        }

        for (const auto &child: object->getChildrenByID()) {
            if (auto *nested = dynamic_cast<Prehistoric::GameObject *>(child.second)) DrawGameObjectNode(nested);
        }
        ImGui::TreePop();
    }

    if (entityDuplicated) {
        try {
            selectionContext = scene->Duplicate(*object);
            selectedSubmesh.reset();
            revealSelection = true;
            actionError.clear();
        } catch (const std::exception &error) { actionError = error.what(); }
    }
    if (entityDeleted) {
        try { scene->CheckRemoval(object); } catch (const std::exception &error) {
            actionError = error.what();
            return;
        }
        for (Prehistoric::Node *node = selectionContext; node; node = node->getParent()) {
            if (node == object) {
                selectionContext = nullptr;
                break;
            }
        }
        object->getParent()->RemoveChild(object);
    }
}

void SceneHierarchyPanel::DrawSubmesh(Prehistoric::RendererComponent &component, uint32_t index) {
    using namespace Prehistoric;
    auto *mesh = component.getMeshBuffer();
    if (!mesh || index >= mesh->getSubmeshCount()) return;
    auto *mat = component.ResolveSubmeshMaterial(mesh->getModel(), index, component.getMaterial());
    if (!mat) return;
    ImGui::TextUnformatted(mesh->getModel().getMesh(index).getName().c_str());
    ImGui::Text("Material: %s", mat->getName().c_str());
    auto colour = mat->getVector3f(COLOUR);
    bool useTexture = mat->exists(ALBEDO_MAP) && colour.x < 0;
    if (mat->exists(ALBEDO_MAP) && ImGui::Checkbox("Use albedo texture", &useTexture)) {
        colour = Vector3f(useTexture ? -1.0f : 1.0f);
        component.EditSubmesh(index)->getVector3fs()[COLOUR] = colour;
    }
    if (!useTexture && ImGui::ColorEdit3("Base colour", &colour.x))
        component.EditSubmesh(index)->getVector3fs()[COLOUR] = colour;
    auto mrot = mat->getVector4f(MROT);
    bool changed = ImGui::SliderFloat("Metallic", &mrot.x, 0.0f, 1.0f);
    changed |= ImGui::SliderFloat("Roughness", &mrot.y, 0.0f, 1.0f);
    changed |= ImGui::SliderFloat("Occlusion", &mrot.z, 0.0f, 1.0f);
    if (changed) component.EditSubmesh(index)->getVector4fs()[MROT] = mrot;
    float emission = mat->getFloat(EMISSION);
    if (ImGui::DragFloat("Emission", &emission, 0.05f, 0.0f, 10000.0f))
        component.EditSubmesh(index)->getFloats()[EMISSION] = emission;
    if (component.getSubmeshOverride(index) && ImGui::Button("Reset to imported material"))
        component.ResetSubmesh(index);
}

template<typename T, typename UIFunction>
static void DrawComponent(const std::string &name, Prehistoric::GameObject *object, UIFunction uiFunction) {
    const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
                                             ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
                                             ImGuiTreeNodeFlags_FramePadding;
    Prehistoric::Component *comp;
    if (comp = object->GetComponent<T>()) {
        ImGui::PushID(comp);
        ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImGui::Separator();
        bool open = ImGui::TreeNodeEx((void *) typeid(T).hash_code(), treeNodeFlags, name.c_str());
        ImGui::PopStyleVar();
        ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
        if (ImGui::Button("+", ImVec2{lineHeight, lineHeight})) {
            ImGui::OpenPopup("ComponentSettings");
        }

        bool removeComponent = false;
        if (ImGui::BeginPopup("ComponentSettings")) {
            if (ImGui::MenuItem("Remove component"))
                removeComponent = true;

            ImGui::EndPopup();
        }

        if (open) {
            uiFunction(comp);
            ImGui::TreePop();
        }

        ImGui::PopID();
        if (removeComponent) {
            try { object->RemoveComponent<T>(); } catch (const std::exception &error) {
                PR_LOG_WARNING("%s\n", error.what());
            }
        }
    }
}

static void DrawVec3Control(const std::string &label, Prehistoric::Vector3f &values, float resetValue = 0.0f,
                            float columnWidth = 100.0f) {
    ImGuiIO &io = ImGui::GetIO();
    auto boldFont = io.Fonts->Fonts[0];

    ImGui::PushID(label.c_str());

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, columnWidth);
    ImGui::Text(label.c_str());
    ImGui::NextColumn();

    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

    float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
    ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.2f, 0.2f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
    ImGui::PushFont(boldFont);
    if (ImGui::Button("X", buttonSize))
        values.x = resetValue;
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
    ImGui::PushFont(boldFont);
    if (ImGui::Button("Y", buttonSize))
        values.y = resetValue;
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.35f, 0.9f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
    ImGui::PushFont(boldFont);
    if (ImGui::Button("Z", buttonSize))
        values.z = resetValue;
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();

    ImGui::PopStyleVar();

    ImGui::Columns(1);

    ImGui::PopID();
}

void SceneHierarchyPanel::DrawComponents(Prehistoric::GameObject *object) {
    auto oldTag = object->getName();
    auto &tag = object->getName();

    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    std::strncpy(buffer, tag.c_str(), sizeof(buffer));
    if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
        tag = std::string(buffer);
    }

    ImGui::SameLine();
    ImGui::PushItemWidth(-1);

    if (ImGui::Button("Add Component"))
        ImGui::OpenPopup("AddComponent");

    if (ImGui::BeginPopup("AddComponent")) {
        using Prehistoric::VolumetricSphereComponent;
        if (ImGui::MenuItem("Volumetric Sphere", nullptr, false, !object->HasComponent<VolumetricSphereComponent>())) {
            object->AddComponent(VolumetricSphereComponent::Name, new VolumetricSphereComponent());
        }

        if (ImGui::MenuItem("LightComponent")) {
            if (!selectionContext->GetComponent<Prehistoric::Light>()) {
                using namespace Prehistoric;
                selectionContext->AddComponent(LIGHT_COMPONENT, new Light(Vector3f(1), 1.0f, 10.0f, true, false));
            } else {
                PR_LOG_ERROR("This entity already has the Light Component!");
            }

            ImGui::CloseCurrentPopup();
        }

        if (ImGui::MenuItem("RendererComponent")) {
            if (!selectionContext->GetComponent<Prehistoric::RendererComponent>()) {
                using namespace Prehistoric;

                Window *window = Application::Get().getEngineLayer()->getRenderingEngine()->getWindow();
                AssembledAssetManager *manager = Application::Get().getEngineLayer()->getAssetManager();
                AssetManager *man = manager->getAssetManager();

                VertexBufferHandle vertexBuffer = man->loadVertexBuffer(std::nullopt, "engine://models/cube.obj").
                        value();
                ShaderHandle shader = man->loadShader(ShaderName::PBR).value();
                PipelineHandle pipeline = manager->
                        createPipeline(PipelineTypeHashFlags::Graphics, shader, vertexBuffer);

                MaterialHandle material = manager->storeMaterial(new Material(man));
                material->addVector3f(COLOUR, {1});
                material->addVector4f(MROT, {0.0f, 0.1f, 1.0f, 0.0f});
                material->addFloat(EMISSION, 0.0f);

                selectionContext->AddComponent(RENDERER_COMPONENT,
                                               new RendererComponent(window, manager, pipeline, material));
            } else {
                PR_LOG_ERROR("This entity already has the Renderer Component!");
            }

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (Prehistoric::Scene::IsSky(object)) {
        if (auto *environment = dynamic_cast<Prehistoric::EnvironmentMap *>(object)) {
            ImGui::TextWrapped("HDR: %s", environment->GetAsset().c_str());
            if (ImGui::Button("Choose HDR file")) {
                if (auto path = Prehistoric::FileDialogues::Get()->OpenFile("Environment Map (*.hdr)\0*.hdr\0")) {
                    try {
                        environment->SetAsset(*path);
                        actionError.clear();
                    } catch (const std::exception &error) { actionError = error.what(); }
                }
            }
            ImGui::TextWrapped("Global distant lighting: only rotation affects the image and lighting.");
            if (ImGui::Button("Replace with Atmosphere")) {
                try {
                    auto sky = std::make_unique<Prehistoric::Atmosphere>();
                    auto sun = Prehistoric::Atmosphere::CreateSun();
                    sky->setSun(sun->GetComponent<Prehistoric::Light>());
                    std::string name = "Atmosphere Sun";
                    for (unsigned n = 2; root->getChild(name); ++n) name = "Atmosphere Sun " + std::to_string(n);
                    auto *sunObject = sun.get();
                    root->AddChild(name, sun.release());
                    try { scene->ReplaceSky(std::move(sky)); } catch (...) {
                        root->RemoveChild(sunObject);
                        throw;
                    }
                    selectionContext = scene->GetSky();
                    actionError.clear();
                    ImGui::PopItemWidth();
                    return;
                } catch (const std::exception &error) { actionError = error.what(); }
            }
        } else if (ImGui::Button("Replace with Environment Map")) {
            if (auto path = Prehistoric::FileDialogues::Get()->OpenFile("Environment Map (*.hdr)\0*.hdr\0")) {
                try {
                    scene->ReplaceSky(std::make_unique<Prehistoric::EnvironmentMap>(*path));
                    selectionContext = scene->GetSky();
                    actionError.clear();
                    ImGui::PopItemWidth();
                    return;
                } catch (const std::exception &error) { actionError = error.what(); }
            }
        }
    }

    Prehistoric::Transform &tr = object->getWorldTransform();
    DrawVec3Control("Transform", tr.getPosition());
    DrawVec3Control("Rotation", tr.getRotation());
    DrawVec3Control("Scale", tr.getScaling());

    tr.calculateTransformationMatrix();

    DrawComponent<Prehistoric::RendererComponent>("RendererComponent", object, [](Prehistoric::Component *comp) {
        Prehistoric::RendererComponent *component = (Prehistoric::RendererComponent *) comp;
        Prehistoric::Material *mat = component->getMaterial();

        if (component->getPipeline() == nullptr || mat == nullptr) {
            return;
        }

        ImGui::Text("Pipeline Handle: %d", component->getPipelineHandle().handle);
        ImGui::Text("Material Handle: %d", component->getMaterialHandle().handle);
        ImGui::Text("Shader Handle: %d", component->getPipeline()->getShaderHandle().handle);
        ImGui::Text("Textures:");

        for (auto tex: mat->getTextures()) {
            ImGui::Text("%s: (handle) %d", tex.first.c_str(), tex.second.handle);
        }

        for (auto &vec4: mat->getVector4fs()) {
            float vals[] = {vec4.second.x, vec4.second.y, vec4.second.z, vec4.second.w};
            ImGui::ColorEdit4(vec4.first.c_str(), vals);
            vec4.second = Prehistoric::Vector4f(vals[0], vals[1], vals[2], vals[3]);
        }

        for (auto &vec3: mat->getVector3fs()) {
            float vals[] = {vec3.second.x, vec3.second.y, vec3.second.z};
            ImGui::ColorEdit3(vec3.first.c_str(), vals);
            vec3.second = Prehistoric::Vector3f(vals[0], vals[1], vals[2]);
        }

        for (auto &fl: mat->getFloats()) {
            float val[] = {fl.second};
            ImGui::InputFloat(fl.first.c_str(), val, 0.5f, 10.0f);
            //ImGui::SliderFloat(fl.first.c_str(), val, 0.0f, 20.0f, fl.first.c_str());
            fl.second = val[0];
        }
    });

    DrawComponent<Prehistoric::Light>("Light", object, [](Prehistoric::Component *comp) {
        Prehistoric::Light *light = (Prehistoric::Light *) comp;

        {
            float vals[] = {light->getColour().x, light->getColour().y, light->getColour().z};
            ImGui::ColorEdit3("Colour", vals);
            light->setColour({vals[0], vals[1], vals[2]});
        }

        {
            float intensity = light->getIntensity();
            ImGui::InputFloat("Intensity", &intensity, 0.5f, 10.0f);
            light->setIntensity({intensity});
        }

        {
            float radius = light->getRadius();
            ImGui::InputFloat("Radius", &radius, 10.0f, 200.0f);
            light->setRadius({radius});
        }
    });

    DrawComponent<Prehistoric::VolumetricSphereComponent>("Volumetric Sphere", object,
                                                          [](Prehistoric::Component *comp) {
                                                              auto *sphere = static_cast<
                                                                  Prehistoric::VolumetricSphereComponent *>(comp);
                                                              float radius = sphere->getRadius();
                                                              float density = sphere->getDensity();
                                                              float anisotropy = sphere->getAnisotropy();
                                                              const auto &tint = sphere->getTint();
                                                              float colour[] = {tint.x, tint.y, tint.z};
                                                              if (ImGui::DragFloat(
                                                                  "Radius", &radius, 0.1f, 0.0f, 0.0f,
                                                                  "%.2f"))
                                                                  sphere->setRadius(radius);
                                                              if (ImGui::DragFloat(
                                                                  "Density", &density, 0.001f, 0.0f, 0.0f,
                                                                  "%.4f"))
                                                                  sphere->setDensity(density);
                                                              if (ImGui::ColorEdit3("Tint", colour))
                                                                  sphere->setTint({
                                                                      colour[0], colour[1], colour[2]
                                                                  });
                                                              if (ImGui::SliderFloat(
                                                                  "Anisotropy", &anisotropy, -0.95f,
                                                                  0.95f))
                                                                  sphere->setAnisotropy(anisotropy);
                                                          });

    ImGui::PopItemWidth();
}