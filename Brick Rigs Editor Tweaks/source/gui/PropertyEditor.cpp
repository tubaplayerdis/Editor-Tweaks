#include <imgui/imgui.h>

#ifdef _DEBUG
const char* BuildMode = "Debug";
#else
const char* BuildMode = "Release";
#endif // _DEBUG

constexpr int MAIN_MENU_WIDTH = 400;
constexpr int MAIN_MENU_HEIGHT = 250;

#include "../../Include/gui/Menus.h"

void property_editor_function()
{
    ImGui::SetNextWindowSize(ImVec2(MAIN_MENU_WIDTH, MAIN_MENU_HEIGHT), ImGuiCond_Appearing);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f); // round window
    if (ImGui::Begin("Editor Tweaks - Property Editor"))
    {
        ImGui::Text("Version: 0.0.1");
        ImGui::Text("Build Mode: %s", BuildMode);
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

DEFINE_GUI_MENU(menus::property_editor, property_editor_function)