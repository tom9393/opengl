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

    GLenum BufferTextureTypeToGL(BufferTextureType type);

    class OpenGLRenderTarget : public RenderTarget
    {
        public:
        OpenGLRenderTarget(const RenderTargetSpecification& specification);
        ~OpenGLRenderTarget();

        unsigned int GetID() const override { return m_RendererID; }
        void Resize(unsigned int width, unsigned int height) override;
        const RenderTargetSpecification& GetSpecification() const override { return m_Specification; }

        private:
        void AllocateStorage();
        void SetSamplerParameters();

        RenderTargetSpecification m_Specification;
        unsigned int m_RendererID = 0;
    };

    class OpenGLFramebuffer : public Framebuffer
    {
        public:
        OpenGLFramebuffer(const FramebufferSpecification& specification);
        ~OpenGLFramebuffer();

        void Bind() const override;
        void Unbind() const override;

        int ReadPixel(unsigned int attachmentIdx, int x, int y) override;
        void ClearAttachment(unsigned int attachmentIdx, int value) override;
        void ClearColorAttachment(unsigned int attachmentIdx, const glm::vec4& value) override;
        void ClearDepthAttachment() override;

        private:
        unsigned int m_RendererID = 0;

        // Attachment textures are owned by their RenderTargets; the framebuffer
        // only references them.
        std::vector<Shared<RenderTarget>> m_ColorAttachments;
        Shared<RenderTarget> m_DepthAttachment;
    };

};
