#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include "../Buffer.h"

// A small pool that caches created Framebuffers keyed by size + attachment layout.
// It wraps your existing Framebuffer::Create(FramebufferSpecification).

namespace Engine
{

    class FramebufferPool
    {
    public:
        FramebufferPool() = default;
        ~FramebufferPool() = default;

        // Acquire or create a framebuffer configured with the given attachments and size.
        // The attachments vector is a list of FramebufferTextureFormat in attachment order.
        Shared<Framebuffer> AcquireFramebuffer(const std::vector<FramebufferTextureFormat>& attachments, unsigned int width, unsigned int height);

        // Clears the pool (free all resources)
        void Clear();

    private:
        std::string BuildKey(const std::vector<FramebufferTextureFormat>& attachments, unsigned int width, unsigned int height) const;

    private:
        std::unordered_map<std::string, Shared<Framebuffer>> m_Pool;
    };

}
