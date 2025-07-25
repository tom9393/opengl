#pragma once

#include "Layer.h"

class ImGuiLayer : public Layer
{
    public:
    ~ImGuiLayer();

    void OnAttach() override;
    void OnDetach() override;
    void OnImGuiRender() override;

    void Begin();
    void End();

    private:
    float m_Time = 0.0f;
};
