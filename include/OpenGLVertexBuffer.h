#pragma once

#include "VertexBuffer.h"

class OpenGLVertexBuffer : public VertexBuffer
{
    public:
    OpenGLVertexBuffer(float* vertices, uint32_t size);
    ~OpenGLVertexBuffer();
    void Bind() const;
    void Unbind() const;

    private:
    uint32_t m_RendererID;
};
