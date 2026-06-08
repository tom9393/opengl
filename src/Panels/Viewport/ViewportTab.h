#pragma once

namespace Engine {

    // A tab shown in the Viewport tab bar. Each tab renders its own contents and carries
    // an edit-mode lifecycle: a Viewport tab change calls Exit on the tab being left and
    // Enter on the one being opened. Enter/Exit default to no-ops (Layout); the edit tabs
    // (Modeling, UV) override them to begin/commit their MeshEditSession.
    class ViewportTab {
    public:
        virtual ~ViewportTab() = default;

        virtual void OnImGuiRender() = 0;

        virtual void Enter() {}
        virtual void Exit() {}
    };

}
