#pragma once

#include "imgui.h"

namespace Flux
{
enum class EditorTheme : int
{
    Default = 0,
    Light = 1,
    ClassicImGui = 2,
    Legacy = 3
};

struct StalkerthemePreset
{
    ImVec2 windowPadding = ImVec2(10.0f, 10.0f);
    ImVec2 framePadding = ImVec2(20.0f, 8.0f);
    ImVec2 itemSpacing = ImVec2(10.0f, 8.0f);
    float scrollbarSize = 17.0f;
    float scrollbarRounding = 12.0f;
    float frameRounding = 8.0f;
    float grabRounding = 8.0f;
    float tabRounding = 8.0f;
    float windowRounding = 8.0f;
    float childRounding = 0.0f;
    float popupRounding = 8.0f;

    float windowBorderSize = 1.0f;
    float childBorderSize = 1.0f;
    float popupBorderSize = 1.0f;
    float frameBorderSize = 1.0f;
    float tabBorderSize = 1.0f;

    ImVec4 mainBgLight0 = ImVec4(0.404f, 0.404f, 0.404f, 1.0f);
    ImVec4 mainBg = ImVec4(0.21f, 0.21f, 0.21f, 1.0f);
    ImVec4 mainBgDark0 = ImVec4(0.190f, 0.190f, 0.190f, 1.0f);
    ImVec4 mainBgDark1 = ImVec4(0.145f, 0.145f, 0.145f, 1.0f);
    ImVec4 mainBgDark2 = ImVec4(0.098f, 0.098f, 0.098f, 1.0f);

    ImVec4 accentDark0 = ImVec4(0.102f, 0.102f, 0.102f, 1.0f);

    ImVec4 header = ImVec4(0.338f, 0.338f, 0.338f, 1.0f);
    ImVec4 headerHovered = ImVec4(0.276f, 0.276f, 0.276f, 1.0f);
    ImVec4 headerActive = ImVec4(0.379f, 0.379f, 0.379f, 1.0f);

    ImVec4 font = ImVec4(0.902f, 0.902f, 0.902f, 1.0f);
    ImVec4 fontDisabled = ImVec4(0.36f, 0.36f, 0.36f, 1.0f);
    ImVec4 highlight = ImVec4(0.145f, 0.553f, 0.384f, 1.0f);
};

struct LegacyThemePreset
{
    ImVec4 windowBg = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    ImVec4 childBg = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    ImVec4 popupBg = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);

    ImVec4 frameBg = ImVec4(0.20f, 0.21f, 0.22f, 0.54f);
    ImVec4 frameBgHovered = ImVec4(0.29f, 0.59f, 0.83f, 0.40f);
    ImVec4 frameBgActive = ImVec4(0.29f, 0.59f, 0.83f, 0.67f);

    ImVec4 titleBg = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    ImVec4 titleBgActive = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
    ImVec4 titleBgCollapsed = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);

    ImVec4 menuBarBg = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);

    ImVec4 scrollbarBg = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    ImVec4 scrollbarGrab = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
    ImVec4 scrollbarGrabHovered = ImVec4(0.29f, 0.59f, 0.83f, 0.60f);
    ImVec4 scrollbarGrabActive = ImVec4(0.29f, 0.59f, 0.83f, 0.90f);

    ImVec4 accent = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
    ImVec4 accentHovered = ImVec4(0.29f, 0.59f, 0.83f, 0.80f);
    ImVec4 accentActive = ImVec4(0.29f, 0.59f, 0.83f, 1.00f);

    ImVec4 separator = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    ImVec4 separatorHovered = ImVec4(0.29f, 0.59f, 0.83f, 0.78f);
    ImVec4 separatorActive = ImVec4(0.29f, 0.59f, 0.83f, 1.00f);

    ImVec4 resizeGrip = ImVec4(0.29f, 0.59f, 0.83f, 0.20f);
    ImVec4 resizeGripHovered = ImVec4(0.29f, 0.59f, 0.83f, 0.67f);
    ImVec4 resizeGripActive = ImVec4(0.29f, 0.59f, 0.83f, 0.95f);

    ImVec4 tab = ImVec4(0.18f, 0.35f, 0.58f, 0.86f);
    ImVec4 tabHovered = ImVec4(0.29f, 0.59f, 0.83f, 0.80f);
    ImVec4 tabActive = ImVec4(0.20f, 0.41f, 0.68f, 1.00f);
    ImVec4 tabUnfocused = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
    ImVec4 tabUnfocusedActive = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);

    ImVec4 dockingPreview = ImVec4(0.29f, 0.59f, 0.83f, 0.70f);
};

void ApplyTheme(EditorTheme theme);
inline void ApplyTheme(int themeIndex)
{
    ApplyTheme(static_cast<EditorTheme>(themeIndex));
}
} // namespace Flux