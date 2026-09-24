#pragma once

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include <SDL3/SDL.h> // removed glad.h, imgui now renders through SDL_GPU instead of raw GL
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
#include "imgui_impl_sdlgpu3.h"
#include "stb_image.h"
#include "logic/SplashScreen.h"
#include "logic/runtime.h"
#include "core/SceneSerializer.h"
#include "render/3D/OpenGL/OpenGLManager.h"

#include <dwmapi.h>

#define DEBUG_MODE true // Internal debugger switch (I was just testing preprocessor stuff actually :/)

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

        SDL_Window* m_window = nullptr;
        SDL_GPUDevice* m_gpuDevice = nullptr;
        bool m_shouldClose = false;

        SDL_FColor m_clearColor = { 0.0f, 0.0f, 0.0f, 1.0f }; // clear() now just stashes the color for the render pass below

        int m_width, m_height;
        std::string m_title;

        Viewport m_viewport;
        Assets m_explorer;
        Ribbon m_ribbon;
        Output m_output;
        Properties m_properties;
        Heiarchy m_heiarchy;
        TextEditor m_texteditor;
        LuaEngine m_luaEngine;
        Runtime m_runtime;

        SceneSerializer m_sceneSerializer;

        void StartRuntimeEngine();
        void StopRuntimeEngine();

        bool m_stoppingRuntime = false;
    };
}