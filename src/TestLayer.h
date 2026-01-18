#pragma once

#include "Buffer.h"
#include "Layer.h"
#include "Texture.h"
#include "TimeStep.h"
#include "Events/MouseEvent.h"
#include "Model.h"
#include "Camera.h"

namespace Engine
{

    class Testlayer : public Layer
    {
        public:
        Testlayer();
        ~Testlayer();

        void OnAttach();
        void OnDetach();
        void OnUpdate(TimeStep ts);
        void OnImGuiRender();
        bool OnMouseButtonPressed(MouseButtonPressedEvent &e);

        void TrySelectMesh(float mouseX, float mouseY, float screenWidth, float screenHeight, std::shared_ptr<Camera> camera, std::shared_ptr<Model> model);

        private:
        std::shared_ptr<Model> m_Model;
        bool m_CameraActive = true;
        float m_Previous = 0;

        std::shared_ptr<Texture2D> m_Texture;
        std::shared_ptr<Mesh> m_SelectedMesh;
    };

};
