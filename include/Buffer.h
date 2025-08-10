#pragma once

#include "../pch.h"
#include "BufferLayout.h"

class VertexBuffer
{
    public:
    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual const BufferLayout& GetLayout() const = 0;
    virtual void SetLayout(const BufferLayout &layout) = 0;
    
    static std::shared_ptr<VertexBuffer> Create(unsigned int size);
    static std::shared_ptr<VertexBuffer> Create(float* vertices, unsigned int size);
};

class IndexBuffer
{
    public:
    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual unsigned int GetCount() const = 0;
    
    static std::shared_ptr<IndexBuffer> Create(unsigned int* indices, unsigned int count);
};

class VertexArray
{
    public:
    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer> &vertexBuffer) = 0;
    virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer> &indexBuffer) = 0;

    virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const = 0;
    virtual const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const = 0;
    
    static std::shared_ptr<VertexArray> Create();
};

class SSBOBuffer
{
    public:
    virtual void Bind(unsigned int index) const = 0;
    virtual void Unbind() const = 0;

    static std::shared_ptr<SSBOBuffer> Create(void* data, unsigned int size);
};

struct FramebufferSpecification
{
    unsigned int width, height;
};

class Framebuffer
{
    public:
    virtual ~Framebuffer() = default;

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual void Resize(unsigned int width, unsigned int height) = 0;

    virtual const FramebufferSpecification &GetSpecification() const = 0;
    virtual const unsigned int GetColorAttachmentRendererID() const = 0;
    static std::shared_ptr<Framebuffer> Create(FramebufferSpecification &specification);
};
