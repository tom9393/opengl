#pragma once

#include "../../include/IndexBuffer.h"

class OpenGLIndexBuffer : public IndexBuffer
{
    public:
    OpenGLIndexBuffer(unsigned int* indices, unsigned int size);
    ~OpenGLIndexBuffer();

    void Bind() const;
    void Unbind() const;

    unsigned int GetCount() const { return m_Count; }

    private:
    unsigned int m_RendererID;
    unsigned int m_Count;
};
