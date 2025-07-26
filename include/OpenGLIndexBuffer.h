#pragma once

#include "IndexBuffer.h"

class OpenGLIndexBuffer : public IndexBuffer
{
    public:
    OpenGLIndexBuffer(uint32_t* indices, uint32_t size);
    ~OpenGLIndexBuffer();

    void Bind() const;
    void Unbind() const;

    uint32_t GetCount() const { return m_Count; }

    private:
    uint32_t m_RendererID;
    uint32_t m_Count;
};
