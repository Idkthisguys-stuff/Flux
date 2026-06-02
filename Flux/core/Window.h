#pragma once

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include <glad/glad.h>
#include <SDL3/SDL.h>
#include <string>
#include <filesystem>
#include "editor/viewport.h"
#include "editor/explorer.h"
#include "editor/ribbon.h"
#include "editor/output.h"
#include "editor/properties.h"
#include "editor/heiarchy.h"
#include "editor/texteditor.h"
#include "scripting/luaEngine.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"
#include "stb_image.h"
#include "logic/SplashScreen.h"
#include "logic/runtime.h"
#include "core/SceneSerializer.h"

#include <dwmapi.h>

namespace Flux {
    class Window {
    public:
        Window(int width, int height, const std::string& title);
        ~Window();

        bool shouldClose() const;
        void update();
        void clear(float r, float g, float b, float a);

        SDL_Window* getNativeWindow() const { return m_window; }

        bool m_pendingStop = false;
		bool m_pendingStart = false;

    private:
        std::vector<SceneNode> m_runtimeNodes;

        SDL_Window*   m_window    = nullptr;
        SDL_GLContext m_glContext = nullptr;
        bool          m_shouldClose = false;

        int         m_width, m_height;
        std::string m_title;

        Viewport   m_viewport;
        Explorer   m_explorer;
        Ribbon     m_ribbon;
        Output     m_output;
        Properties m_properties;
        Heiarchy   m_heiarchy;
        TextEditor m_texteditor;
        LuaEngine  m_luaEngine;
        Runtime    m_runtime;

        SceneSerializer m_sceneSerializer;

        void StartRuntimeEngine();
        void StopRuntimeEngine();

        bool m_stoppingRuntime = false;
    };
}