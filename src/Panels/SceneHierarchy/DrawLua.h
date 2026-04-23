#pragma once

#include "../../pch.h"
#include "../../Scene/Components.h"
#include "../../Scene/Entity.h"
#include "../../Asset/AssetManager.h"

namespace Engine {

    inline static void DrawLua(Entity &entity, LuaScriptComponent &component)
    {
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetType::LuaScript"))
            {
                UUID handle = *(UUID*)payload->Data;
                component.ScriptHandle = handle;
            }
            ImGui::EndDragDropTarget();
        }
    }
}