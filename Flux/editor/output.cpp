#include "output.h"

namespace Flux
{
void Output::addLog(const std::string &log)
{
    logBuffer += log + "\n";
}

void Output::renderOutput()
{
    ImGui::Begin("Console");

    if (ImGui::Button("Clear"))
    {
        logBuffer.clear();
    }
    ImGui::SameLine();
    static bool autoScroll = true;
    ImGui::Checkbox("Auto-scroll", &autoScroll);

    ImGui::Separator();

    if (ImGui::BeginChild("ConsoleScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar))
    {
        size_t start = 0;
        size_t end = logBuffer.find('\n');
        while (end != std::string::npos)
        {
            std::string line = logBuffer.substr(start, end - start);
            if (!line.empty())
            {
                ImVec4 color = ImVec4(0.85f, 0.85f, 0.85f, 1.00f); // Default (white/light grey)
                if (line.find("Warning") != std::string::npos || line.find("WARNING") != std::string::npos)
                {
                    color = ImVec4(1.0f, 0.8f, 0.2f, 1.0f); // Yellow
                }
                else if (line.find("ERROR") != std::string::npos || line.find("Error") != std::string::npos || line.find("FATAL") != std::string::npos || line.find("Failed") != std::string::npos)
                {
                    color = ImVec4(1.0f, 0.35f, 0.3f, 1.0f); // Red/Orange
                }
                else if (line.find("SUCCESS") != std::string::npos || line.find("success") != std::string::npos || line.find("Imported") != std::string::npos)
                {
                    color = ImVec4(0.2f, 0.7f, 0.3f, 1.00f); // Green
                }

                ImGui::TextColored(color, "%s", line.c_str());
            }
            start = end + 1;
            end = logBuffer.find('\n', start);
        }

        if (autoScroll)
        {
            ImGui::SetScrollHereY(1.0f);
        }
    }
    ImGui::EndChild();
    ImGui::End();
}
} // namespace Flux