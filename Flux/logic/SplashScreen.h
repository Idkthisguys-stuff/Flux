#pragma once
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"
#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <string>

#include "editor/output.h"
#include "stb_image.h"

#include "core/pathHelper.h"
namespace Flux
{

struct SplashConfig
{
    const char *title = "Flux Engine";
    const char *subtitle = "v0.1 Alpha";
    float durationSec = 2.8f;
    ImVec4 bgColor = {0.0f, 0.0f, 0.0f, 1.f};
    ImVec4 titleColor = {1.f, 0.85f, 0.35f, 1.f};
    ImVec4 subColor = {0.6f, 0.6f, 0.6f, 1.f};
    ImVec4 barColor = {1.f, 0.85f, 0.35f, 1.f};
    float barHeight = 4.f;
    float titleSize = 2.4f;
    float subSize = 1.1f;
};

inline void RunSplashScreen(SDL_Window *window, const SplashConfig &cfg = {})
{

    GLuint textureID = 0;
    {
        int imgW, imgH, channels;
        unsigned char *data = stbi_load(PathHelper::GetAssetPath("assets/splash/iconSplash.png").c_str(), &imgW, &imgH, &channels, 4);
        if (data)
        {
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgW, imgH, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
    }

    const char *vertSrc = R"(#version 330 core
        layout(location = 0) in vec2 aPos;
        layout(location = 1) in vec2 aUV;
        out vec2 vUV;
        void main() {
            vUV = aUV;
            gl_Position = vec4(aPos, 0.0, 1.0);
        }
    )";

    const char *fragSrc = R"(#version 330 core
        in vec2 vUV;
        out vec4 FragColor;
        uniform sampler2D uTex;
        uniform float uAlpha;
        void main() {
            vec4 c = texture(uTex, vUV);
            FragColor = vec4(c.rgb, c.a * uAlpha);
        }
    )";

    auto compileShader = [](GLenum type, const char *src) -> GLuint {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        return s;
    };

    GLuint vert = compileShader(GL_VERTEX_SHADER, vertSrc);
    GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragSrc);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);
    glDeleteShader(vert);
    glDeleteShader(frag);

    GLint alphaLoc = glGetUniformLocation(prog, "uAlpha");

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glBindVertexArray(0);

    const float imagePixels = 400.0f;
    const Uint64 start = SDL_GetTicks();
    const float fadeStart = 0.72f;
    bool shouldQuit = false;

    while (!shouldQuit)
    {
        float elapsed = (SDL_GetTicks() - start) / 1000.0f;
        float t = elapsed / cfg.durationSec;
        if (t >= 1.f)
            break;

        float alpha = (t < fadeStart) ? 1.0f : 1.0f - ((t - fadeStart) / (1.0f - fadeStart));

        SDL_Event e;
        while (SDL_PollEvent(&e))
            if (e.type == SDL_EVENT_QUIT)
                shouldQuit = true;

        int fbW, fbH;
        SDL_GetWindowSizeInPixels(window, &fbW, &fbH);

        float ndcHalfW = imagePixels / (float)fbW;
        float ndcHalfH = imagePixels / (float)fbH;

        float quad[24] = {

            -ndcHalfW, -ndcHalfH, 0.0f, 0.0f, ndcHalfW, -ndcHalfH, 1.0f, 0.0f, ndcHalfW,  ndcHalfH, 1.0f, 1.0f,

            -ndcHalfW, -ndcHalfH, 0.0f, 0.0f, ndcHalfW, ndcHalfH,  1.0f, 1.0f, -ndcHalfW, ndcHalfH, 0.0f, 1.0f};

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad), quad);

        glViewport(0, 0, fbW, fbH);
        glClearColor(cfg.bgColor.x, cfg.bgColor.y, cfg.bgColor.z, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);

        glUseProgram(prog);
        glUniform1i(glGetUniformLocation(prog, "uTex"), 0);
        glUniform1f(alphaLoc, alpha);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        SDL_GL_SwapWindow(window);
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(prog);
    if (textureID)
        glDeleteTextures(1, &textureID);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glUseProgram(0);
}

} // namespace Flux