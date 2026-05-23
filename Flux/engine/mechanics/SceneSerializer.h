#pragma once

#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include "heiarchy.h"
#include "Model.h"

#include "Textureloader.h"

#include "utils/pathHelper.h"

using json = nlohmann::json;

namespace Flux {
    class SceneSerializer {
        public:
            static void Save(const Heiarchy& heiarchy, const std::string& filePath, const std::filesystem::path& projectRoot) {
                json j;
                j["nodes"] = json::array();

                for (const auto& node : heiarchy.nodes) {
                    json n;
                    n["name"] = node.name;
                    n["type"] = (int)(node.type);
                    n["position"] = { node.position.x, node.position.y, node.position.z};
                    n["rotation"] = { node.rotation.x, node.rotation.y, node.rotation.z};
                    n["scale"] = {node.scale.x, node.scale.y, node.scale.z};

                    if (!node.texturePath.empty()) {
                        n["texturePath"] = std::filesystem::relative(node.texturePath, projectRoot).string();
                    }
                    if (node.model) {
                        n["modelPath"] = std::filesystem::relative(node.model->path, projectRoot).string();
                    }

                    n["baseColor"] = {node.baseColor.r, node.baseColor.g, node.baseColor.b};

                    n["roughness"] = node.roughness;
                    n["metallic"] = node.metallic;
                    n["isLightingNode"] = node.isLightingNode;

                    if (node.type == NodeType::Camera) n["isMainCamera"] = node.isMainCamera;

                    if (node.type == NodeType::Camera) n["fov"] = node.fov;

                    n["velocity"] = {node.velocity.x, node.velocity.y, node.velocity.z};
                    n["isAnchored"] = node.isAnchored;

                    j["nodes"].push_back(n);
                }
                std::ofstream file(filePath);
                file << j.dump(4);
                j["version"] = 2;
            }

            static void Load(Heiarchy& h, const std::filesystem::path& loadPath, const std::filesystem::path& projectRoot) {
                std::ifstream file(loadPath);
                if (!file.is_open()) return;
                json j;
                file >> j;

                int version = j.value("version", 1); // 1 = legacy, 2 = current

                h.nodes.clear();

                for (const auto& jNode : j["nodes"]) {
                    SceneNode n;
                    n.name = jNode["name"];
                    n.type = (NodeType)jNode["type"];

                    n.position = glm::vec3(jNode["position"][0], jNode["position"][1], jNode["position"][2]);
                    n.rotation = glm::vec3(jNode["rotation"][0], jNode["rotation"][1], jNode["rotation"][2]);
                    n.scale = glm::vec3(jNode["scale"][0], jNode["scale"][1], jNode["scale"][2]);

                    if (jNode.contains("modelPath") && !jNode["modelPath"].get<std::string>().empty()) {
                        std::filesystem::path absoluteModel = projectRoot / jNode["modelPath"].get<std::string>();
                        n.model = std::make_shared<Model>(absoluteModel.string());
                    }

                    n.roughness = jNode.value("roughness", 0.7f);
                    n.metallic  = jNode.value("metallic", 0.0f);
                    n.isLightingNode = jNode.value("isLightingNode", false);
                    if (n.type == NodeType::Camera) n.isMainCamera = jNode.value("isMainCamera", false);

                    if (jNode.contains("baseColor")) {
                        n.baseColor = glm::vec3(
                            jNode["baseColor"][0],
                            jNode["baseColor"][1],
                            jNode["baseColor"][2]
                        );
                    } else {
                        n.baseColor = glm::vec3(0.8f);
                    }

                    if (n.type == NodeType::Camera) n.fov = jNode.value("fov", 70.0f);
                    
                    if (jNode.contains("texturePath") && !jNode["texturePath"].get<std::string>().empty()) {
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
                        n.isLightingNode = (n.type == NodeType::DirectionalLight && n.name == "Lighting");

                    if (!jNode.contains("roughness")) n.roughness = 0.7f;
                    if (!jNode.contains("metallic"))  n.metallic  = 0.0f;

                    if (n.type == NodeType::Camera) {
                    std::string modelPath = PathHelper::GetAssetPath("assets/models/camera.obj");
                    if (std::filesystem::exists(modelPath))
                        n.model = std::make_shared<Model>(modelPath);
                    // icon
                    std::string iconPath = PathHelper::GetAssetPath("assets/icons/camera.png");
                    if (std::filesystem::exists(iconPath))
                        n.textureID = TextureLoader::Load(iconPath);

                    n.isAnchored = jNode.value("isAnchored", false);
                    if (jNode.contains("velocity")) {
                        auto v = jNode["velocity"];
                        n.velocity = glm::vec3(v[0], v[1], v[2]);
                    }
                }
                h.nodes.push_back(n);
                }
            }
    };
}