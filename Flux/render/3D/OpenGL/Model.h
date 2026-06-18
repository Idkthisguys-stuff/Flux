#pragma once
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include "logic/data/material.h"

namespace Flux
{

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

struct Mesh
{
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int EBO = 0;
    unsigned int indexCount = 0;
    unsigned int textureID = 0;
    glm::vec3 matColor = glm::vec3(0.8f, 0.4f, 0.1f);
    bool hasMtlColor = false;

    bool twoSided = false;
    bool hasAlpha = false;

    Material material;

    std::vector<Vertex> verticies;
    std::vector<unsigned int> indices;
};

class Model
{
  public:
    std::string path;
    std::vector<Mesh> meshes;

    Model(const std::string &modelPath) : path(modelPath)
    {
        Load();
    }
    ~Model();

    void Load();
    void Draw(float alphaOverride = 1.0f);
    void SetTexture(unsigned int texID);
};

} // namespace Flux