#pragma once

#include "../../include/Buffer.h"

static const unsigned int s_MaxFramebufferSize = 8192;

class OpenGLVertexBuffer : public VertexBuffer
{
    public:
    OpenGLVertexBuffer(unsigned int size);
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

    void AddVertexBuffer(const std::shared_ptr<VertexBuffer> &vertexBuffer) override;
    void SetIndexBuffer(const std::shared_ptr<IndexBuffer> &indexBuffer) override;

    const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
    const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; }

    private:
    std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
    std::shared_ptr<IndexBuffer> m_IndexBuffer;
    unsigned int m_RendererID;
};

class OpenGLSSBOBuffer : public SSBOBuffer
{
    public:
    OpenGLSSBOBuffer(void* data, unsigned int size);
    ~OpenGLSSBOBuffer();

    void Bind(unsigned int index) const override;
    void Unbind() const override;

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

    const FramebufferSpecification &GetSpecification() const override { return m_Specification; }
    const unsigned int GetColorAttachmentRendererID(unsigned int idx = 0) const override { return m_ColorAttachments[idx]; }
    void Invalidate();

    private:
    unsigned int m_RendererID = 0;
    FramebufferSpecification m_Specification;

    std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecs;
    FramebufferTextureSpecification m_DepthSpec = FramebufferTextureFormat::None;

    std::vector<unsigned int> m_ColorAttachments;
    unsigned int m_DepthAttachment;
};
