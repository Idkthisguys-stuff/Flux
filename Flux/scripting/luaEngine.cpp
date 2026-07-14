#include "luaEngine.h"

namespace Flux
{

void LuaEngine::bindEngineAPI()
{
    lua.new_usertype<glm::vec3>(
        "Vector3", sol::constructors<glm::vec3(), glm::vec3(float), glm::vec3(float, float, float)>(), "x",
        &glm::vec3::x, "y", &glm::vec3::y, "z", &glm::vec3::z, sol::meta_function::addition,
        [](const glm::vec3 &a, const glm::vec3 &b) { return a + b; }, sol::meta_function::subtraction,
        [](const glm::vec3 &a, const glm::vec3 &b) { return a - b; }, sol::meta_function::multiplication,
        [](const glm::vec3 &a, float s) { return a * s; });

    sol::table color3Table = lua.create_table();
    color3Table["new"] = [](float r, float g, float b) -> glm::vec3 {
        return glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f);
    };
    lua["Color3"] = color3Table;

    lua.new_usertype<glm::vec4>("Color4", sol::constructors<glm::vec4(), glm::vec4(float, float, float, float)>(), "r",
                                &glm::vec4::x, "g", &glm::vec4::y, "b", &glm::vec4::z, "a", &glm::vec4::w);

    lua.new_usertype<SceneNode>(
        "SceneNode", "name", &SceneNode::name, "type", [](SceneNode &n) { return static_cast<int>(n.type); },
        "position",
        sol::property([](SceneNode &n) { return n.position; },
                      [](SceneNode &n, const glm::vec3 &v) { n.position = v; }),
        "rotation",
        sol::property([](SceneNode &n) { return n.rotation; },
                      [](SceneNode &n, const glm::vec3 &v) { n.rotation = v; }),
        "baseColor",
        sol::property([](SceneNode &n) { return n.baseColor; },
                      [](SceneNode &n, const glm::vec3 &v) { n.baseColor = v; }),
        "roughness", &SceneNode::roughness, "metallic", &SceneNode::metallic, "isAnchored", &SceneNode::isAnchored,
        "isLocked", &SceneNode::isLocked, "fov", &SceneNode::fov, "isMainCamera", &SceneNode::isMainCamera,

        "isIndependent", &SceneNode::isIndependent,

        "getWorldPosition",
        [this](SceneNode &n) -> glm::vec3 {
            glm::mat4 w = n.GetWorldTransform(*activeNodes);
            return glm::vec3(w[3]); // 4th column is position
        },

        "parent",
        sol::property(
            [this](SceneNode &n) -> SceneNode * {
                if (n.parentIndex != -1 && n.parentIndex < activeNodes->size())
                {
                    return &(*activeNodes)[n.parentIndex];
                }
                return nullptr;
            },
            [this](SceneNode &child, SceneNode *newParent) {
                if (!newParent)
                {
                    child.parentIndex = -1; // Unparent
                    return;
                }
                auto it = std::find_if(activeNodes->begin(), activeNodes->end(),
                                       [&](const SceneNode &node) { return &node == newParent; });
                if (it != activeNodes->end())
                {
                    child.parentIndex = std::distance(activeNodes->begin(), it);
                }
            }),

        "getChildren",
        [this](SceneNode &n) -> sol::table {
            sol::table childrenTable = lua.create_table();

            int currentIndex = -1;
            for (size_t i = 0; i < activeNodes->size(); i++)
            {
                if (&(*activeNodes)[i] == &n)
                {
                    currentIndex = (int)i;
                    break;
                }
            }

            int tIndex = 1;
            for (auto &node : *activeNodes)
            {
                if (node.parentIndex == currentIndex)
                {
                    childrenTable[tIndex++] = &node;
                }
            }
            return childrenTable;
        },

        "getAncestors",
        [this](SceneNode &n) -> sol::table {
            sol::table ancestorTable = lua.create_table();
            int tIndex = 1;
            int currentParent = n.parentIndex;

            while (currentParent != -1 && currentParent < activeNodes->size())
            {
                ancestorTable[tIndex++] = &(*activeNodes)[currentParent];
                currentParent = (*activeNodes)[currentParent].parentIndex;
            }
            return ancestorTable;
        },

        "getParent",
        [this](SceneNode &n) -> SceneNode * {
            if (n.parentIndex != -1 && n.parentIndex < activeNodes->size())
            {
                return &(*activeNodes)[n.parentIndex];
            }
            return nullptr;
        },
        "setParent",
        [this](SceneNode &child, SceneNode &newParent) {
            auto it = std::find_if(activeNodes->begin(), activeNodes->end(),
                                   [&](const SceneNode &node) { return &node == &newParent; });
            if (it != activeNodes->end())
            {
                child.parentIndex = std::distance(activeNodes->begin(), it);
            }
        });

    sol::table engineTable = lua.create_table();

    engineTable["getNode"] = [this](const std::string &nodeName) -> SceneNode * {
        if (!activeNodes)
            return nullptr;
        for (auto &node : *activeNodes)
            if (node.name == nodeName)
                return &node;
        return nullptr;
    };

    engineTable["destroyNode"] = [this](const std::string &nodeName) {
        if (!activeNodes)
            return;
        for (auto it = activeNodes->begin(); it != activeNodes->end(); ++it)
        {
            if (it->name == nodeName)
            {
                activeNodes->erase(it);
                break;
            }
        }
    };

    engineTable["stop"] = [this]() { isRunning = false; };

    engineTable["getDeltaTime"] = []() -> float { return ImGui::GetIO().DeltaTime; };

    lua["Engine"] = engineTable;
}

void LuaEngine::init()
{
    lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math);

    lua["print"] = [](sol::variadic_args args) {
        std::string full_msg;
        for (auto v : args)
            full_msg += v.as<std::string>() + " ";
        Output::addLog(full_msg);
    };

    sol::table inputTable = lua.create_table();

    inputTable["setMouseLock"] = [](bool locked) {
        SDL_Window *currentWindow = SDL_GetKeyboardFocus();
        if (currentWindow)
        {
            SDL_SetWindowRelativeMouseMode(currentWindow, locked ? true : false);
        }
    };

    inputTable["isKeyDown"] = [](const std::string &key) -> bool {
        const bool *state = SDL_GetKeyboardState(nullptr);
        SDL_Scancode sc = SDL_GetScancodeFromName(key.c_str());
        return sc != SDL_SCANCODE_UNKNOWN && state[sc] != 0;
    };

    inputTable["anyKey"] = []() -> bool {
        const bool *state = SDL_GetKeyboardState(nullptr);
        for (int i = 0; i < SDL_SCANCODE_COUNT; ++i)
            if (state[i])
                return true;
        return false;
    };

    inputTable["getKeyPressed"] = []() -> std::string {
        const bool *state = SDL_GetKeyboardState(nullptr);
        for (int i = 0; i < SDL_SCANCODE_COUNT; ++i)
        {
            if (state[i])
            {
                const char *name = SDL_GetScancodeName(static_cast<SDL_Scancode>(i));
                if (name && name[0] != '\0')
                    return name;
            }
        }
        return "";
    };

    inputTable["getMouseX"] = []() -> float {
        float x, y;
        SDL_GetMouseState(&x, &y);
        return x;
    };

    inputTable["getMouseY"] = []() -> float {
        float x, y;
        SDL_GetMouseState(&x, &y);
        return y;
    };

    inputTable["getMouseDeltaX"] = []() -> float {
        float x = 0.0f;
        SDL_GetRelativeMouseState(&x, nullptr);
        return x;
    };

    inputTable["getMouseDeltaY"] = []() -> float {
        float y = 0.0f;
        SDL_GetRelativeMouseState(nullptr, &y);
        return y;
    };

    inputTable["isMouseDown"] = [](int buttonIndex) -> bool {
        return (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_MASK(buttonIndex)) != 0;
    };

    lua["Input"] = inputTable;

    sol::table taskTable = lua.create_table();

    taskTable["delay"] = [this](float seconds, sol::protected_function fn) -> int {
        int handle = m_nextTaskHandle++;
        m_delayedTasks.push_back({handle, seconds, std::move(fn), false});
        return handle;
    };

    taskTable["spawn"] = [this](sol::protected_function fn) -> int {
        int handle = m_nextTaskHandle++;

        sol::thread runner = sol::thread::create(lua.lua_state());
        sol::coroutine co(runner.state(), fn);

        m_spawnedTasks.push_back({handle, std::move(runner), std::move(co), false});
        return handle;
    };

    taskTable["stop"] = [this](int handle) {
        for (auto &t : m_delayedTasks)
            if (t.handle == handle)
            {
                t.cancelled = true;
                return;
            }
        for (auto &t : m_spawnedTasks)
            if (t.handle == handle)
            {
                t.cancelled = true;
                return;
            }
    };

    lua["Task"] = taskTable;
}

void LuaEngine::runScript(const std::string &code)
{
    sol::environment env(lua, sol::create, lua.globals());
    m_environments.push_back(env);

    auto load_result = lua.load(code, "script", sol::load_mode::text);
    if (!load_result.valid())
    {
        sol::error err = load_result;
        Output::addLog("LUA SYNTAX ERROR: " + std::string(err.what()));
        return;
    }

    sol::protected_function script = load_result;
    env.set_on(script);

    auto exec_result = script();
    if (!exec_result.valid())
    {
        sol::error err = exec_result;
        Output::addLog("LUA EXECUTION ERROR: " + std::string(err.what()));
        return;
    }

    sol::protected_function onStart = env["onStart"];
    sol::protected_function onUpdate = env["onUpdate"];

    if (onStart.valid())
        m_startFuncs.push_back(onStart);
    if (onUpdate.valid())
        m_updateFuncs.push_back(onUpdate);

    if (isRunning && onStart.valid())
    {
        try
        {
            auto result = onStart();
            if (!result.valid())
            {
                sol::error err = result;
                Output::addLog("LUA RUNTIME ERROR (onStart): " + std::string(err.what()));
            }
        }
        catch (const std::exception &e)
        {
            Output::addLog("[ENGINE FATAL BINDING EXCEPTION]: " + std::string(e.what()));
        }
    }
}

void LuaEngine::step()
{
    if (!isRunning)
        return;

    float dt = ImGui::GetIO().DeltaTime;

    size_t delayCount = m_delayedTasks.size();
    for (size_t i = 0; i < delayCount; ++i)
    {
        if (m_delayedTasks[i].cancelled)
            continue;

        m_delayedTasks[i].remaining -= dt;
        if (m_delayedTasks[i].remaining <= 0.f)
        {
            m_delayedTasks[i].cancelled = true;
            if (m_delayedTasks[i].fn.valid())
            {
                bool hasError = false;
                {

                    auto funcCopy = m_delayedTasks[i].fn;
                    auto result = funcCopy();
                    if (!result.valid())
                    {
                        sol::error err = result;
                        Output::addLog("[TASK DELAY ERROR] " + std::string(err.what()));
                        hasError = true;
                    }
                }

                if (hasError)
                {
                    isRunning = false;
                    stop();
                    return;
                }
            }
        }
    }

    m_delayedTasks.erase(
        std::remove_if(m_delayedTasks.begin(), m_delayedTasks.end(), [](const DelayedTask &t) { return t.cancelled; }),
        m_delayedTasks.end());

    size_t spawnCount = m_spawnedTasks.size();
    for (size_t i = 0; i < spawnCount; ++i)
    {
        if (m_spawnedTasks[i].cancelled)
            continue;
        if (!m_spawnedTasks[i].co.valid())
        {
            m_spawnedTasks[i].cancelled = true;
            continue;
        }

        bool hasError = false;
        {
            auto result = m_spawnedTasks[i].co();
            if (!result.valid())
            {
                sol::error err = result;
                Output::addLog("[TASK SPAWN ERROR] " + std::string(err.what()));
                hasError = true;
            }
            else if (m_spawnedTasks[i].co.status() == sol::call_status::ok)
            {
                m_spawnedTasks[i].cancelled = true;
            }
        }

        if (hasError)
        {
            isRunning = false;
            stop();
            return;
        }
    }

    m_spawnedTasks.erase(
        std::remove_if(m_spawnedTasks.begin(), m_spawnedTasks.end(), [](const SpawnedTask &t) { return t.cancelled; }),
        m_spawnedTasks.end());

    if (m_updateFuncs.empty())
        return;

    for (size_t i = 0; i < m_updateFuncs.size(); ++i)
    {
        if (!m_updateFuncs[i].valid())
            continue;

        bool hasError = false;
        {
            auto funcCopy = m_updateFuncs[i];
            auto result = funcCopy();

            if (!result.valid())
            {
                sol::error err = result;
                sol::call_status status = result.status();

                if (status == sol::call_status::runtime)
                    Output::addLog("[USER SCRIPT ERROR] " + std::string(err.what()));
                else
                    Output::addLog("[ENGINE BINDING ERROR] C++ failed to execute: " + std::string(err.what()));

                hasError = true;
            }
        }

        if (hasError)
        {
            isRunning = false;
            stop();
            return;
        }
    }
}

void LuaEngine::stop()
{
    if (!isRunning && m_startFuncs.empty() && m_updateFuncs.empty())
        return;

    isRunning = false;

    m_startFuncs.clear();
    m_updateFuncs.clear();
    m_environments.clear();

    m_delayedTasks.clear();
    m_spawnedTasks.clear();

    lua = sol::state{};
}

void LuaEngine::runAllScriptsInFolder(const std::string &folderPath)
{
    m_startFuncs.clear();
    m_updateFuncs.clear();
    m_environments.clear();

    for (const auto &entry : std::filesystem::recursive_directory_iterator(folderPath))
    {
        if (entry.path().extension() != ".lua")
            continue;

        std::ifstream ifs(entry.path());
        if (!ifs.is_open())
            continue;

        std::string code{std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};
        runScript(code);
    }
}

} // namespace Flux