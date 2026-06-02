#include "runtime.h"
#include <glad/glad.h>
#include <iostream>

#include <Jolt/Math/Quat.h>
#include <Jolt/Math/Vec3.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/RegisterTypes.h>
namespace Flux
{

void Runtime::Start(const std::string &projectName, const std::filesystem::path &projectPath,
                    std::vector<SceneNode> &copiedNodes, int windowWidth, int windowHeight)
{
    if (isRunning)
        Stop();

    isRunning = true;

    lastTimeFrame = SDL_GetPerformanceCounter();

    if (!std::filesystem::exists(projectPath) || !std::filesystem::is_directory(projectPath))
    {
        Output::addLog("RUNTIME ERROR: Project folder missing: " + projectPath.string());
        isRunning = false;
        return;
    }

    Output::addLog("Runtime: Creating game window...");

    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);

    m_window =
        SDL_CreateWindow(projectName.c_str(), windowWidth, windowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!m_window)
    {
        Output::addLog("RUNTIME ERROR: SDL_CreateWindow failed: " + std::string(SDL_GetError()));
        isRunning = false;
        return;
    }

    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext)
    {
        Output::addLog("RUNTIME ERROR: SDL_GL_CreateContext failed: " + std::string(SDL_GetError()));
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
        isRunning = false;
        return;
    }

    SDL_GL_MakeCurrent(m_window, m_glContext);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        Output::addLog("RUNTIME ERROR: Failed to load GLAD for runtime context");
        isRunning = false;
        return;
    }

    SDL_ShowWindow(m_window);
    SDL_RaiseWindow(m_window);

    SplashConfig splash;
    splash.title = "Flux Game";
    splash.subtitle = "Loading...";
    Flux::RunSplashScreen(m_window, splash);

    if (!SDL_GL_MakeCurrent(m_window, m_glContext))
    {
        Output::addLog("RUNTIME ERROR: Context re-binding failed after splash screen: " + std::string(SDL_GetError()));
        isRunning = false;
        return;
    }

    SDL_GL_SetSwapInterval(1);

    for (auto &[path, id] : m_runtimeTextureCache)
        if (id)
            glDeleteTextures(1, &id);
    m_runtimeTextureCache.clear();

    m_renderer.Init();
    m_renderer.InitSkybox();
    m_renderer.InitShadowMap(4096);

    m_gameNodes.clear();

    for (const auto &node : copiedNodes)
    {
        SceneNode newNode = node;

        if (node.model)
            newNode.model = std::make_shared<Model>(node.model->path);

        if (!node.texturePath.empty())
        {
            auto it = m_runtimeTextureCache.find(node.texturePath);
            if (it != m_runtimeTextureCache.end())
            {
                newNode.textureID = it->second;
            }
            else
            {
                unsigned int id = TextureLoader::Load(node.texturePath);
                m_runtimeTextureCache[node.texturePath] = id;
                newNode.textureID = id;
            }
        }

        newNode.baseColor = node.baseColor;
        newNode.roughness = node.roughness;
        newNode.metallic = node.metallic;

        m_gameNodes.push_back(std::move(newNode));
    }

    m_luaEngine.activeNodes = &m_gameNodes;
    m_luaEngine.init();
    m_luaEngine.bindEngineAPI();
    m_luaEngine.isRunning = true;
    m_luaEngine.runAllScriptsInFolder(projectPath.string());

    static bool joltInitialized = false;

    if (!joltInitialized)
    {
        JPH::RegisterDefaultAllocator();
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();
        joltInitialized = true;
    }
    m_tempAllocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);
    m_jobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers,
                                               std::thread::hardware_concurrency() - 1);

    m_PhysicsSystem = new JPH::PhysicsSystem();

    JPH::PhysicsSettings settings;

    m_PhysicsSystem->SetGravity(JPH::Vec3(0.0f, -4.0f, 0.0f));

    m_PhysicsSystem->Init(1024, 0, 1024, 1024, m_broadPhaseLayerInterface, m_objectVsBroadPhaseLayerFilter,
                          m_objectLayerPairFilter);

    JPH::BodyInterface &bodyInterface = m_PhysicsSystem->GetBodyInterface();
    for (auto &node : m_gameNodes)
    {
        if (node.type == NodeType::Mesh && node.model)
        {
            JPH::ShapeRefC finalShape;

            JPH::Array<JPH::Vec3> joltVerticies;
            for (const auto &mesh : node.model->meshes)
            {
                for (const auto &v : mesh.verticies)
                {
                    joltVerticies.push_back(JPH::Vec3(v.Position.x * node.scale.x, v.Position.y * node.scale.y,
                                                      v.Position.z * node.scale.z));
                }
            }

            if (!node.isAnchored)
            {
                JPH::ConvexHullShapeSettings hullSettings(joltVerticies, JPH::cDefaultConvexRadius);
                JPH::ShapeSettings::ShapeResult hullResult = hullSettings.Create();

                if (hullResult.HasError())
                {
                    Output::addLog("Hull Error: " + std::string(hullResult.GetError().c_str()));
                    continue;
                }

                finalShape = hullResult.Get();
            }
            else
            {
                JPH::VertexList joltVerticies;
                JPH::IndexedTriangleList joltTriangles;

                uint32_t vertexOffset = 0;

                for (const auto &mesh : node.model->meshes)
                {
                    for (const auto &v : mesh.verticies)
                    {
                        joltVerticies.push_back(JPH::Float3(v.Position.x * node.scale.x, v.Position.y * node.scale.y,
                                                            v.Position.z * node.scale.z));
                    }

                    for (size_t i = 0; i < mesh.indices.size(); i += 3)
                    {
                        joltTriangles.push_back(JPH::IndexedTriangle(vertexOffset + mesh.indices[i],
                                                                     vertexOffset + mesh.indices[i + 1],
                                                                     vertexOffset + mesh.indices[i + 2]));
                    }

                    vertexOffset += mesh.verticies.size();
                }

                JPH::MeshShapeSettings meshSettings(joltVerticies, joltTriangles);

                meshSettings.Sanitize();

                JPH::ShapeSettings::ShapeResult meshResult = meshSettings.Create();
                if (meshResult.HasError())
                {
                    Output::addLog("TriMesh Error: " + std::string(meshResult.GetError().c_str()));
                    continue;
                }

                finalShape = meshResult.Get();
            }

            JPH::EMotionType motionType = node.isAnchored ? JPH::EMotionType::Kinematic : JPH::EMotionType::Dynamic;
            JPH::ObjectLayer layer = node.isAnchored ? Layers::NON_MOVING : Layers::MOVING;

            auto rot = glm::radians(node.rotation);
            JPH::Quat joltRotation = JPH::Quat::sEulerAngles(JPH::Vec3(rot.x, rot.y, rot.z));

            JPH::BodyCreationSettings settings(finalShape, JPH::Vec3(node.position.x, node.position.y, node.position.z),
                                               joltRotation, motionType, layer);

            settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
            settings.mMassPropertiesOverride.mMass = 1.0f;

            JPH::Body *body = bodyInterface.CreateBody(settings);
            bodyInterface.AddBody(body->GetID(), JPH::EActivation::Activate);
            node.physicsBodyID = body->GetID();
        }
    }

    Output::addLog("Runtime started successfully.");

    if (m_window && m_glContext)
        SDL_GL_MakeCurrent(m_window, nullptr);
}

void Runtime::Update()
{
    if (!isRunning || !m_window || !m_glContext)
        return;

    if (!SDL_GL_MakeCurrent(m_window, m_glContext))
    {
        Output::addLog("RUNTIME ERROR: Context re-binding failed: " + std::string(SDL_GetError()));
        isRunning = false;
        return;
    }

    m_luaEngine.step();

    int w, h;
    SDL_GetWindowSize(m_window, &w, &h);

    glViewport(0, 0, w, h);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float gameTime = 14.0f;
    glm::vec3 gameSunDir = glm::vec3(0, -1, 0);

    glm::mat4 proj = glm::perspective(glm::radians(70.0f), (float)w / (float)h, 0.1f, 2000.0f);
    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, glm::vec3(0, 1, 0));

    uint64_t semiCurrentTime = SDL_GetPerformanceCounter();
    float dt = (float)(semiCurrentTime - lastTimeFrame) / (float)SDL_GetPerformanceFrequency();
    if (dt > 0.33f)
        dt = 0.33f;

    JPH::BodyInterface &bodyInterface = m_PhysicsSystem->GetBodyInterface();

    for (auto &node : m_gameNodes)
    {
        if (node.type == NodeType::Mesh && node.isAnchored && !node.physicsBodyID.IsInvalid())
        {
            JPH::Vec3 joltPos(node.position.x, node.position.y, node.position.z);
            auto rot = glm::radians(node.rotation);
            JPH::Quat joltRot = JPH::Quat::sEulerAngles(JPH::Vec3(rot.x, rot.y, rot.z));
            bodyInterface.SetPositionAndRotation(node.physicsBodyID, joltPos, joltRot, JPH::EActivation::Activate);
        }
    }

    m_PhysicsSystem->Update(dt, 1, m_tempAllocator, m_jobSystem);

    for (auto &node : m_gameNodes)
    {
        if (node.type == NodeType::Mesh && !node.isAnchored && !node.physicsBodyID.IsInvalid())
        {
            JPH::Vec3 pos = bodyInterface.GetPosition(node.physicsBodyID);
            JPH::Quat rot = bodyInterface.GetRotation(node.physicsBodyID);

            glm::quat glmRot(rot.GetW(), rot.GetX(), rot.GetY(), rot.GetZ());
            glm::mat4 rotMat = glm::mat4_cast(glmRot);
            glm::mat4 transMat = glm::translate(glm::mat4(1.0f), glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ()));
            glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), node.scale);

            node.physicsWorldMatrix = transMat * rotMat * scaleMat;
            node.hasPhysicsTransform = true;
            node.position = glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ());
        }

        if (node.isLightingNode)
        {
            gameTime = node.light.timeOfDay;
            gameSunDir = node.light.direction;
        }
    }

    for (auto &node : m_gameNodes)
    {
        if (node.type == NodeType::Camera && node.isMainCamera)
        {
            glm::mat4 transform = node.GetTransformMatrix();
            glm::vec3 camPos = node.position;
            glm::vec3 camFront = glm::normalize(glm::vec3(transform * glm::vec4(0, 0, -1, 0)));
            glm::vec3 camUp = glm::normalize(glm::vec3(transform * glm::vec4(0, 1, 0, 0)));

            view = glm::lookAt(camPos, camPos + camFront, camUp);
            proj =
                glm::perspective(glm::radians(node.fov > 0.f ? node.fov : 70.0f), (float)w / (float)h, 0.1f, 2000.0f);
            break;
        }
    }

    m_renderer.DrawSkybox(view, proj, gameSunDir, gameTime, true);

    for (auto &node : m_gameNodes)
    {
        if (node.model)
        {
            glm::mat4 modelMat = node.GetTransformMatrix();
            m_renderer.DrawScene(*node.model, node.textureID, modelMat, view, proj, cameraPos, m_gameNodes, 1.0f,
                                 node.roughness, node.metallic, gameTime, node.baseColor);
        }
    }

    SDL_GL_SwapWindow(m_window);

    if (m_window && m_glContext)
        SDL_GL_MakeCurrent(m_window, nullptr);

    lastTimeFrame = semiCurrentTime;
}

void Runtime::Stop()
{
    if (!isRunning)
        return;

    Output::addLog("Stopping runtime...");

    if (m_tempAllocator != nullptr && m_PhysicsSystem != nullptr)
    {
        JPH::BodyInterface &bodyInterface = m_PhysicsSystem->GetBodyInterface();
        for (auto &node : m_gameNodes)
        {
            if (node.type == NodeType::Mesh && !node.physicsBodyID.IsInvalid())
            {
                bodyInterface.RemoveBody(node.physicsBodyID);
                bodyInterface.DestroyBody(node.physicsBodyID);

                node.physicsBodyID = JPH::BodyID();
            }
        }

        delete m_PhysicsSystem;
        m_PhysicsSystem = nullptr;

        delete m_jobSystem;
        m_jobSystem = nullptr;

        delete m_tempAllocator;
        m_tempAllocator = nullptr;
    }

    m_luaEngine.stop();
    m_luaEngine.isRunning = false;

    if (m_glContext && m_window)
    {
        SDL_GL_MakeCurrent(m_window, m_glContext);
        for (auto &[path, id] : m_runtimeTextureCache)
            if (id)
                glDeleteTextures(1, &id);
        m_runtimeTextureCache.clear();
    }

    if (m_glContext)
    {
        SDL_GL_DestroyContext(m_glContext);
        m_glContext = nullptr;
    }
    if (m_window)
    {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    m_gameNodes.clear();
    isRunning = false;

    Output::addLog("Runtime stopped.");
}

void Runtime::SyncCamera(glm::vec3 editorPos, glm::vec3 editorTarget)
{
    this->cameraPos = editorPos;
    this->cameraTarget = editorTarget;
}

} // namespace Flux