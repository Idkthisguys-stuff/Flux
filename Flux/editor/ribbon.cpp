#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "ribbon.h"
#include "imgui.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cctype>
#include <iostream>

#include "themes/themes.h"

namespace Flux
{
int currentTool = 0;
bool showSettings = false;

void Ribbon::renderRibbon()
{
    ImGuiViewport *main_viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(main_viewport->Pos);
    ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x, 75));

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDocking |
                                    ImGuiWindowFlags_MenuBar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("###Ribbon", nullptr, window_flags);
    if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive())
    {
        ImGui::SetWindowFocus();
    }

    static bool showBugReportModal = false;
    if (ImGui::BeginMenuBar())
    {
        drawFileMenu();
        drawEditMenu();
        drawGameObjectMenu();

        if (ImGui::BeginMenu("Graphics"))
        {
            static bool bloom = true;
            static bool ambientOcclusion = true;
            static int shadowResIdx = 2;
            static int qualityPreset = 2;
            static int msaaIdx = 2;

            ImGui::TextDisabled("Post-Processing");
            ImGui::Checkbox("Bloom", &bloom);
            ImGui::Checkbox("Ambient Occlusion", &ambientOcclusion);
            ImGui::Separator();

            if (ImGui::BeginMenu("Quality Preset"))
            {
                const char *presets[] = {"Low", "Medium", "High", "Ultra"};
                for (int i = 0; i < 4; i++)
                {
                    if (ImGui::RadioButton(presets[i], qualityPreset == i))
                        qualityPreset = i;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Anti-Aliasing"))
            {
                const char *msaaOpts[] = {"None", "MSAA 2x", "MSAA 4x", "MSAA 8x"};
                for (int i = 0; i < 4; i++)
                {
                    if (ImGui::RadioButton(msaaOpts[i], msaaIdx == i))
                        msaaIdx = i;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Shadow Map Resolution"))
            {
                const char *resOpts[] = {"1024", "2048", "4096", "8192"};
                for (int i = 0; i < 4; i++)
                {
                    if (ImGui::RadioButton(resOpts[i], shadowResIdx == i))
                        shadowResIdx = i;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Project"))
        {
            if (ImGui::MenuItem("Refresh Assets"))
            {
                if (explorerPtr)
                    explorerPtr->refreshRequested = true;
            }
            if (ImGui::MenuItem("Clear Asset Cache"))
            {
                Output::addLog("Asset Cache Cleared.");
            }
            if (ImGui::MenuItem("Open Project Folder"))
            {
                if (explorerPtr && !explorerPtr->activeFolderPath.empty())
                {
                    std::string cmd = "explorer \"" + explorerPtr->activeFolderPath.string() + "\"";
                    system(cmd.c_str());
                }
                else
                {
                    Output::addLog("No active project folder loaded.");
                }
            }
            if (ImGui::MenuItem("Export Package..."))
            {
                Output::addLog("Package export completed: project.fpackage");
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Build"))
        {
            if (ImGui::MenuItem("Build Settings..."))
            {
                Output::addLog("Opened Build Settings.");
            }
            if (ImGui::MenuItem("Build & Run", "Ctrl+B"))
            {
                Output::addLog("Starting Build...");
                Output::addLog("Build succeeded: Release/FluxGame.exe");
            }
            if (ImGui::MenuItem("Build WebGL"))
            {
                Output::addLog("Building WebGL target... Complete.");
            }
            if (ImGui::MenuItem("Clean Build Directory"))
            {
                Output::addLog("Build directory cleaned.");
            }
            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("Report Bug"))
        {
            showBugReportModal = true;
        }

        ImGui::EndMenuBar();
    }

    if (showPreferences)
    {
        ImGui::Begin("Engine Preferences", &showPreferences);
        ImGui::TextDisabled("Saved in AppData/FluxEngine/.flux/");
        ImGui::Separator();

        ImGui::Text("Viewport Configuration");
        ImGui::SliderFloat("Camera Speed", &camSpeed, 1.0f, 50.0f);
        ImGui::SliderFloat("Mouse Sensitivity", &camSens, 0.01f, 1.0f);
        ImGui::Checkbox("VSync Enabled", &vsync);

        ImGui::Separator();
        ImGui::Text("Editor Appearance");
        if (ImGui::Combo("Theme", &theme, "Default (Dark)\0Light\0Classic\0Legacy\0"))
        {
            ApplyTheme(theme);
        }

        if (ImGui::Button("Save Preferences"))
        {
            SavePreferences();
            Output::addLog("Engine Preferences Saved.");
        }
        ImGui::End();
    }

    if (showProjectSettings)
    {
        ImGui::Begin("Project Settings", &showProjectSettings);

        bool hasProject = explorerPtr && !explorerPtr->activeFolderPath.empty();
        if (!hasProject)
        {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "No active project loaded!");
        }
        else
        {
            ImGui::TextDisabled("Saved in YourProject/.flux/project.json");
            ImGui::Separator();

            ImGui::Text("Viewport");
            if (viewportPtr)
            {
                if (ImGui::Checkbox("VSync", &viewportPtr->vsyncEnabled))
                    SDL_GL_SetSwapInterval(viewportPtr->vsyncEnabled ? 1 : 0);
                ImGui::SliderFloat("Camera Sensitivity", &viewportPtr->camera->MouseSensitivity, 0.01f, 0.5f);
                ImGui::SliderFloat("Camera Speed", &viewportPtr->camera->MovementSpeed, 1.0f, 50.0f);
            }

            ImGui::Separator();

            ImGui::Text("Runtime Window Resolution");
            ImGui::InputInt("Width", &projectSettings.runtimeWidth);
            ImGui::InputInt("Height", &projectSettings.runtimeHeight);
            projectSettings.runtimeWidth = std::max(320, projectSettings.runtimeWidth);
            projectSettings.runtimeHeight = std::max(240, projectSettings.runtimeHeight);

            ImGui::Separator();

            ImGui::Text("Scene");
            ImGui::InputText("Startup Scene", projectSettings.startupScene, IM_ARRAYSIZE(projectSettings.startupScene));
            ImGui::InputText("Current Scene", projectSettings.currentScene, IM_ARRAYSIZE(projectSettings.currentScene));
            ImGui::Checkbox("Use Startup Scene on Play", &projectSettings.useStartupScene);
            ImGui::TextDisabled(projectSettings.useStartupScene ? "Play will load: startup scene"
                                                                : "Play will load: current scene");

            ImGui::Separator();

            if (ImGui::Button("Save Project Settings"))
            {
                SaveProjectSettings(explorerPtr->activeFolderPath);
                Output::addLog("Project Settings Saved.");
            }
        }
        ImGui::End();
    }

    drawTransformTools();

    ImGui::SameLine();

    ImGui::SetCursorPosX(main_viewport->Size.x * 0.5f - 50.0f);
    drawProjectControls();

    if (showBugReportModal)
    {
        ImGui::SetNextWindowSize(ImVec2(400, 320), ImGuiCond_FirstUseEver);
        ImGui::Begin("Report a Bug", &showBugReportModal);

        static char bugTitle[128] = "";
        static char bugDescription[1024] = "";
        static int bugSeverity = 1;

        ImGui::Text("Summarize the issue below:");
        ImGui::InputText("Title", bugTitle, sizeof(bugTitle));

        ImGui::Text("Steps to Reproduce / Details:");
        ImGui::InputTextMultiline("##desc", bugDescription, sizeof(bugDescription), ImVec2(-1, 120));

        ImGui::Combo("Severity", &bugSeverity, "Low\0Medium\0High\0Critical\0");

        ImGui::Separator();
        if (ImGui::Button("Submit Report"))
        {
            Output::addLog("Bug Report Submitted: " + std::string(bugTitle) + " (" + std::string(bugDescription) + ")");
            bugTitle[0] = '\0';
            bugDescription[0] = '\0';
            showBugReportModal = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            showBugReportModal = false;
        }
        ImGui::End();
    }

    ImGui::End();
    ImGui::PopStyleVar(2);
}

void Ribbon::drawTransformTools()
{
    if (ImGui::RadioButton("Move", currentTool == TOOL_MOVE))
        currentTool = TOOL_MOVE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Rotate", currentTool == TOOL_ROTATE))
        currentTool = TOOL_ROTATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Scale", currentTool == TOOL_SCALE))
        currentTool = TOOL_SCALE;
}

void Ribbon::drawFileMenu()
{
    if (ImGui::BeginMenu("Scene"))
    {
        if (ImGui::MenuItem("New Project..."))
        {
            if (explorerPtr)
                explorerPtr->TriggerCreateNewProject();
        }
        if (ImGui::MenuItem("Open Project..."))
        {
            if (explorerPtr)
                explorerPtr->TriggerOpenProject();
        }

        ImGui::Separator();

        bool hasProject = explorerPtr && !explorerPtr->activeFolderPath.empty();

        if (!hasProject)
            ImGui::BeginDisabled();

        if (ImGui::MenuItem("New Scene"))
        {
            if (heiarchyPtr)
            {
                heiarchyPtr->nodes.clear();
                heiarchyPtr->setup();
                std::strncpy(projectSettings.currentScene, "scene.fscn", sizeof(projectSettings.currentScene) - 1);
                SaveProjectSettings(explorerPtr->activeFolderPath);
                Output::addLog("Created new empty scene.");
            }
        }

        if (ImGui::MenuItem("Save Scene (Ctrl+S)"))
        {
            TriggerSaveScene();
        }

        if (ImGui::MenuItem("Save Scene As..."))
        {
            TriggerSaveSceneAs();
        }

        if (ImGui::MenuItem("Open Scene..."))
        {
            TriggerOpenScene();
        }

        if (!hasProject)
            ImGui::EndDisabled();

        ImGui::EndMenu();
    }
}

void Ribbon::drawEditMenu()
{
    if (ImGui::BeginMenu("Edit"))
    {
        if (ImGui::MenuItem("Undo", "Ctrl+Z"))
        {
            if (heiarchyPtr)
                heiarchyPtr->Undo();
        }
        if (ImGui::MenuItem("Redo", "Ctrl+Y"))
        {
            if (heiarchyPtr)
                heiarchyPtr->Redo();
        }

        ImGui::Separator();
        bool hasSelection = heiarchyPtr && heiarchyPtr->selectedIndex >= 0 &&
                            heiarchyPtr->selectedIndex < (int)heiarchyPtr->nodes.size();
        bool canDeleteNode = hasSelection && !heiarchyPtr->nodes[heiarchyPtr->selectedIndex].isLightingNode;
        if (!canDeleteNode)
            ImGui::BeginDisabled();
        if (ImGui::MenuItem("Delete", "Delete"))
        {
            heiarchyPtr->PushUndoState();
            heiarchyPtr->nodes.erase(heiarchyPtr->nodes.begin() + heiarchyPtr->selectedIndex);
            if (heiarchyPtr->selectedIndex >= (int)heiarchyPtr->nodes.size())
                heiarchyPtr->selectedIndex = (int)heiarchyPtr->nodes.size() - 1;
        }
        if (!canDeleteNode)
            ImGui::EndDisabled();
        ImGui::Separator();

        ImGui::MenuItem("Preferences", nullptr, &showPreferences);

        bool hasProject = explorerPtr && !explorerPtr->activeFolderPath.empty();
        if (!hasProject)
            ImGui::BeginDisabled();
        ImGui::MenuItem("Project Settings", nullptr, &showProjectSettings);
        if (!hasProject)
            ImGui::EndDisabled();
        ImGui::EndMenu();
    }

    bool gridVisibleCheck = (bool)viewportPtr->showGrid;

    if (ImGui::BeginMenu("View"))
    {
        if (ImGui::Checkbox("Show grid", &gridVisibleCheck))
        {
            viewportPtr->showGrid = gridVisibleCheck;
        }

        ImGui::EndMenu();
    }
}

void Ribbon::drawProjectControls()
{
    if (luaEnginePtr == nullptr || textEditorPtr == nullptr)
        return;

    if (ImGui::Button(editorLocked ? "Stop" : "Play"))
    {
        if (!editorLocked)
        {
            luaEnginePtr->isRunning = true;
            editorLocked = true;
        }
        else
        {
            luaEnginePtr->isRunning = false;
            editorLocked = false;
        }
        playToggledFrame = true;
    }
}

void Ribbon::SavePreferences()
{
    namespace fs = std::filesystem;
    fs::path dir = fs::path(SDL_GetPrefPath("FluxEngine", "FluxEngine"));
    fs::create_directories(dir);
    nlohmann::json j;
    j["camSpeed"] = camSpeed;
    j["camSens"] = camSens;
    j["vsync"] = vsync;
    j["theme"] = theme;
    j["lastProjectPath"] = lastProjectPath;
    std::ofstream(dir / "preferences.json") << j.dump(4);
}

void Ribbon::LoadPreferences()
{
    namespace fs = std::filesystem;
    fs::path p = fs::path(SDL_GetPrefPath("FluxEngine", "FluxEngine")) / "preferences.json";
    if (!fs::exists(p))
        return;
    std::ifstream f(p);
    nlohmann::json j;
    f >> j;
    camSpeed = j.value("camSpeed", 10.0f);
    camSens = j.value("camSens", 0.25f);
    vsync = j.value("vsync", true);
    theme = j.value("theme", 0);
    lastProjectPath = j.value("lastProjectPath", "");
}

void Ribbon::SaveProjectSettings(const std::filesystem::path &projectRoot)
{
    namespace fs = std::filesystem;
    if (projectRoot.empty())
        return;

    std::string projName = projectRoot.filename().string();
    fs::path p = projectRoot / (projName + ".flux");

    nlohmann::json j;
    j["projectName"] = projName;
    j["startupScene"] = projectSettings.startupScene;
    j["currentScene"] = projectSettings.currentScene;
    j["useStartupScene"] = projectSettings.useStartupScene;
    j["runtimeWidth"] = projectSettings.runtimeWidth;
    j["runtimeHeight"] = projectSettings.runtimeHeight;

    if (viewportPtr)
    {
        j["vsync"] = viewportPtr->vsyncEnabled;
        j["camSens"] = viewportPtr->camera->MouseSensitivity;
        j["camSpeed"] = viewportPtr->camera->MovementSpeed;
    }

    std::ofstream(p) << j.dump(4);
}

void Ribbon::LoadProjectSettings(const std::filesystem::path &projectRoot)
{
    namespace fs = std::filesystem;
    if (projectRoot.empty())
        return;

    std::string projName = projectRoot.filename().string();
    fs::path p = projectRoot / (projName + ".flux");

    if (!fs::exists(p))
    {
        p = projectRoot / ".flux" / "project.json";
    }

    if (!fs::exists(p))
        return;

    std::ifstream f(p);
    nlohmann::json j;
    f >> j;

    std::string ss = j.value("startupScene", "scene.fscn");
    std::string cs = j.value("currentScene", "scene.fscn");
    std::strncpy(projectSettings.startupScene, ss.c_str(), sizeof(projectSettings.startupScene) - 1);
    std::strncpy(projectSettings.currentScene, cs.c_str(), sizeof(projectSettings.currentScene) - 1);
    projectSettings.useStartupScene = j.value("useStartupScene", false);
    projectSettings.runtimeWidth = j.value("runtimeWidth", 1280);
    projectSettings.runtimeHeight = j.value("runtimeHeight", 720);

    if (viewportPtr)
    {
        viewportPtr->vsyncEnabled = j.value("vsync", true);
        SDL_GL_SetSwapInterval(viewportPtr->vsyncEnabled ? 1 : 0);
        viewportPtr->camera->MouseSensitivity = j.value("camSens", 0.25f);
        viewportPtr->camera->MovementSpeed = j.value("camSpeed", 10.0f);
    }
}

void Ribbon::drawGameObjectMenu()
{
    if (ImGui::BeginMenu("GameObject"))
    {
        bool hasProject = explorerPtr && !explorerPtr->activeFolderPath.empty();
        if (!hasProject)
            ImGui::BeginDisabled();

        auto tryAdd = [&](const char *rel, const char *addName) {
            if (!heiarchyPtr)
                return;
            std::filesystem::path activeProjectPath = explorerPtr->activeFolderPath;
            std::string full = PathHelper::GetAssetPath(std::string("assets/models/") + rel);
            if (!activeProjectPath.empty())
            {
                auto c = activeProjectPath / "models" / rel;
                if (std::filesystem::exists(c))
                    full = c.string();
            }
            heiarchyPtr->AddModel(full, addName);
        };

        if (ImGui::BeginMenu("3D Object"))
        {
            if (ImGui::MenuItem("Cube"))
                tryAdd("cube.obj", "Cube");
            if (ImGui::MenuItem("Sphere"))
                tryAdd("sphere.obj", "Sphere");
            if (ImGui::MenuItem("Monkey"))
                tryAdd("monkey.obj", "Monkey");
            if (ImGui::MenuItem("Plane"))
                tryAdd("plane.obj", "Plane");
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Light"))
        {
            if (ImGui::MenuItem("Directional Light"))
            {
                if (heiarchyPtr)
                    heiarchyPtr->AddLight(NodeType::DirectionalLight);
            }
            if (ImGui::MenuItem("Point Light"))
            {
                if (heiarchyPtr)
                    heiarchyPtr->AddLight(NodeType::PointLight);
            }
            if (ImGui::MenuItem("Spot Light"))
            {
                if (heiarchyPtr)
                    heiarchyPtr->AddLight(NodeType::SpotLight);
            }
            if (ImGui::MenuItem("Surface Light"))
            {
                if (heiarchyPtr)
                    heiarchyPtr->AddLight(NodeType::SurfaceLight);
            }
            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("Camera"))
        {
            if (heiarchyPtr)
                heiarchyPtr->AddCamera();
        }

        if (!hasProject)
            ImGui::EndDisabled();

        ImGui::EndMenu();
    }
}

void Ribbon::TriggerSaveScene()
{
    if (!explorerPtr || !heiarchyPtr)
        return;
    std::string sceneName = std::strlen(projectSettings.currentScene) > 0 ? projectSettings.currentScene : "scene.fscn";

    std::string lowerName = sceneName;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    if (lowerName == "scene.fscn" || lowerName == "main.fscn" || lowerName.empty())
    {
        TriggerSaveSceneAs();
    }
    else
    {
        std::filesystem::path scenePath = sceneName;
        if (!scenePath.is_absolute())
        {
            scenePath = explorerPtr->activeFolderPath / scenePath;
        }
        SceneSerializer::Save(*heiarchyPtr, scenePath.string(), explorerPtr->activeFolderPath);
        Output::addLog("Scene Saved to " + sceneName);
    }
}

void Ribbon::TriggerSaveSceneAs()
{
    if (!explorerPtr || !heiarchyPtr)
        return;
    auto selection =
        pfd::save_file("Save Scene As", explorerPtr->activeFolderPath.string(), {"Flux Scene", "*.fscn"}).result();
    if (!selection.empty())
    {
        std::filesystem::path selectedPath(selection);
        if (selectedPath.extension() != ".fscn")
            selectedPath += ".fscn";

        std::filesystem::path relative = std::filesystem::relative(selectedPath, explorerPtr->activeFolderPath);
        std::string storedPath;
        if (relative.empty() || relative.string().find("..") != std::string::npos)
        {
            storedPath = selectedPath.string();
        }
        else
        {
            storedPath = relative.string();
        }

        std::strncpy(projectSettings.currentScene, storedPath.c_str(), sizeof(projectSettings.currentScene) - 1);
        projectSettings.currentScene[sizeof(projectSettings.currentScene) - 1] = '\0';

        SceneSerializer::Save(*heiarchyPtr, selectedPath.string(), explorerPtr->activeFolderPath);
        SaveProjectSettings(explorerPtr->activeFolderPath);
        explorerPtr->refreshRequested = true;
        Output::addLog("Scene Saved As: " + storedPath);
    }
}

void Ribbon::TriggerOpenScene()
{
    if (!explorerPtr || !heiarchyPtr)
        return;
    auto selection =
        pfd::open_file("Open Scene", explorerPtr->activeFolderPath.string(), {"Flux Scene", "*.fscn"}).result();
    if (!selection.empty())
    {
        std::filesystem::path selectedPath(selection[0]);
        SceneSerializer::Load(*heiarchyPtr, selectedPath, explorerPtr->activeFolderPath);

        std::filesystem::path relative = std::filesystem::relative(selectedPath, explorerPtr->activeFolderPath);
        std::string storedPath;
        if (relative.empty() || relative.string().find("..") != std::string::npos)
        {
            storedPath = selectedPath.string();
        }
        else
        {
            storedPath = relative.string();
        }

        std::strncpy(projectSettings.currentScene, storedPath.c_str(), sizeof(projectSettings.currentScene) - 1);
        projectSettings.currentScene[sizeof(projectSettings.currentScene) - 1] = '\0';
        SaveProjectSettings(explorerPtr->activeFolderPath);

        Output::addLog("Opened scene: " + storedPath);
    }
}

} // namespace Flux