#pragma once

#include "pch.h"
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

        void AttachToTextureSlot(unsigned int slot, unsigned int attachmentIdx) const
        {
            if (!m_Framebuffer) return;
            m_Framebuffer->AttachToTextureSlot(slot, GetColorAttachmentRendererID(attachmentIdx));
        }

        void AttachToTextureSlotDepth(unsigned int slot)
        {
            if (!m_Framebuffer) return;
            m_Framebuffer->AttachToDepthSlot(slot);
        }

    private:
        Shared<Framebuffer> m_Framebuffer;
    };

};
