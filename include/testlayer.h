#pragma once

#include "Layer.h"

class Testlayer : public Layer
{
    public:
    ~Testlayer();

    void OnAttach();
    void OnDetach();
    void OnUpdate();
    void OnImGuiRender();
    void OnEvent(Event& event);
};
