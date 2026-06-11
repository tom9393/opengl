#include "pch.h"
#include "OpenGL/OpenGLRendererAPI.h"
#include "OpenGL/OpenGLBuffer.h"

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

    void OpenGLRendererAPI::ClearColorOnly()
    {
        glClear(GL_COLOR_BUFFER_BIT);
    };

    void OpenGLRendererAPI::SetDepthTest(bool enabled)
    {
        enabled ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
    };

    void OpenGLRendererAPI::SetDepthWrite(bool enabled)
    {
        glDepthMask(enabled ? GL_TRUE : GL_FALSE);
    };

    void OpenGLRendererAPI::SetFaceCulling(bool enabled)
    {
        enabled ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
    };

    void OpenGLRendererAPI::SetAdditiveBlend(bool enabled)
    {
        if (enabled)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            glBlendEquation(GL_FUNC_ADD);
        }
        else
        {
            glDisable(GL_BLEND);
        }
    };

    void OpenGLRendererAPI::SetAlphaBlend(bool enabled)
    {
        if (enabled)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBlendEquation(GL_FUNC_ADD);
        }
        else
        {
            glDisable(GL_BLEND);
        }
    };

    void OpenGLRendererAPI::BindTexture2D(unsigned int slot, unsigned int rendererID)
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, rendererID);
    };

    void OpenGLRendererAPI::BindTextureCube(unsigned int slot, unsigned int rendererID)
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_CUBE_MAP, rendererID);
    };

    void OpenGLRendererAPI::BindTexture(unsigned int slot, unsigned int rendererID, BufferTextureType type)
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(BufferTextureTypeToGL(type), rendererID);
    };

    void OpenGLRendererAPI::PushDebugGroup(const char* name)
    {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name);
    };

    void OpenGLRendererAPI::PopDebugGroup()
    {
        glPopDebugGroup();
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

    static uint32_t s_CubeVAO = 0;
    static uint32_t s_CubeVBO = 0;

    void OpenGLRendererAPI::DrawCube()
    {
        if (s_CubeVAO == 0)
        {
            // A unit cube centered at the origin (position-only), wound so its
            // faces are visible when rendered from the inside (env-map capture).
            float vertices[] = {
                // back face
                -1, -1, -1,  1,  1, -1,  1, -1, -1,  1,  1, -1, -1, -1, -1, -1,  1, -1,
                // front face
                -1, -1,  1,  1, -1,  1,  1,  1,  1,  1,  1,  1, -1,  1,  1, -1, -1,  1,
                // left face
                -1,  1,  1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1,  1,  1,
                // right face
                 1,  1,  1,  1, -1, -1,  1,  1, -1,  1, -1, -1,  1,  1,  1,  1, -1,  1,
                // bottom face
                -1, -1, -1,  1, -1, -1,  1, -1,  1,  1, -1,  1, -1, -1,  1, -1, -1, -1,
                // top face
                -1,  1, -1,  1,  1,  1,  1,  1, -1,  1,  1,  1, -1,  1, -1, -1,  1,  1,
            };

            glGenVertexArrays(1, &s_CubeVAO);
            glGenBuffers(1, &s_CubeVBO);
            glBindVertexArray(s_CubeVAO);
            glBindBuffer(GL_ARRAY_BUFFER, s_CubeVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        }

        glBindVertexArray(s_CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
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
