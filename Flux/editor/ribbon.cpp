#include "ribbon.h"
#include "imgui.h"
#include <SDL3/SDL.h>
#include <iostream>

namespace Flux
{
int currentTool = 0;
bool showSettings = false;

void Ribbon::renderRibbon()
{
    ImGuiViewport *main_viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(main_viewport->Pos);
    ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x, 55));

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDocking |
                                    ImGuiWindowFlags_MenuBar;

    ImGui::Begin("###Ribbon", nullptr, window_flags);
    if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive())
    {
        ImGui::SetWindowFocus();
    }

    if (ImGui::BeginMenuBar())
    {
        drawFileMenu();
        drawEditMenu();

        ImGui::Separator();

        drawTransformTools();

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
        if (ImGui::Combo("Theme", &theme, "Dark Mode\0Light Mode\0Classic\0"))
        {
            if (theme == 0)
                ImGui::StyleColorsDark();
            if (theme == 1)
                ImGui::StyleColorsLight();
            if (theme == 2)
                ImGui::StyleColorsClassic();
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
            projectSettings.runtimeWidth  = std::max(320, projectSettings.runtimeWidth);
            projectSettings.runtimeHeight = std::max(240, projectSettings.runtimeHeight);

            ImGui::Separator();

            ImGui::Text("Scene");
            ImGui::InputText("Startup Scene", projectSettings.startupScene, IM_ARRAYSIZE(projectSettings.startupScene));
            ImGui::InputText("Current Scene", projectSettings.currentScene,  IM_ARRAYSIZE(projectSettings.currentScene));
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

    ImGui::SetCursorPosX(main_viewport->Size.x * 0.5f - 50.0f);
    drawProjectControls();

    ImGui::End();
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
    if (ImGui::BeginMenu("File"))
    {
        bool hasProject = explorerPtr && !explorerPtr->activeFolderPath.empty();

        if (!hasProject)
            ImGui::BeginDisabled();

        if (ImGui::MenuItem("New Scene"))
        {
            heiarchyPtr->nodes.clear();
            heiarchyPtr->setup();
            Output::addLog("Created new empty scene.");
        }

        if (ImGui::MenuItem("Save Scene (Ctrl+S)"))
        {
            std::string savePath = explorerPtr->activeFolderPath.string() + "/main.fscn";
            SceneSerializer::Save(*heiarchyPtr, savePath, explorerPtr->activeFolderPath);
            Output::addLog("Scene Saved.");
        }

        if (ImGui::MenuItem("Open Scene..."))
        {
            auto selection =
                pfd::open_file("Open Scene", explorerPtr->activeFolderPath.string(), {"Flux Scene", "*.fscn"}).result();
            if (!selection.empty())
            {
                SceneSerializer::Load(*heiarchyPtr, selection[0], explorerPtr->activeFolderPath);
                Output::addLog("Opened scene.");
            }
        }

        if (ImGui::BeginMenu("Import 3D Model...")) {
            if (ImGui::MenuItem(".obj")) {
            }

            if (ImGui::MenuItem(".fbx")) {
            }

            ImGui::EndMenu();
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
        if (ImGui::MenuItem("Undo"))
        {
            if (textEditorPtr)
                textEditorPtr->Undo();
        }
        if (ImGui::MenuItem("Redo"))
        {
            if (textEditorPtr)
                textEditorPtr->Redo();
        }
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

    if (ImGui::BeginMenu("View")) {
        if (ImGui::Checkbox("Show grid", &gridVisibleCheck)) {
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
    j["camSens"]  = camSens;
    j["vsync"]    = vsync;
    j["theme"]    = theme;
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
    camSens  = j.value("camSens",  0.25f);
    vsync    = j.value("vsync",    true);
    theme    = j.value("theme",    0);
}

void Ribbon::SaveProjectSettings(const std::filesystem::path &projectRoot)
{
    namespace fs = std::filesystem;
    fs::path dir = projectRoot / ".flux";
    fs::create_directories(dir);

    nlohmann::json j;
    j["startupScene"]     = projectSettings.startupScene;
    j["currentScene"]     = projectSettings.currentScene;
    j["useStartupScene"]  = projectSettings.useStartupScene;
    j["runtimeWidth"]     = projectSettings.runtimeWidth;
    j["runtimeHeight"]    = projectSettings.runtimeHeight;

    if (viewportPtr)
    {
        j["vsync"]    = viewportPtr->vsyncEnabled;
        j["camSens"]  = viewportPtr->camera->MouseSensitivity;
        j["camSpeed"] = viewportPtr->camera->MovementSpeed;
    }

    std::ofstream(dir / "project.json") << j.dump(4);
}

void Ribbon::LoadProjectSettings(const std::filesystem::path &projectRoot)
{
    namespace fs = std::filesystem;
    fs::path p = projectRoot / ".flux" / "project.json";
    if (!fs::exists(p))
        return;
    std::ifstream f(p);
    nlohmann::json j;
    f >> j;

    std::string ss = j.value("startupScene", "main.fscn");
    std::string cs = j.value("currentScene",  "main.fscn");
    std::strncpy(projectSettings.startupScene, ss.c_str(), sizeof(projectSettings.startupScene) - 1);
    std::strncpy(projectSettings.currentScene, cs.c_str(), sizeof(projectSettings.currentScene) - 1);
    projectSettings.useStartupScene = j.value("useStartupScene", false);
    projectSettings.runtimeWidth    = j.value("runtimeWidth",    1280);
    projectSettings.runtimeHeight   = j.value("runtimeHeight",   720);

    if (viewportPtr)
    {
        viewportPtr->vsyncEnabled             = j.value("vsync",    true);
        SDL_GL_SetSwapInterval(viewportPtr->vsyncEnabled ? 1 : 0);
        viewportPtr->camera->MouseSensitivity = j.value("camSens",  0.25f);
        viewportPtr->camera->MovementSpeed    = j.value("camSpeed", 10.0f);
    }
}

} // namespace Flux