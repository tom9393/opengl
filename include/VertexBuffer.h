#pragma once

#include "BufferLayout.h"

class VertexBuffer
{
    public:
    ~VertexBuffer();

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual const BufferLayout& GetLayout() const = 0;
    virtual void SetLayout(const BufferLayout &layout) = 0;
    
    static std::shared_ptr<VertexBuffer> Create(unsigned int size);
    static std::shared_ptr<VertexBuffer> Create(float* vertices, unsigned int size);
};
