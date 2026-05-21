#pragma once

#include "pch.h"
#include "Scene/Scene.h"
#include "Scene/SceneRenderer.h"
#include "Render/EditorCamera.h"
#include "Events/MouseEvent.h"

namespace Engine {

    class PassPanel;

    struct ViewportContext {
        Shared<Scene> SceneContext;
        Shared<SceneRenderer> SceneRendererContext;
        Shared<EditorCamera> Camera;
    };

    class ViewportPanelBase {
    public:
        ViewportPanelBase();
        virtual ~ViewportPanelBase();

        void SetViewportContext(const ViewportContext& ctx);
        void SetPassPanel(PassPanel* passPanel);

        void OnImGuiRender();

        bool IsHovered() const { return m_IsHovered; }
        bool IsFocused() const { return m_IsFocused; }
        glm::vec2 GetSize() const { return m_ViewportSize; }

    protected:
        virtual void OnDraw() {}
        virtual bool OnMousePressed(MouseButtonPressedEvent&) { return false; }

        unsigned int GetDisplayedTextureID();

    protected:
        Shared<Scene> m_Context;
        Shared<SceneRenderer> m_SceneRenderer;
        Shared<EditorCamera> m_EditorCamera;
        PassPanel* m_PassPanel = nullptr;

        bool m_IsHovered = false;
        bool m_IsFocused = false;
        glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
        glm::vec2 m_ViewportBounds[2] = { {0.0f, 0.0f}, {0.0f, 0.0f} };

    private:
        bool OnMouseButtonPressedDispatch(MouseButtonPressedEvent& e);
    };

}
