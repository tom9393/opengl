#include "../../pch.h"
#include "OpenGLRendererAPI.h"

namespace Engine
{

    static const size_t MaxInstances = 100000;

    void OpenGLRendererAPI::Init()
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glGenBuffers(1, &m_DynamicVBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_DynamicVBO);
        
        // Allocate memory (but don't fill it yet), using GL_DYNAMIC_DRAW since it changes every frame
        glBufferData(GL_ARRAY_BUFFER, MaxInstances * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
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

    void OpenGLRendererAPI::DrawIndexedInstances(const std::shared_ptr<VertexArray> &vertexArray, const std::vector<glm::mat4> &transforms)
    {
        // 2. Upload the Transform Array to our Dynamic VBO
        glBindBuffer(GL_ARRAY_BUFFER, m_DynamicVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, transforms.size() * sizeof(glm::mat4), transforms.data());

        // 3. Configure the VAO for the mat4 attributes (Locations 5, 6, 7, 8)
        // We have to enable 4 vec4s to make up the mat4
        std::size_t vec4Size = sizeof(glm::vec4);
        for (int i = 0; i < 4; i++) 
        {
            glEnableVertexAttribArray(5 + i);
            glVertexAttribPointer(5 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (const void*)(i * vec4Size));
            
            // THE MAGIC LINE: This tells OpenGL to advance this attribute once per INSTANCE, not per vertex!
            glVertexAttribDivisor(5 + i, 1); 
        }

        // 4. Fire the Instanced Draw Call!
        glDrawElementsInstanced(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr, transforms.size());

        // Clean up to avoid affecting other non-instanced draw calls
        for (int i = 0; i < 4; i++) {
            glVertexAttribDivisor(5 + i, 0); 
        }
    };

    void OpenGLRendererAPI::DrawLines(const std::shared_ptr<VertexArray> &vertexArray, uint32_t size)
    {
        vertexArray->Bind();
        glDrawArrays(GL_LINES, 0, size);
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
