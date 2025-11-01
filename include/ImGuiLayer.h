#pragma once

#include "Layer.h"

namespace Engine
{

    class ImGuiLayer : public Layer
    {
        public:
        ~ImGuiLayer();

        void OnAttach() override;
        void OnDetach() override;
        void OnImGuiRender() override;
        void OnEvent(Event &e) override;
        void BlockEvents(bool block) { m_BlockEvents = block; }

        void SetDarkTheme();

        void Begin();
        void End();

        private:
        float m_Previous;
        bool m_BlockEvents;
    };

};
