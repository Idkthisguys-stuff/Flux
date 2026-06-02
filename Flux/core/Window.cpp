#include "Window.h"
#include <iostream>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"
#include "imgui_internal.h"
#include <SDL3/SDL.h>

#include <ShObjIdl.h>
#include <propkey.h>

namespace Flux
{
Window::Window(int width, int height, const std::string &title) : m_width(width), m_height(height), m_title(title)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "ERROR: FAILED TO INITIALIZE SDL3: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    

    m_window = SDL_CreateWindow(m_title.c_str(), m_width, m_height,
                                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);

    if (!m_window)
    {
        std::cerr << "FATAL: SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        throw std::runtime_error("SDL_CreateWindow failed");
    }

    if (m_window != nullptr)
    {
        SDL_StartTextInput(m_window);
        std::cout << "SUCCESSFULLY STARTED TEXT INPUT" << std::endl;
    }

    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext)
    {
        std::cerr << "FATAL: SDL_GL_CreateContext failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        throw std::runtime_error("SDL_GL_CreateContext failed");
    }

    SDL_GL_MakeCurrent(m_window, m_glContext);
    SDL_GL_SetSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        std::cerr << "ERROR: FAILED TO INITIALIZE GLAD" << std::endl;
    }

    {
        int iconW, iconH, iconCh;
        unsigned char *pixels = stbi_load("assets/icon.png", &iconW, &iconH, &iconCh, 4);
        if (pixels)
        {
            SDL_Surface *icon = SDL_CreateSurfaceFrom(iconW, iconH, SDL_PIXELFORMAT_RGBA32, pixels, iconW * 4);
            if (icon)
            {
                SDL_SetWindowIcon(m_window, icon);
                SDL_DestroySurface(icon);
            }
            stbi_image_free(pixels);
        }
    }

#if defined(_WIN32)
    {
        SetCurrentProcessExplicitAppUserModelID(L"IdkthisguysStuff.FluxEngine.Core");

        SDL_PropertiesID props = SDL_GetWindowProperties(m_window);
        HWND hwnd = (HWND)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
        if (hwnd)
        {
            BOOL useDarkMode = TRUE;
            DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkMode, sizeof(useDarkMode));
            
            IPropertyStore* pPropStore = nullptr;
            if (SUCCEEDED(SHGetPropertyStoreForWindow(hwnd, IID_PPV_ARGS(&pPropStore)))) {
                PROPVARIANT propvar;
                PropVariantInit(&propvar);
                propvar.vt = VT_LPWSTR;
                propvar.pwszVal = const_cast<wchar_t*>(L"ZeroPointStudio.FluxEngine.Core");
                
                pPropStore->SetValue(PKEY_AppUserModel_ID, propvar);
                pPropStore->Commit();
                pPropStore->Release();
            }
        }

        HWND consoleHwnd = GetConsoleWindow();
        if (consoleHwnd) {
            IPropertyStore* pConsoleStore = nullptr;
            if (SUCCEEDED(SHGetPropertyStoreForWindow(consoleHwnd, IID_PPV_ARGS(&pConsoleStore)))) {
                PROPVARIANT propvar;
                PropVariantInit(&propvar);
                propvar.vt = VT_LPWSTR;
                propvar.pwszVal = const_cast<wchar_t*>(L"IdkthisguysStuff.FluxEngine.Core");
                
                pConsoleStore->SetValue(PKEY_AppUserModel_ID, propvar);
                pConsoleStore->Commit();
                pConsoleStore->Release();
            }
        }
    }
#endif

    m_explorer.textEditor = &m_texteditor;
    m_explorer.ribbonPtr = &m_ribbon;

    m_ribbon.luaEnginePtr = &m_luaEngine;
    m_ribbon.textEditorPtr = &m_texteditor;
    m_ribbon.explorerPtr = &m_explorer;
    m_ribbon.heiarchyPtr = &m_heiarchy;
    m_ribbon.viewportPtr = &m_viewport;
    m_ribbon.LoadPreferences();
    m_texteditor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());

    m_viewport.ribbonPtr = &m_ribbon;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplSDL3_InitForOpenGL(m_window, m_glContext);
    ImGui_ImplOpenGL3_Init("#version 410");

    m_viewport.Init();
    m_heiarchy.setup();
    m_luaEngine.init();
}

Window::~Window()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    if (m_glContext)
        SDL_GL_DestroyContext(m_glContext);
    if (m_window)
        SDL_DestroyWindow(m_window);
    SDL_Quit();
}

bool Window::shouldClose() const
{
    return m_window == nullptr || m_shouldClose;
}

void Window::update()
{
    ImGuiIO &io = ImGui::GetIO();

    if (io.WantTextInput)
    {

        SDL_StartTextInput(m_window);
    }
    if (m_window == nullptr)
        return;

    SDL_GL_MakeCurrent(m_window, m_glContext);

    if (m_pendingStart)
    {
        m_pendingStart = false;
        StartRuntimeEngine();

        if (!SDL_GL_MakeCurrent(m_window, m_glContext))
        {
            Output::addLog("RUNTIME ERROR: Main loop context re-binding failed: " + std::string(SDL_GetError()));
        }
    }
    if (m_pendingStop)
    {
        m_pendingStop = false;
        StopRuntimeEngine();

        SDL_GL_MakeCurrent(m_window, m_glContext);
    }

    Uint32 editorWindowID = SDL_GetWindowID(m_window);
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {

        ImGui_ImplSDL3_ProcessEvent(&event);

        if (event.type == SDL_EVENT_KEY_DOWN)
        {

            if (event.key.key == SDLK_ESCAPE)
            {
                m_shouldClose = true;
            }

            if (io.WantCaptureKeyboard)
            {

                continue;
            }
        }

        bool isEditorWindowEvent = (event.type < SDL_EVENT_WINDOW_FIRST || event.type > SDL_EVENT_WINDOW_LAST) ||
                                   (event.window.windowID == editorWindowID);

        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            m_shouldClose = true;
            break;

        /*case SDL_EVENT_TEXT_INPUT:

            ImGui::GetIO().AddInputCharactersUTF8(event.text.text);
            break;
        */
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            if (event.window.windowID == editorWindowID)
            {
                m_shouldClose = true;
            }
            else if (m_runtime.isRunning)
            {
                m_pendingStop = true;
            }
            break;
        }
    }

    static bool lastState = false;
    if (io.WantCaptureKeyboard != lastState)
    {

        lastState = io.WantCaptureKeyboard;
        std::cout << "[TRAP 2] ImGui Keyboard Capture Changed! Focus State: " << lastState << std::endl;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGuiViewport *viewport = ImGui::GetMainViewport();

    ImVec2 dockPos = viewport->Pos;
    dockPos.y += 55.0f;
    ImVec2 dockSize = viewport->Size;
    dockSize.y -= 55.0f;

    static bool firstTime = true;
    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");

    ImGui::SetNextWindowPos(dockPos);
    ImGui::SetNextWindowSize(dockSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags host_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                  ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                  ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
                                  ImGuiWindowFlags_NoBackground;

    ImGui::Begin("MainDockHost", nullptr, host_flags);
    ImGui::DockSpace(dockspace_id, ImVec2(0.f, 0.f), ImGuiDockNodeFlags_None);

    if (firstTime)
    {
        firstTime = false;

        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, dockSize);

        ImGuiID dock_id_left, dock_id_right, dock_id_bottom, dock_id_bottomRight, dock_id_bottomLeft, dock_id_center = dockspace_id;

        dock_id_bottom = ImGui::DockBuilderSplitNode(dock_id_center, ImGuiDir_Down, 0.25f, nullptr, &dock_id_center);
        dock_id_left = ImGui::DockBuilderSplitNode(dock_id_center, ImGuiDir_Left, 0.20f, nullptr, &dock_id_center);
        dock_id_right = ImGui::DockBuilderSplitNode(dock_id_center, ImGuiDir_Right, 0.30f, nullptr, &dock_id_center);
        dock_id_bottomRight = ImGui::DockBuilderSplitNode(dock_id_right, ImGuiDir_Down, 0.5f, nullptr, &dock_id_right);
        dock_id_bottomLeft = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.5f, nullptr, &dock_id_left);

        ImGui::DockBuilderDockWindow("Viewport", dock_id_center);
        ImGui::DockBuilderDockWindow("###UniqueEditorID", dock_id_center);
        ImGui::DockBuilderDockWindow("Explorer", dock_id_right);
        ImGui::DockBuilderDockWindow("Output", dock_id_bottom);
        ImGui::DockBuilderDockWindow("Properties", dock_id_bottomLeft);
        ImGui::DockBuilderDockWindow("Heiarchy", dock_id_left);
        ImGui::DockBuilderFinish(dockspace_id);
    }
    ImGui::End();

    if (m_luaEngine.isRunning)
        m_luaEngine.step();

    m_ribbon.renderRibbon();
    m_explorer.renderExplorer(m_viewport);

    static std::filesystem::path lastLoadedProject;
    if (m_explorer.activeFolderPath != lastLoadedProject && !m_explorer.activeFolderPath.empty())
    {
        lastLoadedProject = m_explorer.activeFolderPath;
        m_ribbon.LoadProjectSettings(m_explorer.activeFolderPath);
    }

    if (!m_explorer.activeFolderPath.empty())
    {
        if (m_ribbon.editorLocked)
            ImGui::BeginDisabled(true);

        m_viewport.RenderViewport(m_heiarchy);
        m_properties.renderProperties(&m_heiarchy);
        m_heiarchy.renderHeiarchy(m_viewport.activeProjectPath);

        if (m_ribbon.editorLocked)
            ImGui::EndDisabled();

        ImGuiIO &io = ImGui::GetIO();
    }
    else
    {
        ImGui::Begin("Viewport");
        ImGui::SetWindowFontScale(2.0f);
        ImGui::Text("No project loaded. Please Open or Create a project in the Explorer.");
        ImGui::SetWindowFontScale(1.0f);
        ImGui::End();
    }

    if (m_ribbon.playToggledFrame)
    {
        if (m_ribbon.editorLocked)
            m_pendingStart = true;
        else
            m_pendingStop = true;
        m_ribbon.playToggledFrame = false;
    }

    m_output.renderOutput();

    bool isTextEditorFocused = false;

    if (m_explorer.isEditorVisible)
    {
        std::string editorTitle = "Text Editor - " + m_explorer.activeScriptName;
        if (m_explorer.isEditorUnsaved)
            editorTitle += " *";
        editorTitle += "###UniqueEditorID";

        ImGui::Begin(editorTitle.c_str(), &m_explorer.isEditorVisible);

        isTextEditorFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

        if (isTextEditorFocused)
        {
            ImGuiIO &io = ImGui::GetIO();

            if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S))
            {
                if (!m_explorer.activeFilePath.empty())
                {
                    std::ofstream outFile(m_explorer.activeFilePath);
                    if (outFile.is_open())
                    {
                        outFile << m_texteditor.GetText();
                        outFile.close();
                        m_explorer.isEditorUnsaved = false;
                        Output::addLog("Script saved.");
                    }
                }
            }
        }
        m_texteditor.Render("CodeEditorWidget");
        ImGui::End();
    }
    else
    {
        SDL_StopTextInput(m_window);
    }

    if (!isTextEditorFocused && io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S) && !m_ribbon.editorLocked)
    {
        std::string scenePath = m_explorer.activeFolderPath.string() + "/scene.fscn";
        m_sceneSerializer.Save(m_heiarchy, scenePath, m_explorer.activeFolderPath);
        Output::addLog("Scene saved to " + scenePath);
    }

    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_MakeCurrent(m_window, m_glContext);
    SDL_GL_SwapWindow(m_window);

    //m_runtime.SyncCamera(m_viewport.camera->Position, m_viewport.camera->Position + m_viewport.camera->Front);
    m_runtime.Update();

    if (m_window && m_glContext)
        SDL_GL_MakeCurrent(m_window, m_glContext);
}

void Window::clear(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Window::StartRuntimeEngine()
{
    Output::addLog("Starting runtime engine...");

    std::string projName = m_explorer.projectRoot.name;
    if (projName == "Project" || projName.empty())
        projName = "Flux Game";

    auto &ps = m_ribbon.projectSettings;

    if (ps.useStartupScene)
    {
        std::filesystem::path scenePath = m_explorer.activeFolderPath / ps.startupScene;
        if (std::filesystem::exists(scenePath))
        {
            m_sceneSerializer.Load(m_heiarchy, scenePath, m_explorer.activeFolderPath);
            Output::addLog("Runtime loading startup scene: " + std::string(ps.startupScene));
        }
        else
        {
            Output::addLog("WARNING: Startup scene not found: " + scenePath.string() + ", using current hierarchy.");
        }
    }

    m_runtimeNodes = m_heiarchy.nodes;
    m_runtime.Start(projName, m_explorer.activeFolderPath, m_runtimeNodes, ps.runtimeWidth, ps.runtimeHeight);
}

void Window::StopRuntimeEngine()
{
    m_runtime.Stop();

    if (m_ribbon.luaEnginePtr)
        m_ribbon.luaEnginePtr->isRunning = false;

    m_ribbon.editorLocked = false;
    m_runtimeNodes.clear();

    Output::addLog("Runtime stopped.");
}
} // namespace Flux