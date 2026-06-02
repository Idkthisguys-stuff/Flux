#pragma once

#include "render/3D/OpenGL/OpenGLRenderer.h"
#include "scripting/luaEngine.h"
#include "data/Scenenode.h"
#include <SDL3/SDL.h>
#include <string>
#include <vector>


#include "logic/Textureloader.h"
#include "editor/ribbon.h"

#include "SplashScreen.h"

#include "logic/PhysicsLayer.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>

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

    Flux::BroadPhaseLayerInterfaceImpl m_broadPhaseLayerInterface;
    Flux::ObjectLayerPairFilterImpl m_objectLayerPairFilter;
    Flux::ObjectVsBroadPhaseLayerFilterImpl m_objectVsBroadPhaseLayerFilter;

    JPH::PhysicsSystem* m_PhysicsSystem = nullptr;

    JPH::TempAllocatorImpl* m_tempAllocator = nullptr;
    JPH::JobSystemThreadPool* m_jobSystem = nullptr;
};
} // namespace Flux