#include "Model.h"
#include "logic/Textureloader.h"
#include <filesystem>
#include <iostream>

namespace Flux
{

static bool HasRealAlpha(const unsigned char *data, int w, int h)
{
    const int total = w * h;
    for (int i = 0; i < total; ++i)
        if (data[i * 4 + 3] < 254)
            return true;
    return false;
}

static unsigned int LoadMaterialTextures(aiMaterial *mat, aiTextureType type, const aiScene *scene,
                                         const std::string &modelPath, const std::filesystem::path &modelDir,
                                         bool &outHasAlpha, std::string &outPath)
{
    aiString texPath;
    if (mat->GetTexture(type, 0, &texPath) != AI_SUCCESS)
    {
        return 0;
    }

    std::string rawPath = texPath.C_Str();

    const aiTexture *aitex = scene->GetEmbeddedTexture(rawPath.c_str());
    if (aitex)
    {
        outPath = "[Embedded] " + std::filesystem::path(rawPath).filename().string();

        std::string cacheKey = modelPath + "#" + rawPath;

        if (aitex->mHeight == 0)
        {
            return TextureLoader::LoadFromMemory(cacheKey, reinterpret_cast<const unsigned char *>(aitex->pcData),
                                                 (size_t)aitex->mWidth, &outHasAlpha);
        }
        else
        {
            return TextureLoader::LoadFromMemoryRaw(cacheKey, reinterpret_cast<const unsigned char *>(aitex->pcData),
                                                    (int)aitex->mWidth, (int)aitex->mHeight, GL_BGRA);
        }
    }
    else
    {
        std::filesystem::path fullTex = modelDir / rawPath;
        std::string texStr = fullTex.string();
        std::replace(texStr.begin(), texStr.end(), '\\', '/');

        if (!std::filesystem::exists(texStr))
        {
            std::string justName = std::filesystem::path(rawPath).filename().string();
            texStr = (modelDir / justName).string();
            std::replace(texStr.begin(), texStr.end(), '\\', '/');

            if (!std::filesystem::exists(texStr))
            {
                std::filesystem::path fbmDir = modelDir / (std::filesystem::path(modelPath).stem().string() + ".fbm");
                std::filesystem::path fbmPath = fbmDir / justName;
                if (std::filesystem::exists(fbmPath))
                {
                    texStr = fbmPath.string();
                    std::replace(texStr.begin(), texStr.end(), '\\', '/');
                }
            }
        }

        outPath = texStr;

        unsigned int id = TextureLoader::Load(texStr);

        size_t fsize;
        void *fbuf = SDL_LoadFile(texStr.c_str(), &fsize);
        if (fbuf)
        {
            int aw, ah, ach;
            unsigned char *adata = stbi_load_from_memory((const unsigned char *)fbuf, (int)fsize, &aw, &ah, &ach, 4);
            SDL_free(fbuf);
            if (adata)
            {
                outHasAlpha = HasRealAlpha(adata, aw, ah);
                stbi_image_free(adata);
            }
        }
        return id;
    }
}

Model::~Model()
{
    for (auto &mesh : meshes)
    {
        glDeleteVertexArrays(1, &mesh.VAO);
        glDeleteBuffers(1, &mesh.VBO);
        glDeleteBuffers(1, &mesh.EBO);
    }
}

void Model::Load()
{
    for (auto &mesh : meshes)
    {
        glDeleteVertexArrays(1, &mesh.VAO);
        glDeleteBuffers(1, &mesh.VBO);
        glDeleteBuffers(1, &mesh.EBO);
    }
    meshes.clear();

    size_t fileSize;
    void *fileBuffer = SDL_LoadFile(path.c_str(), &fileSize);
    if (!fileBuffer)
    {
        std::cerr << "Failed to load model file: " << path << " - " << SDL_GetError() << std::endl;
        Output::addLog("MODEL ERROR: Failed to load model file: " + path);
        return;
    }

    std::string ext = std::filesystem::path(path).extension().string();
    if (!ext.empty() && ext[0] == '.')
        ext = ext.substr(1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    bool isFbx = (ext == "fbx");
    unsigned int importFlags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace |
                               aiProcess_PreTransformVertices | aiProcess_JoinIdenticalVertices |
                               aiProcess_OptimizeMeshes | aiProcess_RemoveRedundantMaterials |
                               aiProcess_FindDegenerates | aiProcess_SortByPType | aiProcess_FixInfacingNormals;
    if (isFbx)
        importFlags |= aiProcess_FlipUVs;

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFileFromMemory(fileBuffer, fileSize, importFlags, ext.c_str());

    SDL_free(fileBuffer);

    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
    {
        std::cerr << "ASSIMP: Could not parse buffer from " << path << "\n";
        Output::addLog("MODEL ERROR: Could not parse buffer from " + path);
        return;
    }

    std::filesystem::path modelDir = std::filesystem::path(path).parent_path();

    for (unsigned int i = 0; i < scene->mNumMeshes; i++)
    {
        aiMesh *aMesh = scene->mMeshes[i];

        std::vector<Vertex> verts;
        std::vector<unsigned int> indices;

        for (unsigned int j = 0; j < aMesh->mNumVertices; j++)
        {
            Vertex v;
            v.Position = {aMesh->mVertices[j].x, aMesh->mVertices[j].y, aMesh->mVertices[j].z};
            v.Normal = aMesh->HasNormals() ? glm::vec3(aMesh->mNormals[j].x, aMesh->mNormals[j].y, aMesh->mNormals[j].z)
                                           : glm::vec3(0.f, 1.f, 0.f);
            v.TexCoords = aMesh->mTextureCoords[0]
                              ? glm::vec2(aMesh->mTextureCoords[0][j].x, aMesh->mTextureCoords[0][j].y)
                              : glm::vec2(0.f);
            v.Tangent = aMesh->mTangents
                            ? glm::vec3(aMesh->mTangents[j].x, aMesh->mTangents[j].y, aMesh->mTangents[j].z)
                            : glm::vec3(1.f, 0.f, 0.f);
            v.Bitangent = aMesh->mBitangents
                              ? glm::vec3(aMesh->mBitangents[j].x, aMesh->mBitangents[j].y, aMesh->mBitangents[j].z)
                              : glm::vec3(0.f, 1.f, 0.f);
            verts.push_back(v);
        }

        for (unsigned int j = 0; j < aMesh->mNumFaces; j++)
        {
            const aiFace &f = aMesh->mFaces[j];
            for (unsigned int k = 0; k < f.mNumIndices; k++)
                indices.push_back(f.mIndices[k]);
        }

        if (verts.empty() || indices.empty())
            continue;

        Mesh myMesh;
        myMesh.indexCount = (unsigned int)indices.size();

        if (aMesh->mMaterialIndex < scene->mNumMaterials)
        {
            aiMaterial *mat = scene->mMaterials[aMesh->mMaterialIndex];

            int twoSidedVal = 0;
            mat->Get(AI_MATKEY_TWOSIDED, twoSidedVal);
            myMesh.twoSided = (twoSidedVal != 0);

            bool dummyAlpha = false;

            myMesh.material.baseColor = myMesh.matColor;
            
            myMesh.textureID = LoadMaterialTextures(mat, aiTextureType_DIFFUSE, scene, path, modelDir, myMesh.hasAlpha,
                                                    myMesh.material.albedoPath);

            myMesh.material.normalMap = LoadMaterialTextures(mat, aiTextureType_NORMALS, scene, path, modelDir,
                                                             dummyAlpha, myMesh.material.normalPath);
            if (myMesh.material.normalMap == 0)
            {
                myMesh.material.normalMap = LoadMaterialTextures(mat, aiTextureType_HEIGHT, scene, path, modelDir,
                                                                 dummyAlpha, myMesh.material.normalPath);
            }
            myMesh.material.metallicMap = LoadMaterialTextures(mat, aiTextureType_METALNESS, scene, path, modelDir,
                                                               dummyAlpha, myMesh.material.metallicPath);
            myMesh.material.roughnessMap = LoadMaterialTextures(mat, aiTextureType_DIFFUSE_ROUGHNESS, scene, path,
                                                                modelDir, dummyAlpha, myMesh.material.roughnessPath);
            if (myMesh.material.roughnessMap == 0)
            {
                myMesh.material.roughnessMap = LoadMaterialTextures(mat, aiTextureType_SHININESS, scene, path, modelDir,
                                                                    dummyAlpha, myMesh.material.roughnessPath);
            }
            myMesh.material.aoMap = LoadMaterialTextures(mat, aiTextureType_AMBIENT, scene, path, modelDir, dummyAlpha,
                                                         myMesh.material.aoPath);

            float opacity = 1.0f;
            mat->Get(AI_MATKEY_OPACITY, opacity);
            if (opacity < 0.99f)
                myMesh.hasAlpha = true;

            if (myMesh.hasAlpha && myMesh.textureID != 0 && !myMesh.twoSided)
                myMesh.twoSided = true;

            aiColor3D col(0.8f, 0.4f, 0.1f);
            mat->Get(AI_MATKEY_COLOR_DIFFUSE, col);
            myMesh.matColor = glm::vec3(col.r, col.g, col.b);
            myMesh.material.albedoMap = myMesh.textureID;
            myMesh.hasMtlColor = true;

            mat->Get(AI_MATKEY_METALLIC_FACTOR, myMesh.material.metallic);
            mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, myMesh.material.roughness);
        }

        glGenVertexArrays(1, &myMesh.VAO);
        glGenBuffers(1, &myMesh.VBO);
        glGenBuffers(1, &myMesh.EBO);
        glBindVertexArray(myMesh.VAO);

        glBindBuffer(GL_ARRAY_BUFFER, myMesh.VBO);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, myMesh.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Position));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, TexCoords));

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Tangent));

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Bitangent));

        glBindVertexArray(0);

        myMesh.verticies = verts;
        myMesh.indices = indices;

        meshes.push_back(myMesh);
    }
}

void Model::Draw(float /*alphaOverride*/)
{
    for (auto &mesh : meshes)
    {
        // Albedo / Diffuse
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mesh.material.albedoMap ? mesh.material.albedoMap : mesh.textureID);

        // Normals
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mesh.material.normalMap);

        // Metallic
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, mesh.material.metallicMap);

        // Roughness
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, mesh.material.roughnessMap);

        // Ambient Occlusion
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, mesh.material.aoMap);

        /* // Displacement
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, mesh.material.dispMap);

        // Alpha
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, mesh.material.alphaMap);*/

        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }
}

void Model::SetTexture(unsigned int texID)
{
    for (auto &m : meshes)
        m.textureID = texID;
}

} // namespace Flux