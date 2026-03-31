#pragma once

#include "../../src/Render/Context.h"

namespace Engine
{

    class OpenGLContext : public Context
    {
        public:
        OpenGLContext(GLFWwindow* window);
        void Init() override;
        void SwapBuffers() override;

        private:
        GLFWwindow* m_WindowHandle;
    };

};
