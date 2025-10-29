#pragma once

class ScopedImGuiStyle
{
    public:
    ScopedImGuiStyle(ImGuiStyleVar var, const ImVec2& value) { ImGui::PushStyleVar(var, value); }
    ~ScopedImGuiStyle() { ImGui::PopStyleVar(); }
};
