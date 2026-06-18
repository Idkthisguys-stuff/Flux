#include "Themes.h"

namespace Flux
{

static void ApplyStalkerTheme(ImGuiStyle &style)
{
    const StalkerthemePreset preset;

    ImGui::StyleColorsDark(&style);

    style.WindowPadding = preset.windowPadding;
    style.FramePadding = preset.framePadding;
    style.ItemSpacing = preset.itemSpacing;
    style.ScrollbarSize = preset.scrollbarSize;
    style.ScrollbarRounding = preset.scrollbarRounding;
    style.FrameRounding = preset.frameRounding;
    style.GrabRounding = preset.grabRounding;
    style.TabRounding = preset.tabRounding;
    style.WindowRounding = preset.windowRounding;
    style.ChildRounding = preset.childRounding;
    style.PopupRounding = preset.popupRounding;

    style.WindowBorderSize = preset.windowBorderSize;
    style.ChildBorderSize = preset.childBorderSize;
    style.PopupBorderSize = preset.popupBorderSize;
    style.FrameBorderSize = preset.frameBorderSize;
    style.TabBorderSize = preset.tabBorderSize;

    ImVec4 *colors = style.Colors;

    colors[ImGuiCol_WindowBg] = preset.mainBg;
    colors[ImGuiCol_ChildBg] = preset.mainBg;

    colors[ImGuiCol_Text] = preset.font;
    colors[ImGuiCol_TextDisabled] = preset.fontDisabled;
    colors[ImGuiCol_TextSelectedBg] = preset.highlight;

    colors[ImGuiCol_FrameBg] = preset.mainBgDark1;
    colors[ImGuiCol_FrameBgHovered] = preset.mainBgDark0;
    colors[ImGuiCol_FrameBgActive] = preset.mainBgDark2;

    colors[ImGuiCol_TitleBg] = preset.mainBgDark0;
    colors[ImGuiCol_TitleBgCollapsed] = preset.mainBgDark0;
    colors[ImGuiCol_TitleBgActive] = preset.mainBgDark0;
    colors[ImGuiCol_MenuBarBg] = preset.accentDark0;

    colors[ImGuiCol_Tab] = preset.mainBgDark0;
    colors[ImGuiCol_TabUnfocused] = preset.mainBgDark0;
    colors[ImGuiCol_TabHovered] = preset.mainBgDark1;
    colors[ImGuiCol_TabActive] = preset.mainBgDark1;
    colors[ImGuiCol_TabUnfocusedActive] = preset.mainBgDark1;

    colors[ImGuiCol_ScrollbarBg] = preset.mainBgDark1;
    colors[ImGuiCol_ScrollbarGrab] = preset.font;
    colors[ImGuiCol_ScrollbarGrabActive] = preset.fontDisabled;
    colors[ImGuiCol_ScrollbarGrabHovered] = preset.fontDisabled;
    colors[ImGuiCol_CheckMark] = preset.font;
    colors[ImGuiCol_SliderGrab] = preset.font;
    colors[ImGuiCol_SliderGrabActive] = preset.fontDisabled;

    colors[ImGuiCol_Header] = preset.header;
    colors[ImGuiCol_HeaderHovered] = preset.headerHovered;
    colors[ImGuiCol_HeaderActive] = preset.headerActive;

    colors[ImGuiCol_Separator] = preset.mainBgLight0;
    colors[ImGuiCol_SeparatorHovered] = preset.mainBgLight0;
    colors[ImGuiCol_SeparatorActive] = preset.mainBgLight0;
    colors[ImGuiCol_Border] = preset.mainBgLight0;

    colors[ImGuiCol_ResizeGrip] = preset.mainBg;
    colors[ImGuiCol_ResizeGripHovered] = preset.mainBg;
    colors[ImGuiCol_ResizeGripActive] = preset.mainBg;

    colors[ImGuiCol_DockingPreview] = preset.accentDark0;
    colors[ImGuiCol_NavHighlight] = preset.accentDark0;
}

static void ApplyLegacyTheme(ImGuiStyle &style)
{
    const LegacyThemePreset preset;
    
    ImGui::StyleColorsDark(&style);

    ImVec4 *colors = style.Colors;

    colors[ImGuiCol_WindowBg]         = preset.windowBg;
    colors[ImGuiCol_ChildBg]          = preset.childBg;
    colors[ImGuiCol_PopupBg]          = preset.popupBg;

    colors[ImGuiCol_FrameBg]          = preset.frameBg;
    colors[ImGuiCol_FrameBgHovered]   = preset.frameBgHovered;
    colors[ImGuiCol_FrameBgActive]    = preset.frameBgActive;

    colors[ImGuiCol_TitleBg]          = preset.titleBg;
    colors[ImGuiCol_TitleBgActive]    = preset.titleBgActive;
    colors[ImGuiCol_TitleBgCollapsed] = preset.titleBgCollapsed;
    colors[ImGuiCol_MenuBarBg]        = preset.menuBarBg;

    colors[ImGuiCol_ScrollbarBg]          = preset.scrollbarBg;
    colors[ImGuiCol_ScrollbarGrab]        = preset.scrollbarGrab;
    colors[ImGuiCol_ScrollbarGrabHovered] = preset.scrollbarGrabHovered;
    colors[ImGuiCol_ScrollbarGrabActive]  = preset.scrollbarGrabActive;

    colors[ImGuiCol_CheckMark]        = preset.accentActive;
    colors[ImGuiCol_SliderGrab]       = preset.accent;
    colors[ImGuiCol_SliderGrabActive] = preset.accentActive;

    colors[ImGuiCol_Button]        = preset.accent;
    colors[ImGuiCol_ButtonHovered] = preset.accentHovered;
    colors[ImGuiCol_ButtonActive]  = preset.accentActive;

    colors[ImGuiCol_Header]        = preset.accent;
    colors[ImGuiCol_HeaderHovered] = preset.accentHovered;
    colors[ImGuiCol_HeaderActive]  = preset.accentActive;

    colors[ImGuiCol_Separator]        = preset.separator;
    colors[ImGuiCol_SeparatorHovered] = preset.separatorHovered;
    colors[ImGuiCol_SeparatorActive]  = preset.separatorActive;

    colors[ImGuiCol_ResizeGrip]        = preset.resizeGrip;
    colors[ImGuiCol_ResizeGripHovered] = preset.resizeGripHovered;
    colors[ImGuiCol_ResizeGripActive]  = preset.resizeGripActive;

    colors[ImGuiCol_Tab]                = preset.tab;
    colors[ImGuiCol_TabHovered]         = preset.tabHovered;
    colors[ImGuiCol_TabActive]          = preset.tabActive;
    colors[ImGuiCol_TabUnfocused]       = preset.tabUnfocused;
    colors[ImGuiCol_TabUnfocusedActive] = preset.tabUnfocusedActive;

    colors[ImGuiCol_DockingPreview] = preset.dockingPreview;
}

void ApplyTheme(EditorTheme theme)
{
    ImGuiStyle &style = ImGui::GetStyle();

    style = ImGuiStyle();

    switch (theme)
    {
    case EditorTheme::Light:
        ImGui::StyleColorsLight(&style);
        break;
    case EditorTheme::ClassicImGui:
        ImGui::StyleColorsClassic(&style);
        break;
    case EditorTheme::Legacy:
        ApplyLegacyTheme(style);
        break;
    case EditorTheme::Default:
    default:
        ApplyStalkerTheme(style);
        break;
    }
}

} // namespace Flux