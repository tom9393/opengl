#pragma once

#include "pch.h"
#include "Buffer.h"
#include "Shader.h"

namespace Engine {

    struct PointVertex {
        glm::vec3 Position;
        glm::vec4 Color;
        float Size;
    };

    struct PointData {
        static const uint32_t MaxPoints = 10000;

        Shared<VertexArray> PointVertexArray;
        Shared<VertexBuffer> PointVertexBuffer;
        Shared<Shader> PointShader;

        uint32_t VertexCount = 0;
        PointVertex* VertexBufferBase = nullptr;
        PointVertex* VertexBufferPtr = nullptr;

        void Init() {
            PointVertexArray = VertexArray::Create();
            PointVertexBuffer = VertexBuffer::Create(MaxPoints * sizeof(PointVertex));

            PointVertexBuffer->SetLayout({
                { ShaderDataType::Float3, "a_Position" },
                { ShaderDataType::Float4, "a_Color"    },
                { ShaderDataType::Float,  "a_Size"     }
            });

            PointVertexArray->AddVertexBuffer(PointVertexBuffer);

            VertexBufferBase = new PointVertex[MaxPoints];
            VertexBufferPtr = VertexBufferBase;
            VertexCount = 0;

            PointShader = Shader::Create("PointShader");
            const std::string vert = "/home/tom/Coding/c++/opengl/shaders/point.vert";
            const std::string frag = "/home/tom/Coding/c++/opengl/shaders/point.frag";
            PointShader->AddShaders({ vert, frag });
            ShaderLibrary::Add(PointShader);
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
            return VertexCount + 1 >= MaxPoints;
        }

        void AddVertex(const glm::vec3& position, const glm::vec4& color, float size) {
            VertexBufferPtr->Position = position;
            VertexBufferPtr->Color = color;
            VertexBufferPtr->Size = size;
            VertexBufferPtr++;
            VertexCount++;
        }

        uint32_t GetDataSize() const {
            return (uint32_t)((uint8_t*)VertexBufferPtr - (uint8_t*)VertexBufferBase);
        }
    };
}
