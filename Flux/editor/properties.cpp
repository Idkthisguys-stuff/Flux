#include "properties.h"
#include "heiarchy.h"
#include "logic/Textureloader.h"
#include "output.h"
#include "render/3D/OpenGL/Model.h"
#include <algorithm>
#include <filesystem>
#include <variant>

namespace Flux
{

static bool BeginTable2Col(const char *id = "##t")
{
    if (ImGui::BeginTable(id, 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings))
    {
        ImGui::TableSetupColumn("L", ImGuiTableColumnFlags_WidthFixed, 100.f);
        ImGui::TableSetupColumn("V", ImGuiTableColumnFlags_WidthStretch);
        return true;
    }
    return false;
}

static bool DragVec3Row(const char *label, glm::vec3 &v, float speed = 0.1f, Heiarchy *h = nullptr)
{
    float a[3] = {v.x, v.y, v.z};
    ImGui::PushID(label);
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);
    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-1);
    bool c = ImGui::DragFloat3("##v", a, speed, -FLT_MAX, FLT_MAX, "%.3f");
    if (c)
        v = {a[0], a[1], a[2]};
    if (ImGui::IsItemDeactivatedAfterEdit() && h)
        h->PushUndoState();
    ImGui::PopID();
    return c;
}

static bool ColorRow(const char *label, glm::vec3 &c, Heiarchy *h = nullptr)
{
    float col[3] = {c.r, c.g, c.b};
    ImGui::PushID(label);
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);
    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-1);
    bool ch = ImGui::ColorEdit3("##c", col, ImGuiColorEditFlags_Float);
    if (ch)
        c = {col[0], col[1], col[2]};
    if (ImGui::IsItemDeactivatedAfterEdit() && h)
        h->PushUndoState();
    ImGui::PopID();
    return ch;
}

static bool FloatRow(const char *label, float &f, float spd = 0.01f, float mn = 0.f, float mx = FLT_MAX,
                     const char *fmt = "%.3f", ImGuiSliderFlags flags = 0, Heiarchy *h = nullptr)
{
    ImGui::PushID(label);
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);
    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-1);
    bool c = ImGui::DragFloat("##f", &f, spd, mn, mx, fmt);
    if (ImGui::IsItemDeactivatedAfterEdit() && h)
        h->PushUndoState();
    ImGui::PopID();
    return c;
}

static bool SliderRow(const char *label, float &f, float mn = 0.f, float mx = 1.0f, const char *fmt = "%.2f",
                      Heiarchy *h = nullptr)
{
    ImGui::PushID(label);
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);
    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-1);
    bool c = ImGui::SliderFloat("##s", &f, mn, mx, fmt);
    if (ImGui::IsItemDeactivatedAfterEdit() && h)
        h->PushUndoState();
    ImGui::PopID();
    return c;
}

static void TextureSlot(const char *label, SceneNode &node, Heiarchy *h = nullptr)
{
    float avail = ImGui::GetContentRegionAvail().x;
    float swatchH = 56.f;

    ImGui::Spacing();
    ImGui::TextUnformatted(label);

    ImGui::PushID(label);
    if (node.textureID != 0)
    {
        ImGui::Image(reinterpret_cast<void *>(static_cast<intptr_t>(node.textureID)), ImVec2(avail - 60.f, swatchH),
                     ImVec2(0, 1), ImVec2(1, 0));
    }
    else
    {
        float col[3] = {node.baseColor.r, node.baseColor.g, node.baseColor.b};
        ImGui::SetNextItemWidth(avail - 60.f);
        if (ImGui::ColorButton("##bc", ImVec4(col[0], col[1], col[2], 1.f),
                               ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoBorder,
                               ImVec2(avail - 60.f, swatchH)))
        {
            ImGui::OpenPopup("##bcPicker");
        }
        if (ImGui::BeginPopup("##bcPicker"))
        {
            bool ch = ImGui::ColorPicker3("##bcp", col, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoSidePreview);
            if (ch)
            {
                node.baseColor = {col[0], col[1], col[2]};
            }
            if (ImGui::IsItemDeactivatedAfterEdit() && h)
                h->PushUndoState();
            ImGui::EndPopup();
        }
    }

    ImVec2 dropMin = ImGui::GetItemRectMin();
    ImVec2 dropMax = ImGui::GetItemRectMax();

    ImGui::SameLine();
    if (!node.texturePath.empty())
    {
        if (ImGui::SmallButton("X##clr"))
        {
            if (h)
                h->PushUndoState();
            node.texturePath = "";
            node.textureID = 0;
            if (node.model)
                node.model->SetTexture(0);
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Clear texture");
    }
    else
    {
        ImGui::TextDisabled(" drop\n here");
    }

    ImGui::SetCursorScreenPos(dropMin);
    ImGui::InvisibleButton("##texDrop", ImVec2(dropMax.x - dropMin.x, dropMax.y - dropMin.y));
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload *p = ImGui::AcceptDragDropPayload("EXPLORER_FILE"))
        {
            std::string droppedPath(static_cast<const char *>(p->Data));
            std::string ext = std::filesystem::path(droppedPath).extension().string();
            if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga")
            {
                if (h)
                    h->PushUndoState();
                node.texturePath = droppedPath;
                node.textureID = TextureLoader::Load(droppedPath);
                if (node.model)
                    node.model->SetTexture(node.textureID);
            }
        }
        ImGui::EndDragDropTarget();
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
        ImGui::GetWindowDrawList()->AddRect(dropMin, dropMax, IM_COL32(80, 160, 255, 200), 3.f, 0, 2.f);

    if (!node.texturePath.empty())
        ImGui::TextDisabled("%s", std::filesystem::path(node.texturePath).filename().string().c_str());

    ImGui::PopID();
}

static glm::vec3 DirToEuler(glm::vec3 dir)
{
    dir = glm::normalize(dir);
    float pitch = glm::degrees(std::asin(-dir.y));
    float yaw = glm::degrees(std::atan2(dir.x, -dir.z));
    return glm::vec3(pitch, yaw, 0.f);
}

static void DrawProfileAndStats(Heiarchy *h)
{
    ImGui::Separator();
    if (ImGui::CollapsingHeader("Profile & Stats", ImGuiTreeNodeFlags_DefaultOpen))
    {
        float fps = ImGui::GetIO().Framerate;
        ImGui::Text("FPS: %.1f", fps);
        ImGui::Text("RAM: 3.71 GB");

        int totalDrawCalls = 0;
        int totalVertices = 0;
        if (h)
        {
            for (const auto &n : h->nodes)
            {
                if (n.model)
                {
                    totalDrawCalls += (int)n.model->meshes.size();
                    for (const auto &mesh : n.model->meshes)
                    {
                        totalVertices += (int)mesh.verticies.size();
                    }
                }
            }
        }
        ImGui::Text("Draw Calls: %d", totalDrawCalls);
        ImGui::Text("Vertices: %d", totalVertices);
    }
}

static void gatherAssetsByExtension(const virtualFile &folder, const std::vector<std::string> &extensions,
                                    std::vector<std::string> &outPaths)
{
    for (const virtualFile &child : folder.children)
    {
        if (child.type == Flux::fileType::Folder)
        {
            gatherAssetsByExtension(child, extensions, outPaths);
        }
        else
        {
            std::string ext = child.path.extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

            for (const std::basic_string<char> &targetExt : extensions)
            {
                if (ext == targetExt)
                {
                    outPaths.push_back(child.path.string());
                    break;
                }
            }
        }
    }
}

void Properties::renderProperties(Assets &assets, Heiarchy *h)
{
    ImGui::Begin("Properties");
    if (ImGui::IsWindowHovered())
        ImGui::SetWindowFocus();

    if (!h || h->selectedIndices.empty())
    {
        ImGui::TextDisabled("No object selected.");
    }
    else
    {
        if (h->selectedIndices.size() > 1)
        {
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "%d items selected. Editing primary.",
                               (int)h->selectedIndices.size());
        }

        int primaryIndex = h->lastClickedIndex != -1 ? h->lastClickedIndex : h->selectedIndices.back();
        SceneNode &node = h->nodes[primaryIndex];

        if (node.isLightingNode)
        {
            ImGui::TextColored(ImVec4(1.f, 0.85f, 0.35f, 1.f), "Lighting  [locked]");
        }
        else
        {
            char nameBuf[128];
            std::strncpy(nameBuf, node.name.c_str(), sizeof(nameBuf) - 1);
            nameBuf[sizeof(nameBuf) - 1] = '\0';
            ImGui::SetNextItemWidth(-1);
            if (ImGui::InputText("##name", nameBuf, sizeof(nameBuf)))
                node.name = nameBuf;
        }
        ImGui::Separator();

        if (node.isLightingNode)
        {
            ImGui::Text("Lighting Properties");
            ImGui::Spacing();

            if (BeginTable2Col("##t_lighting"))
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted("Time of Day");
                ImGui::TableSetColumnIndex(1);
                ImGui::SetNextItemWidth(-1);
                int h24 = (int)node.light.timeOfDay;
                int m60 = (int)((node.light.timeOfDay - h24) * 60.f);
                char disp[16];
                snprintf(disp, sizeof(disp), "%02d:%02d", h24, m60);
                ImGui::PushID("tod");
                if (ImGui::DragFloat("##tod", &node.light.timeOfDay, 0.01f, 0.f, 24.f, disp))
                {
                    float angle = (node.light.timeOfDay - 12.0f) * 15.0f;
                    node.rotation.x = angle;

                    glm::quat q = glm::angleAxis(glm::radians(node.rotation.y), glm::vec3(0, 1, 0)) *
                                  glm::angleAxis(glm::radians(node.rotation.x), glm::vec3(1, 0, 0));
                    node.light.direction = glm::normalize(q * glm::vec3(0.f, -1.f, 0.f));
                }
                ImGui::PopID();
            }

            FloatRow("Brightness", node.light.brightness, 0.01f, 0.f, 10.f, "%.3f", 0, h);
            ColorRow("Color", node.light.color, h);
            ColorRow("Moon Color", node.light.moonColor, h);
            FloatRow("Moon Intensity", node.light.moonIntensity, 0.01f, 0.f, 100.f, "%.3f", 0, h);
            ColorRow("ColorShift", node.light.colorShift, h);
            FloatRow("Ambient Day", node.light.ambientDaytime, 0.005f, 0.f, 1.f, "%.3f", 0, h);
            FloatRow("Ambient Night", node.light.ambientNight, 0.005f, 0.f, 1.f, "%.3f", 0, h);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Direction");
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-1);
            {
                float arr[3] = {node.light.direction.x, node.light.direction.y, node.light.direction.z};
                ImGui::PushID("ldir");
                if (ImGui::DragFloat3("##ld", arr, 0.005f, -1.f, 1.f, "%.3f"))
                {
                    node.light.direction = glm::normalize(glm::vec3(arr[0], arr[1], arr[2]));
                    node.rotation = DirToEuler(node.light.direction);
                }
                if (ImGui::IsItemDeactivatedAfterEdit() && h)
                    h->PushUndoState();
                ImGui::PopID();
            }

            ImGui::Separator();
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Spacing();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("Fog Color");
            ImGui::TableSetColumnIndex(1); /* handled below */
            ImGui::EndTable();

            if (BeginTable2Col("##t_lighting_fog"))
            {
                ColorRow("Fog Color", node.light.fogColor, h);
                FloatRow("Fog Start", node.light.fogStart, 1.f, 0.f, 5000.f, "%.3f", 0, h);
                FloatRow("Fog End", node.light.fogEnd, 1.f, 0.f, 5000.f, "%.3f", 0, h);
                ImGui::EndTable();
            }
        }
        else
        {
            ImGui::Text("Local Transform");
            ImGui::Spacing();
            if (BeginTable2Col("##t_transform_local"))
            {
                DragVec3Row("Position", node.position, 0.1f, h);
                DragVec3Row("Rotation", node.rotation, 0.5f, h);
                DragVec3Row("Scale", node.scale, 0.01f, h);
                ImGui::EndTable();
            }

            if (node.parentIndex != -1)
            {
                ImGui::Separator();
                ImGui::Text("Absolute Transform (Read-Only)");
                glm::mat4 worldMat = node.GetWorldTransform(h->nodes);
                float t[3], r[3], s[3];
                ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(worldMat), t, r, s);

                ImGui::BeginDisabled();
                if (BeginTable2Col("##t_transform_world"))
                {
                    glm::vec3 wt(t[0], t[1], t[2]);
                    glm::vec3 wr(r[0], r[1], r[2]);
                    DragVec3Row("World Pos", wt);
                    DragVec3Row("World Rot", wr);
                    ImGui::EndTable();
                }
                ImGui::EndDisabled();

                if (h->nodes[node.parentIndex].type == NodeType::Empty)
                {
                    ImGui::Separator();
                    if (ImGui::Checkbox("Independent", &node.isIndependent))
                    {
                        h->PushUndoState();
                    }
                    ImGui::SameLine();
                    ImGui::TextDisabled("(Ignores Empty Parent)");
                }
            }

            int compIndex = 0;
            int componentToRemove = -1;

            bool canRemove = true;

            for (auto &comp : node.components)
            {
                ImGui::PushID(compIndex);
                bool isHeaderOpen = ImGui::CollapsingHeader(comp.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen |
                                                                                   ImGuiTreeNodeFlags_AllowOverlap);

                ImGui::SameLine(ImGui::GetWindowWidth() - 32.f);
                if (ImGui::Button("...##options", ImVec2(22.f, 18.f)))
                {
                    ImGui::OpenPopup("ComponentContextMenu");
                }

                if (ImGui::BeginPopup("ComponentContextMenu"))
                {
                    if (ImGui::MenuItem("Reset Component values"))
                    {
                        // Reset here
                    }
                    if (ImGui::MenuItem("Remove Component", nullptr, false, canRemove))
                    {
                        componentToRemove = compIndex;
                        if (h)
                            h->PushUndoState();
                    }
                    ImGui::EndPopup();
                }

                if (isHeaderOpen)
                {
                    ImGui::Spacing();

                    if (node.hasComponent<CameraComponent>() && comp.name == "Camera Settings")
                    {
                        canRemove = true;
                    }
                    else if (node.type == NodeType::Mesh && comp.name == "Mesh Renderer")
                    {
                        canRemove = true;
                    }

                    if (!canRemove)
                    {
                        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "WARNING: Essential Component!");
                        canRemove = true;
                    }

                    std::visit(
                        [&](auto &&arg) {
                            using T = std::decay_t<decltype(arg)>;

                            if constexpr (std::is_same_v<T, CameraComponent>)
                            {
                                if (ImGui::Checkbox("Viewing Camera", &arg.isMainCamera))
                                {
                                    if (h)
                                        h->PushUndoState();

                                    if (arg.isMainCamera && h)
                                    {
                                        for (SceneNode &otherNode : h->nodes)
                                        {
                                            if (&otherNode == &node)
                                                continue;

                                            otherNode.isMainCamera = false;

                                            for (auto &comp : otherNode.components)
                                            {
                                                if (std::holds_alternative<CameraComponent>(comp.data))
                                                {
                                                    std::get<CameraComponent>(comp.data).isMainCamera = false;
                                                }
                                            }
                                        }
                                    }
                                }

                                if (BeginTable2Col("##t_comp_cam"))
                                {
                                    SliderRow("FOV", arg.fov, 10.0f, 170.0f, "&.1f deg", h);
                                    ImGui::EndTable();
                                }

                                node.fov = arg.fov;
                                node.isMainCamera = arg.isMainCamera;
                            }
                            else if constexpr (std::is_same_v<T, MeshComponent>)
                            {
                                ImGui::TextUnformatted("Model File");

                                std::string currentModelName =
                                    arg.modelPath.empty() ? "None (Click to select)"
                                                          : std::filesystem::path(arg.modelPath).filename().string();

                                float width = ImGui::GetContentRegionAvail().x - 35.0f;
                                ImGui::SetNextItemWidth(width);

                                if (ImGui::BeginCombo("###ModelDropdown", currentModelName.c_str()))
                                {
                                    std::vector<std::string> modelPaths;

                                    gatherAssetsByExtension(assets.projectRoot, {".obj", ".fbx"}, modelPaths);

                                    if (ImGui::Selectable("None", arg.modelPath.empty()))
                                    {
                                        if (h)
                                            h->PushUndoState();
                                        arg.modelPath = "";
                                        node.model = nullptr;
                                    }

                                    for (const std::basic_string<char> &path : modelPaths)
                                    {
                                        std::string fileName = std::filesystem::path(path).filename().string();
                                        bool isSelected = (arg.modelPath == path);

                                        if (ImGui::Selectable(fileName.c_str(), isSelected))
                                        {
                                            if (h)
                                                h->PushUndoState();
                                            arg.modelPath = path;
                                            node.model = h->GetOrLoadModel(path);
                                        }
                                        if (isSelected)
                                        {
                                            ImGui::SetItemDefaultFocus();
                                        }
                                    }
                                    ImGui::EndCombo();
                                }

                                if (ImGui::BeginDragDropTarget())
                                {
                                    if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("EXPLORER_FILE"))
                                    {
                                        std::string droppedPath(static_cast<const char *>(payload->Data));
                                        std::string ext = std::filesystem::path(droppedPath).extension().string();
                                        if (ext == ".obj" || ext == ".fbx")
                                        {
                                            if (h)
                                                h->PushUndoState();
                                            arg.modelPath = droppedPath;
                                            node.model = h->GetOrLoadModel(droppedPath);
                                        }
                                    }

                                    ImGui::EndDragDropTarget();
                                }
                                if (ImGui::IsItemHovered())
                                    ImGui::SetTooltip("Clear current model");

                                if (!arg.modelPath.empty())
                                {
                                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
                                    ImGui::TextWrapped("Path: %s", arg.modelPath.c_str());
                                    ImGui::PopStyleColor();
                                }
                                ImGui::Spacing();
                                ImGui::Separator();
                                ImGui::Spacing();

                                ImGui::TextUnformatted("Texture File");

                                std::string currentTextureName =
                                    node.texturePath.empty()
                                        ? "None (Click to select)"
                                        : std::filesystem::path(node.texturePath).filename().string();
                                ImGui::SetNextItemWidth(width);

                                if (ImGui::BeginCombo("##TextureDropdown", currentTextureName.c_str()))
                                {
                                    std::vector<std::string> texturePaths;
                                    gatherAssetsByExtension(assets.projectRoot,
                                                            {".png", ".jpg", ".jpeg", ".bmp", ".tga"}, texturePaths);

                                    if (ImGui::Selectable("None", node.texturePath.empty()))
                                    {
                                        if (h)
                                            h->PushUndoState();
                                        node.texturePath = "";
                                        node.textureID = 0;
                                        if (node.model)
                                            node.model->SetTexture(0);
                                    }

                                    for (const auto &path : texturePaths)
                                    {
                                        std::string filename = std::filesystem::path(path).filename().string();
                                        bool isSelected = (node.texturePath == path);
                                        if (ImGui::Selectable(filename.c_str(), isSelected))
                                        {
                                            if (h)
                                                h->PushUndoState();
                                            node.texturePath = path;
                                            node.textureID = TextureLoader::Load(path);
                                            if (node.model)
                                                node.model->SetTexture(node.textureID);
                                        }
                                        if (isSelected)
                                        {
                                            ImGui::SetItemDefaultFocus();
                                        }
                                    }
                                    ImGui::EndCombo();
                                }

                                if (ImGui::BeginDragDropTarget())
                                {
                                    if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("EXPLORER_FILE"))
                                    {
                                        std::string droppedPath(static_cast<const char *>(payload->Data));
                                        std::string ext = std::filesystem::path(droppedPath).extension().string();
                                        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" ||
                                            ext == ".tga")
                                        {
                                            if (h)
                                                h->PushUndoState();
                                            node.texturePath = droppedPath;
                                            node.textureID = TextureLoader::Load(droppedPath);
                                            if (node.model)
                                                node.model->SetTexture(node.textureID);
                                        }
                                    }
                                    ImGui::EndDragDropTarget();
                                }

                                ImGui::SameLine();
                                if (ImGui::Button("X##ClearTexture", ImVec2(25.f, 0.f)))
                                {
                                    if (h)
                                        h->PushUndoState();
                                    node.texturePath = "";
                                    node.textureID = 0;
                                    if (node.model)
                                        node.model->SetTexture(0);
                                }
                                if (ImGui::IsItemHovered())
                                    ImGui::SetTooltip("Clear current texture");

                                if (!node.texturePath.empty())
                                {
                                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
                                    ImGui::TextWrapped("Path: %s", node.texturePath.c_str());
                                    ImGui::PopStyleColor();
                                }

                                ImGui::Spacing();
                                ImGui::Separator();

                                if (BeginTable2Col("##t_comp_mesh"))
                                {
                                    SliderRow("Roughness", arg.roughness, 0.0f, 1.0f, "%.2f", h);
                                    SliderRow("Metallic", arg.metallic, 0.0f, 1.0f, "%.2f", h);

                                    ImGui::EndTable();
                                }

                                node.roughness = arg.roughness;
                                node.metallic = arg.metallic;
                            }
                            else if constexpr (std::is_same_v<T, PhysicsComponent>)
                            {
                                if (BeginTable2Col("##t_comp_phys"))
                                {
                                    DragVec3Row("Velocity", arg.velocity, 0.1f, h);

                                    ImGui::TableNextRow();
                                    ImGui::TableSetColumnIndex(0);
                                    ImGui::AlignTextToFramePadding();
                                    ImGui::TextUnformatted("Anchored");

                                    ImGui::TableSetColumnIndex(1);
                                    if (ImGui::Checkbox("##anch", &arg.isAnchored))
                                    {
                                        if (h)
                                            h->PushUndoState();
                                    }

                                    ImGui::EndTable();
                                }

                                node.velocity = arg.velocity;
                                node.isAnchored = arg.isAnchored;
                            }
                        },
                        comp.data);
                    ImGui::Spacing();
                }

                ImGui::PopID();
                compIndex++;
            }

            if (componentToRemove != -1)
            {
                node.components.erase(node.components.begin() + componentToRemove);
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            float contentWidth = ImGui::GetContentRegionAvail().x;
            ImGui::SetCursorPosX((contentWidth - 160.0f) * 0.5f);

            if (ImGui::Button("Add Component", ImVec2(160.f, 26.f)))
            {
                ImGui::OpenPopup("AddComponentMenuPopup");
            }

            if (ImGui::BeginPopup("AddComponentMenuPopup"))
            {
                auto hasComponent = [&](const std::string &name) {
                    return std::any_of(node.components.begin(), node.components.end(),
                                       [&](const auto &c) { return c.name == name; });
                };

                if (ImGui::MenuItem("Camera"))
                {
                    if (h)
                        h->PushUndoState();
                    node.components.push_back({"Camera Settings", CameraComponent{70.0f, false}});
                }
                if (ImGui::MenuItem("Mesh Renderer"))
                {
                    if (h)
                        h->PushUndoState();
                    node.components.push_back({"Mesh Renderer", MeshComponent{"", 0.7f, 0.0f}});
                }
                if (ImGui::MenuItem("Rigidbody"))
                {
                    if (h)
                        h->PushUndoState();
                    node.components.push_back({"Rigidbody", PhysicsComponent{glm::vec3(0.f), false}});
                }
                ImGui::EndPopup();
            }
        }
        DrawProfileAndStats(h);
    }

    ImGui::End();
}
} // namespace Flux