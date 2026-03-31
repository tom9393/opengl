#pragma once

#include "../pch.h"
#include "BufferLayout.h"

namespace Engine
{

    class VertexBuffer
    {
        public:
        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        virtual const BufferLayout& GetLayout() const = 0;
        virtual void SetLayout(const BufferLayout &layout) = 0;

        virtual void SetData(const void* data, uint32_t size) = 0;
        
        static Shared<VertexBuffer> Create(unsigned int size);
        static Shared<VertexBuffer> Create(float* vertices, unsigned int size);
    };

    class IndexBuffer
    {
        public:
        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        virtual unsigned int GetCount() const = 0;
        
        static Shared<IndexBuffer> Create(unsigned int* indices, unsigned int count);
    };

    class VertexArray
    {
        public:
        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        virtual void AddVertexBuffer(const Shared<VertexBuffer> &vertexBuffer) = 0;
        virtual void SetIndexBuffer(const Shared<IndexBuffer> &indexBuffer) = 0;

        virtual const std::vector<Shared<VertexBuffer>>& GetVertexBuffers() const = 0;
        virtual const Shared<IndexBuffer>& GetIndexBuffer() const = 0;
        
        static Shared<VertexArray> Create();
    };

    class UniformBuffer
    {
        public:
        virtual void Bind(unsigned int index) const = 0;
        virtual void Unbind() const = 0;

        virtual void Update(unsigned int offset, unsigned int size, void* data) = 0;

        static Shared<UniformBuffer> Create(void* data, unsigned int size);
    };

    class SSBOBuffer
    {
        public:
        virtual void Bind(unsigned int index) const = 0;
        virtual void Unbind() const = 0;

        static Shared<SSBOBuffer> Create(void* data, unsigned int size);
    };

    enum class FramebufferTextureFormat
    {
        None = 0,

        RGBA8,
        RED_INTEGER,

        DEPTH24STENCIL8,

        Depth = DEPTH24STENCIL8
    };

    struct FramebufferTextureSpecification
    {
        FramebufferTextureSpecification() = default;
        FramebufferTextureSpecification(FramebufferTextureFormat format) : TextureFormat(format) {};

        FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
    };

    struct FramebufferAttachmentSpecification
    {
        FramebufferAttachmentSpecification() = default;
        FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> attachments) : Attachments(attachments) {};
        FramebufferAttachmentSpecification(std::vector<FramebufferTextureSpecification> attachments) : Attachments(attachments) {};

        std::vector<FramebufferTextureSpecification> Attachments;
    };

    struct FramebufferSpecification
    {
        unsigned int width = 0, height = 0;
        FramebufferAttachmentSpecification attachments;
        unsigned int samples = 1;
        bool swapChainTarget = false;
    };

    class Framebuffer
    {
        public:
        virtual ~Framebuffer() = default;

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        virtual void Resize(unsigned int width, unsigned int height) = 0;
        virtual int ReadPixel(unsigned int attachmentIdx, int x, int y) = 0;

        virtual void ClearAttachment(unsigned int attachmentIdx, int value) = 0;

        virtual const FramebufferSpecification &GetSpecification() const = 0;
        virtual const unsigned int GetColorAttachmentRendererID(unsigned int idx) const = 0;
        virtual const unsigned int GetDepthAttachmentRendererID() const = 0;
        static Shared<Framebuffer> Create(FramebufferSpecification &specification);
    };

};
