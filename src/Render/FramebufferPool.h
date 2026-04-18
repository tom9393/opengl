#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include "Buffer.h"

namespace Engine
{

    // FramebufferHandle wraps a Shared<Framebuffer> and exposes helper methods the rest of the engine expects:
    // - GetColorAttachmentRendererID(idx)
    // - ReadPixel(attachmentIdx, x, y)
    // - ClearAttachment(attachmentIdx, value)
    class FramebufferHandle
    {
    public:
        explicit FramebufferHandle(const Shared<Framebuffer>& fb) : m_Framebuffer(fb) {}

        // Expose underlying framebuffer for advanced operations
        Shared<Framebuffer> GetFramebuffer() const { return m_Framebuffer; }

        unsigned int GetColorAttachmentRendererID(unsigned int idx = 0) const
        {
            if (!m_Framebuffer) return 0;
            return m_Framebuffer->GetColorAttachmentRendererID(idx);
        }

        unsigned int GetDepthAttachmentRendererID() const
        {
            if (!m_Framebuffer) return 0;
            return m_Framebuffer->GetDepthAttachmentRendererID();
        }

        int ReadPixel(unsigned int attachmentIdx, int x, int y) const
        {
            if (!m_Framebuffer) return -1;
            return m_Framebuffer->ReadPixel(attachmentIdx, x, y);
        }

        void ClearAttachment(unsigned int attachmentIdx, int value) const
        {
            if (!m_Framebuffer) return;
            m_Framebuffer->ClearAttachment(attachmentIdx, value);
        }

    private:
        Shared<Framebuffer> m_Framebuffer;
    };

    // A small pool that caches created Framebuffers keyed by size + attachment layout.
    // It wraps your existing Framebuffer::Create(FramebufferSpecification).
    class FramebufferPool
    {
    public:
        FramebufferPool() = default;
        ~FramebufferPool() = default;

        // Acquire or create a framebuffer configured with the given attachments and size.
        // Returns a FramebufferHandle (wrapped in Shared) that provides the convenience methods.
        Shared<FramebufferHandle> AcquireFramebuffer(const std::string& passName, const std::vector<FramebufferTextureFormat>& attachments, BufferTextureType type, unsigned int width, unsigned int height);

        // Clears the pool (free all resources)
        void Clear();

    private:
        std::string BuildKey(const std::string& passName, const std::vector<FramebufferTextureFormat>& attachments, BufferTextureType type, unsigned int width, unsigned int height) const;

    private:
        std::unordered_map<std::string, Shared<FramebufferHandle>> m_Pool;
    };

};
