#pragma once

extern "C" {
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
}

#define SOL_ALL_SAFETIES_ON 1
#include "editor/output.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sol/sol.hpp>
#include <string>

#include <logic/data/Scenenode.h>

#include <SDL3/SDL.h>

namespace Flux
{
class Output;

class LuaEngine
{
  public:
    LuaEngine() = default;
    ~LuaEngine() = default;

    bool isRunning = false;

    void init();
    void step();

    void start();
    void stop();

    void runScript(const std::string &code);
    void runAllScriptsInFolder(const std::string &folderPath);

    std::vector<SceneNode> *activeNodes = nullptr;

    void bindEngineAPI();

  private:
    sol::state lua;

    // luaOnStart / luaOnUpdate / luaOnEnd were declared here but never
    // assigned by any code path (scripts are registered via
    // runScript -> m_startFuncs / m_updateFuncs instead).
    // Calling luaOnEnd.valid() on an unassigned sol::protected_function
    // internally dereferences a null lua_State pointer -> crash.
    // Removed entirely; per-script callbacks live in the vectors below.

    std::vector<sol::protected_function> m_startFuncs;
    std::vector<sol::protected_function> m_updateFuncs;
    std::vector<sol::environment> m_environments;

    struct DelayedTask
    {
        int handle;
        float remaining; // seconds left
        sol::protected_function fn;
        bool cancelled = false;
    };

    struct SpawnedTask
    {
        int handle;
        sol::thread runner;
        sol::coroutine co;
        bool cancelled = false;
    };

    std::vector<DelayedTask> m_delayedTasks;
    std::vector<SpawnedTask> m_spawnedTasks;
    int m_nextTaskHandle = 0;
};
} // namespace Flux