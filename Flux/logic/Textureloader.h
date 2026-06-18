#pragma once
#include <glad/glad.h>
#include <string>
#include <vector>
#include <unordered_map>
#include "stb_image.h"
#include <SDL3/SDL.h>
#include "editor/output.h"

namespace Flux {

    class Output;
    class TextureLoader {
    public:
        static TextureLoader& Get() {
            static TextureLoader instance;
            return instance;
        }

        static unsigned int Load(const std::string& path);
        void Unload(const std::string& path);
        static unsigned int LoadCubemap(std::vector<std::string> faces);
        static unsigned int LoadFromMemory(const std::string& cacheKey, const unsigned char* data, size_t size, bool* outHasAlpha = nullptr);

        static unsigned int LoadFromMemoryRaw(const std::string& cacheKey, const unsigned char* data, int width, int height, GLenum srcFormat = GL_BGRA);

        static std::unordered_map<std::string, unsigned int> cache;
    private:
        
    };

}