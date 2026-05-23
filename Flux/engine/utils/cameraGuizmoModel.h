#pragma once
#include <glm/glm.hpp>
#include <imgui.h>
#include <vector>


namespace Flux
{

struct CameraGizmoRenderer
{

    static void Draw(const glm::mat4 &modelMatrix, const glm::mat4 &view, const glm::mat4 &proj,
                     ImU32 color = IM_COL32(255, 255, 0, 255))
    {
        ImDrawList *drawList = ImGui::GetWindowDrawList();
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();

        std::vector<glm::vec3> localVertices = {

            {-0.20f, 0.15f, 0.40f}, {0.20f, 0.15f, 0.40f}, {0.20f, -0.15f, 0.40f}, {-0.20f, -0.15f, 0.40f},
            {-0.20f, 0.15f, 0.15f}, {0.20f, 0.15f, 0.15f}, {0.20f, -0.15f, 0.15f}, {-0.20f, -0.15f, 0.15f},

            {0.00f, 0.00f, 0.00f}};

        std::vector<ImVec2> screenPoints(localVertices.size());
        glm::mat4 mvp = proj * view * modelMatrix;

        for (size_t i = 0; i < localVertices.size(); ++i)
        {
            glm::vec4 clipSpace = mvp * glm::vec4(localVertices[i], 1.0f);

            if (clipSpace.w <= 0.0f)
                clipSpace.w = 0.0001f;

            glm::vec3 ndc = glm::vec3(clipSpace) / clipSpace.w;

            float screenX = windowPos.x + (ndc.x + 1.0f) * 0.5f * windowSize.x;
            float screenY = windowPos.y + (1.0f - ndc.y) * 0.5f * windowSize.y;
            screenPoints[i] = ImVec2(screenX, screenY);
        }

        auto DrawLine = [&](int idxA, int idxB) {
            drawList->AddLine(screenPoints[idxA], screenPoints[idxB], color, 2.0f);
        };

        DrawLine(0, 1);
        DrawLine(1, 2);
        DrawLine(2, 3);
        DrawLine(3, 0);

        DrawLine(4, 5);
        DrawLine(5, 6);
        DrawLine(6, 7);
        DrawLine(7, 4);

        DrawLine(0, 4);
        DrawLine(1, 5);
        DrawLine(2, 6);
        DrawLine(3, 7);

        DrawLine(4, 8);
        DrawLine(5, 8);
        DrawLine(6, 8);
        DrawLine(7, 8);
    }
};

} // namespace Flux