#include "pch.h"
#include "OpenGL/OpenGLRendererAPI.h"

namespace Engine
{

    void OpenGLRendererAPI::Init()
    {
    };

    void OpenGLRendererAPI::SetClearColor(const glm::vec4 &color)
    {
        glClearColor(color.r, color.g, color.b, color.a);
    };

    void OpenGLRendererAPI::Clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    };

    void OpenGLRendererAPI::DrawIndexed(const std::shared_ptr<VertexArray> &vertexArray)
    {
        glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
    };

    void OpenGLRendererAPI::DrawIndexedInstances(const std::shared_ptr<VertexArray> &vertexArray, unsigned int count)
    {
        glDrawElementsInstanced(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr, count);
    };

    void OpenGLRendererAPI::DrawLines(const std::shared_ptr<VertexArray> &vertexArray, uint32_t size)
    {
        vertexArray->Bind();
        glDrawArrays(GL_LINES, 0, size);
    }

    void OpenGLRendererAPI::DrawPoints(const std::shared_ptr<VertexArray> &vertexArray, uint32_t size)
    {
        glEnable(GL_PROGRAM_POINT_SIZE);
        vertexArray->Bind();
        glDrawArrays(GL_POINTS, 0, size);
    }

    static uint32_t s_QuadVAO = 0;
    static uint32_t s_QuadVBO = 0;

    void OpenGLRendererAPI::DrawFullscreenQuad()
    {
        // For now, fix this later
        if (s_QuadVAO == 0)
        {
            // A fullscreen quad defined in Normalized Device Coordinates (NDC)
            // spanning from -1 to 1 on both X and Y axes.
            float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
                 1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
                 1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
            };

            glGenVertexArrays(1, &s_QuadVAO);
            glGenBuffers(1, &s_QuadVBO);

            glBindVertexArray(s_QuadVAO);

            glBindBuffer(GL_ARRAY_BUFFER, s_QuadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

            // Layout location 0: a_Position (vec3)
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

            // Layout location 1: a_TexCoords (vec2)
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        }

        // Bind the VAO and draw the quad as a triangle strip
        glBindVertexArray(s_QuadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }

    void OpenGLRendererAPI::SetViewport(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
    {
        glViewport(x, y, width, height);
    };

    void OpenGLRendererAPI::ToggleWireframe()
    {
        if (m_Wireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        m_Wireframe = !m_Wireframe;
    };

};
