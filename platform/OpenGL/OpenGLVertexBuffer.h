#pragma once

#include "../../include/BufferLayout.h"
#include "../../include/VertexBuffer.h"

class OpenGLVertexBuffer : public VertexBuffer
{
    public:
    OpenGLVertexBuffer(float* vertices, unsigned int size);
    ~OpenGLVertexBuffer();
    void Bind() const override;
    void Unbind() const override;
    const BufferLayout& GetLayout() const override { return m_Layout; }
    void SetLayout(const BufferLayout &layout) override { m_Layout = layout; }

    private:
    unsigned int m_RendererID;
    BufferLayout m_Layout;
};
