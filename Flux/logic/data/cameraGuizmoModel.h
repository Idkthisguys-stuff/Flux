
#pragma once
#include <cmath>
#include <glm/glm.hpp>
#include <imgui.h>

namespace Flux
{

struct CameraGizmoRenderer
{
    static ImVec2 Project(const glm::vec3 &local, const glm::mat4 &mvp, ImVec2 imagePos, ImVec2 sz)
    {
        glm::vec4 clip = mvp * glm::vec4(local, 1.0f);
        if (clip.w <= 0.0f)
            clip.w = 0.0001f;
        glm::vec3 ndc = glm::vec3(clip) / clip.w;
        return ImVec2(imagePos.x + (ndc.x + 1.0f) * 0.5f * sz.x, imagePos.y + (1.0f - ndc.y) * 0.5f * sz.y);
    }

    static void Draw(const glm::mat4 &modelMatrix, const glm::mat4 &view, const glm::mat4 &proj, ImVec2 imagePos,
                     ImVec2 sz, float fov, ImU32 color = IM_COL32(0, 220, 255, 220))
    {
        ImDrawList *dl = ImGui::GetWindowDrawList();
        glm::mat4 mvp = proj * view * modelMatrix;

        auto P = [&](glm::vec3 v) { return Project(v, mvp, imagePos, sz); };

        const float depth = 0.8f;

        const float halfH = std::tan(glm::radians(fov * 0.5f)) * depth;

        const float halfW = halfH * (16.0f / 9.0f);

        glm::vec3 apex = {0.f, 0.f, 0.f};
        glm::vec3 tl = {-halfW, halfH, -depth};
        glm::vec3 tr = {halfW, halfH, -depth};
        glm::vec3 br = {halfW, -halfH, -depth};
        glm::vec3 bl = {-halfW, -halfH, -depth};

        dl->AddLine(P(apex), P(tl), color, 1.8f);
        dl->AddLine(P(apex), P(tr), color, 1.8f);
        dl->AddLine(P(apex), P(br), color, 1.8f);
        dl->AddLine(P(apex), P(bl), color, 1.8f);

        dl->AddLine(P(tl), P(tr), color, 1.8f);
        dl->AddLine(P(tr), P(br), color, 1.8f);
        dl->AddLine(P(br), P(bl), color, 1.8f);
        dl->AddLine(P(bl), P(tl), color, 1.8f);
    }
};

} // namespace Flux