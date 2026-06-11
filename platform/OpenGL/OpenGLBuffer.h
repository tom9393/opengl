#pragma once

#include "Render/Buffer.h"

namespace Engine
{

    static const unsigned int s_MaxFramebufferSize = 8192;

    class OpenGLVertexBuffer : public VertexBuffer
    {
        public:
        OpenGLVertexBuffer(unsigned int size);
        OpenGLVertexBuffer(float* vertices, unsigned int size);
        ~OpenGLVertexBuffer();

        void Bind() const override;
        void Unbind() const override;

        BufferLayout& GetLayout() override { return m_Layout; }
        const BufferLayout& GetLayout() const override { return m_Layout; }
        void SetLayout(const BufferLayout &layout) override { m_Layout = layout; }
        void EnableAttribute(uint32_t index) override;
        void DisableAttribute(uint32_t index) override;

        void SetData(const void* data, const uint32_t size) override;

        private:
        unsigned int m_RendererID;
        BufferLayout m_Layout;
    };

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

    class OpenGLVertexArray : public VertexArray
    {
        public:
        OpenGLVertexArray();
        ~OpenGLVertexArray();

        void Bind() const override;
        void Unbind() const override;

        void AddVertexBuffer(const Shared<VertexBuffer> &vertexBuffer) override;
        void SetIndexBuffer(const Shared<IndexBuffer> &indexBuffer) override;

        const std::vector<Shared<VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
        const Shared<IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; }

        private:
        std::vector<Shared<VertexBuffer>> m_VertexBuffers;
        Shared<IndexBuffer> m_IndexBuffer;
        unsigned int m_RendererID;
        unsigned int m_VertexBufferIndex = 0; // For now use this
    };

    class OpenGLUniformBuffer : public UniformBuffer
    {
        public:
        OpenGLUniformBuffer(void* data, unsigned int size);
        ~OpenGLUniformBuffer();

        void Bind(unsigned int index) const override;
        void Unbind() const override;

        void Update(unsigned int offset, unsigned int size, void* data) override;

        private:
        unsigned int m_RendererID;
    };

    class OpenGLShaderStorageBuffer : public ShaderStorageBuffer
    {
        public:
        OpenGLShaderStorageBuffer(unsigned int size);
        OpenGLShaderStorageBuffer(void* data, unsigned int size);
        ~OpenGLShaderStorageBuffer();

        void Bind(unsigned int index) const override;
        void Unbind() const override;

        void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;

        private:
        unsigned int m_RendererID;
    };

    class OpenGLFramebuffer : public Framebuffer
    {
        public:

        OpenGLFramebuffer(FramebufferSpecification &specification);
        ~OpenGLFramebuffer();

        void Bind() const override;
        void Unbind() const override;

        void Resize(unsigned int width, unsigned int height) override;
        int ReadPixel(unsigned int attachmentIdx, int x, int y) override;

        void ClearAttachment(unsigned int attachmentIdx, int value) override;
        void AttachLayer(unsigned int layer) override;
        void AttachToTextureSlot(unsigned int idx, unsigned int rendererID) override;
        void AttachToDepthSlot(unsigned int idx) override;
        void AttachExternalDepth(unsigned int textureRendererID, bool hasStencil) override;

        const FramebufferSpecification &GetSpecification() const override { return m_Specification; }
        const unsigned int GetColorAttachmentRendererID(unsigned int idx = 0) const override { return m_ColorAttachments[idx]; }
        const unsigned int GetDepthAttachmentRendererID() const override { return m_DepthAttachment; }
        void Invalidate();

        private:
        unsigned int m_RendererID = 0;
        FramebufferSpecification m_Specification;

        std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecs;
        FramebufferTextureSpecification m_DepthSpec = FramebufferTextureFormat::None;

        std::vector<unsigned int> m_ColorAttachments;
        unsigned int m_DepthAttachment = 0;
    };

};
