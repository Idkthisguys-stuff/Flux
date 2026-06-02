 

#pragma once
#include <glad/glad.h>

namespace Flux {
    class OpenGLManager {
    public:
        void Init(int width, int height);
        void Resize(int width, int height);
        void Bind();
        void Unbind();
        unsigned int GetTexture() { return textureColorBuffer; }

    private:
        unsigned int fbo, rbo, textureColorBuffer;
    };
}