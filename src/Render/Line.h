#pragma once

#include "pch.h"
#include "Buffer.h"
#include "Shader.h"

namespace Engine {

    struct LineVertex {
        glm::vec3 Position;
        glm::vec4 Color;
    };

    struct LineData {
        static const uint32_t MaxLines = 10000;
        static const uint32_t MaxVertices = MaxLines * 2;

        Shared<VertexArray> LineVertexArray;
        Shared<VertexBuffer> LineVertexBuffer;
        Shared<Shader> LineShader;

        uint32_t VertexCount = 0;
        LineVertex* VertexBufferBase = nullptr;
        LineVertex* VertexBufferPtr = nullptr;

        void Init() {
            LineVertexArray = VertexArray::Create();
            LineVertexBuffer = VertexBuffer::Create(MaxLines * sizeof(LineVertex));

            LineVertexBuffer->SetLayout({
                { ShaderDataType::Float3, "a_Position" },
                { ShaderDataType::Float4, "a_Color"    }
            });

            LineVertexArray->AddVertexBuffer(LineVertexBuffer);
            
            VertexBufferBase = new LineVertex[MaxVertices];
            VertexBufferPtr = VertexBufferBase;
            VertexCount = 0;

            LineShader = Shader::Create("LineShader");
            const std::string vert = "/home/tom/Coding/c++/opengl/shaders/line.vert";
            const std::string frag = "/home/tom/Coding/c++/opengl/shaders/line.frag";
            LineShader->AddShaders({ vert, frag });
            ShaderLibrary::Add(LineShader);
        }

        void Shutdown() {
            delete[] VertexBufferBase;
            VertexBufferBase = nullptr;
            VertexBufferPtr = nullptr;
        }

        void ResetBatch() {
            VertexCount = 0;
            VertexBufferPtr = VertexBufferBase;
        }

        bool IsFull() const {
            return VertexCount + 2 >= MaxVertices;
        }

        void AddVertex(const glm::vec3& position, const glm::vec4& color) {
            VertexBufferPtr->Position = position;
            VertexBufferPtr->Color = color;
            VertexBufferPtr++;
            VertexCount++;
        }

        uint32_t GetDataSize() const {
            return (uint32_t)((uint8_t*)VertexBufferPtr - (uint8_t*)VertexBufferBase);
        }
    };
}