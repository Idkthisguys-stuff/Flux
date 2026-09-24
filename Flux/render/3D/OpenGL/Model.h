#pragma once
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <SDL3/SDL_gpu.h>
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
    SDL_GPUBuffer* vertexBuffer = nullptr;
    SDL_GPUBuffer* indexBuffer = nullptr;

    SDL_GPUTexture* texture = nullptr;
    SDL_GPUSampler* sampler = nullptr;

    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;

    glm::vec3 matColor = glm::vec3(0.8f, 0.04f, 0.1f);
    bool hasMtlColor = false;
    bool twoSided = false;
    bool hasAlpha = false;

    Material mat;

    std::vector<Vertex> vertecies;
    std::vector<uint32_t> indices;
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