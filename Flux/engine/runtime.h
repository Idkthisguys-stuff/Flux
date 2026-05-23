#pragma once

#include "3DRenderer.h"
#include "luaEngine.h"
#include "mechanics/Scenenode.h"
#include <SDL3/SDL.h>
#include <string>
#include <vector>


#include "Textureloader.h"
#include "gui/viewport/ribbon.h"

#include "mechanics/SplashScreen.h"

namespace Flux
{
class Runtime
{
  public:
    void Start(const std::string &projectName, const std::filesystem::path &projectPath,
               std::vector<SceneNode> &copiedNodes, int windowWidth = 1280, int windowHeight = 720);
    void Update();
    void Stop();
    void SyncCamera(glm::vec3 editorPos, glm::vec3 editorTarget);

    bool isRunning = false;

  private:
    SDL_Window *m_window = nullptr;
    SDL_GLContext m_glContext;
    TextureLoader m_textureLoader;
    Renderer3D m_renderer;
    LuaEngine m_luaEngine;

    Ribbon m_ribbon;
    std::vector<SceneNode> m_gameNodes;

    glm::vec3 cameraPos{0.0f, 5.0f, 15.0f};
    glm::vec3 cameraTarget{0.0f, 0.0f, 0.0f};

    std::unordered_map<std::string, unsigned int> m_runtimeTextureCache;\

    uint64_t lastTimeFrame = 0;
};
} // namespace Flux