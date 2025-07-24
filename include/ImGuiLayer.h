#pragma once

#include "Layer.h"

class ImGuiLayer : public Layer
{
    public:
    ~ImGuiLayer();

    void OnAttach();
    void OnDetach();
    void OnUpdate();
    void OnImGuiRender();
    void OnEvent(Event& event);

    private:
    float m_Time = 0.0f;
};
