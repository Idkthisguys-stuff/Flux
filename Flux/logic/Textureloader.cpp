#include "Textureloader.h"
#include <iostream>
#include <string>
#include <vector>

namespace Flux
{
std::unordered_map<std::string, unsigned int> TextureLoader::cache;

SDL_GPUTexture* Flux::TextureLoader::LoadCubemap(SDL_GPUDevice* device, std::vector<std::string> faces)
{
    if (!device || faces.size() != 6) return nullptr;

    int width = 0, height = 0, channels = 0;

    unsigned char* firstFaceData = stbi_load(faces[0].c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (!firstFaceData) {
        std::cerr << "Cubemap face 0 failrd to load at path: " << faces[0] << "\n";
        return nullptr;
    }

    uint32_t faceSize = width * height * 4;

    SDL_GPUTextureCreateInfo texInfo = {};
    texInfo.type = SDL_GPU_TEXTURETYPE_CUBE;
    texInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    texInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
    texInfo.width = static_cast<uint32_t>(width);
    texInfo.height = static_cast<uint32_t>(height);
    texInfo.layer_count_or_depth = 6;
    texInfo.num_levels = 1;

    SDL_GPUTexture* cubeTexture = SDL_CreateGPUTexture(device, &texInfo);
    if (!cubeTexture) {
        stbi_image_free(firstFaceData);
        return nullptr;
    }

    SDL_GPUTransferBufferCreateInfo xferInfo = {};
    xferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    xferInfo.size = faceSize * 6;
    SDL_GPUTransferBuffer* xferBuffer = SDL_CreateGPUTransferBuffer(device, &xferInfo);

    uint8_t* mapPtr = static_cast<uint8_t*>(SDL_MapGPUTransferBuffer(device, xferBuffer, false));

    memcpy(mapPtr, firstFaceData, faceSize);
    stbi_image_free(firstFaceData);

    for (uint32_t i = 1; i < 6; i++) {
        int w = 0, h = 0, c = 0;
        unsigned char* data = stbi_load(faces[i].c_str(), &w, &h, &c, STBI_rgb_alpha);
        
        if (data) {
            memcpy(mapPtr + (i * faceSize), data, faceSize);
            stbi_image_free(data);
        } else {
            std::cerr << "Cubemap face couldn't be loaded at path: " << faces[i] << "\n";
        }

        SDL_UnmapGPUTransferBuffer(device, xferBuffer);

        SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
        SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmd);

        for (uint32_t i = 0; i < 6; i++) {
            SDL_GPUTextureTransferInfo srcInfo = {};
            srcInfo.transfer_buffer = xferBuffer;
            srcInfo.offset = i * faceSize;

            SDL_GPUTextureRegion dstRegion = {};
            dstRegion.texture = cubeTexture;
            dstRegion.layer = i; // Target face layer (0 = +X, 1 = -X, 2 = +Y, 3 = -Y, 4 = +Z, 5 = -Z)
            dstRegion.w = static_cast<uint32_t>(width);
            dstRegion.h = static_cast<uint32_t>(height);
            dstRegion.d = 1;
        }
    }
}

unsigned int TextureLoader::Load(const std::string &path)
{
    auto it = cache.find(path);
    if (it != cache.end())
        return it->second;

    size_t fileSize;
    void *fileBuffer = SDL_LoadFile(path.c_str(), &fileSize);

    if (!fileBuffer)
    {
        std::cerr << "Failed to load file: " << path << " - " << SDL_GetError() << std::endl;
        Output::addLog("SDL3 ERROR: Failed to load texture file: " + path);
        return 0;
    }

    stbi_set_flip_vertically_on_load(true);
    int w, h, ch;

    unsigned char *data = stbi_load_from_memory((unsigned char *)fileBuffer, (int)fileSize, &w, &h, &ch, 0);

    SDL_free(fileBuffer);

    if (!data)
    {
        std::cerr << "Failed to parse image: " << path << std::endl;
        Output::addLog("STB_IMAGE ERROR: Failed to parse texture file: " + path);
        return 0;
    }

    GLenum fmt = (ch == 4) ? GL_RGBA : (ch == 3) ? GL_RGB : GL_RED;

    unsigned int id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    cache[path] = id;
    return id;
}

unsigned int TextureLoader::LoadFromMemory(const std::string &cacheKey, const unsigned char *data, size_t size,
                                           bool *outHasAlpha)
{
    auto it = cache.find(cacheKey);
    if (it != cache.end())
    {
        if (outHasAlpha)
        {
            int w, h, ch;
            stbi_set_flip_vertically_on_load(true);
            unsigned char *probe = stbi_load_from_memory(data, (int)size, &w, &h, &ch, 4);
            if (probe)
            {
                bool hasAlpha = false;
                const int total = w * h;
                for (int i = 0; i < total; ++i)
                {
                    if (probe[i * 4 + 3] < 254)
                    {
                        hasAlpha = true;
                        break;
                    }
                }
                *outHasAlpha = hasAlpha;
                stbi_image_free(probe);
            }
        }
        return it->second;
    }

    stbi_set_flip_vertically_on_load(true);
    int w, h, ch;
    unsigned char *img = stbi_load_from_memory(data, (int)size, &w, &h, &ch, 4);
    if (!img)
    {
        std::cerr << "Failed to parse embedded texture: " << cacheKey << std::endl;
        Output::addLog("STB_IMAGE ERROR: Failed to parse embedded texture: " + cacheKey);
        return 0;
    }

    if (outHasAlpha)
    {
        bool hasAlpha = false;
        const int total = w * h;
        for (int i = 0; i < total; ++i)
        {
            if (img[i * 4 + 3] < 254)
            {
                hasAlpha = true;
                break;
            }
        }
        *outHasAlpha = hasAlpha;
    }

    unsigned int id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(img);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    cache[cacheKey] = id;
    return id;
}

SDL_GPUTexture* TextureLoader::LoadFromMemoryRaw(SDL_GPUDevice* device, const std::string& cacheKey, const unsigned char* data, int width, int height, SDL_GPUTextureFormat* format)
{
    if (!device || !data || width <= 0 || height <= 0) return nullptr;

    if (cache.contains(cacheKey)) {
        return cache[cacheKey];
    }
}

void TextureLoader::Unload(const std::string &path)
{
    auto it = cache.find(path);
    if (it == cache.end())
        return;
    glDeleteTextures(1, &it->second);
    cache.erase(it);
}

} // namespace Flux