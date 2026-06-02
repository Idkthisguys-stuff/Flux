#pragma once

#include "editor/heiarchy.h"
#include "render/3D/OpenGL/Model.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

#include "logic/Textureloader.h"

#include "core/pathHelper.h"

using json = nlohmann::json;

namespace Flux
{
class SceneSerializer
{
  public:
    static void Save(const Heiarchy &heiarchy, const std::string &filePath, const std::filesystem::path &projectRoot)
    {
        json j;
        j["nodes"] = json::array();

        for (const auto &node : heiarchy.nodes)
        {
            json n;
            n["name"] = node.name;
            n["type"] = (int)(node.type);
            n["position"] = {node.position.x, node.position.y, node.position.z};
            n["rotation"] = {node.rotation.x, node.rotation.y, node.rotation.z};
            n["scale"] = {node.scale.x, node.scale.y, node.scale.z};

            if (!node.texturePath.empty())
            {
                n["texturePath"] = std::filesystem::relative(node.texturePath, projectRoot).string();
            }
            if (node.model)
            {
                n["modelPath"] = std::filesystem::relative(node.model->path, projectRoot).string();
            }

            n["baseColor"] = {node.baseColor.r, node.baseColor.g, node.baseColor.b};

            n["roughness"] = node.roughness;
            n["metallic"] = node.metallic;

            // -- LIGHTING STUFF --
            n["isLightingNode"] = node.isLightingNode;
            n["timeOfDay"] = node.light.timeOfDay;
            n["lightColor"] = {node.light.color.r, node.light.color.g, node.light.color.b};
            n["intensity"] = node.light.intensity;
            n["range"] = node.light.range;
            n["innerCutoff"] = node.light.innerCutoff;
            n["outerCutoff"] = node.light.outerCutoff;
            n["lightDirection"] = {node.light.direction.x, node.light.direction.y, node.light.direction.z};
            n["brightness"] = node.light.brightness;
            n["ambientDaytime"] = node.light.ambientDaytime;
            n["ambientNight"] = node.light.ambientNight;
            n["fogColor"] = {node.light.fogColor.r, node.light.fogColor.g, node.light.fogColor.b};
            n["lightDirection"] = {node.light.direction.x, node.light.direction.y, node.light.direction.z};

            //////////////////////

            if (node.type == NodeType::Camera)
                n["isMainCamera"] = node.isMainCamera;

            if (node.type == NodeType::Camera)
                n["fov"] = node.fov;

            if (node.isLightingNode)
            {
                n["timeofday"] = node.light.timeOfDay;
            }

            n["velocity"] = {node.velocity.x, node.velocity.y, node.velocity.z};
            n["isAnchored"] = node.isAnchored;

            j["nodes"].push_back(n);
        }
        std::ofstream file(filePath);
        file << j.dump(4);
        j["version"] = 2;
    }

    static void Load(Heiarchy &h, const std::filesystem::path &loadPath, const std::filesystem::path &projectRoot)
    {
        std::ifstream file(loadPath);
        if (!file.is_open())
            return;
        json j;
        file >> j;

        int version = j.value("version", 1); // 1 = legacy, 2 = current

        h.nodes.clear();

        for (const auto &jNode : j["nodes"])
        {
            SceneNode n;
            n.name = jNode["name"];
            n.type = (NodeType)jNode["type"];

            n.position = glm::vec3(jNode["position"][0], jNode["position"][1], jNode["position"][2]);
            n.rotation = glm::vec3(jNode["rotation"][0], jNode["rotation"][1], jNode["rotation"][2]);
            n.scale = glm::vec3(jNode["scale"][0], jNode["scale"][1], jNode["scale"][2]);

            if (jNode.contains("modelPath") && !jNode["modelPath"].get<std::string>().empty())
            {
                std::filesystem::path absoluteModel = projectRoot / jNode["modelPath"].get<std::string>();
                n.model = std::make_shared<Model>(absoluteModel.string());
            }

            n.roughness = jNode.value("roughness", 0.7f);
            n.metallic = jNode.value("metallic", 0.0f);

            n.isLightingNode = jNode.value("isLightingNode", false);
            n.light.intensity = jNode.value("intensity", 1.0f);
            n.light.range = jNode.value("range", 10.0f);
            n.light.innerCutoff = jNode.value("innerCutoff", 12.5f);
            n.light.outerCutoff = jNode.value("outerCutoff", 17.5f);
            n.light.brightness = jNode.value("brightness", 2.0f);
            n.light.ambientDaytime = jNode.value("ambientDaytime", 0.3f);
            n.light.ambientNight = jNode.value("ambientNight", 0.1f);

            if (jNode.contains("lightDirection") && jNode["lightDirection"].is_array() &&
                jNode["lightDirection"].size() == 3)
            {
                auto d = jNode["lightDirection"];
                n.light.direction = glm::vec3(d[0].get<float>(), d[1].get<float>(), d[2].get<float>());
            }
            else
            {
                n.light.direction = glm::vec3(0.0f, -1.0f, 0.0f);
            }

            if (n.type == NodeType::Camera)
                n.isMainCamera = jNode.value("isMainCamera", false);

            if (jNode.contains("baseColor"))
            {
                n.baseColor = glm::vec3(jNode["baseColor"][0], jNode["baseColor"][1], jNode["baseColor"][2]);
            }
            else
            {
                n.baseColor = glm::vec3(0.8f);
            }

            if (n.type == NodeType::Camera)
                n.fov = jNode.value("fov", 70.0f);

            if (jNode.contains("texturePath") && !jNode["texturePath"].get<std::string>().empty())
            {
                std::filesystem::path absoluteTex = projectRoot / jNode["texturePath"].get<std::string>();
                n.texturePath = absoluteTex.string();
                n.textureID = TextureLoader::Load(absoluteTex.string());

                unsigned int id = TextureLoader::Load(absoluteTex.string());
                Output::addLog("Loaded texture: " + absoluteTex.string() + " -> ID " + std::to_string(id));
                n.textureID = id;
            }

            if (n.type == NodeType::Camera && !jNode.contains("fov"))
                n.fov = 70.0f;

            if (!jNode.contains("isLightingNode"))
            {
                n.isLightingNode = (n.type == NodeType::DirectionalLight && n.name == "Lighting");
            }

            if (jNode.contains("timeOfDay"))
            {
                n.light.timeOfDay = jNode["timeOfDay"].get<float>();
            }
            else
            {
                n.light.timeOfDay = 14.0f;
            }

            if (!jNode.contains("roughness"))
                n.roughness = 0.7f;
            if (!jNode.contains("metallic"))
                n.metallic = 0.0f;

            n.isAnchored = jNode.value("isAnchored", false);
            if (jNode.contains("velocity"))
            {
                auto v = jNode["velocity"];
                n.velocity = glm::vec3(v[0], v[1], v[2]);
            }

            if (n.type == NodeType::Camera)
            {
                std::string iconPath = PathHelper::GetAssetPath("assets/icons/camera.png");
                if (std::filesystem::exists(iconPath))
                    n.textureID = TextureLoader::Load(iconPath);
            }
            h.nodes.push_back(n);
        }
    }
};
} // namespace Flux