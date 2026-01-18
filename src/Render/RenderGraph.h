#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include "RenderPass.h"
#include "FramebufferPool.h"
#include "../TimeStep.h"

namespace Engine
{

    // Simple RenderGraph: linear list of passes that execute in order.
    // Each pass has attachments; after Execute() you can query the framebuffer handle used by a pass
    // (useful to fetch the resolved color texture ID for ImGui::Image or ReadPixel).
    class RenderGraph
    {
    public:
        RenderGraph(unsigned int width, unsigned int height);
        ~RenderGraph();

        // Add a pass (returns reference to configure)
        RenderPass& AddPass(const std::string& name);

        // Execute all passes in order for the given timestep
        void Execute(TimeStep ts);

        // Resize invalidates pool resources where necessary
        void Resize(unsigned int width, unsigned int height);

        // After Execute, get the framebuffer handle used by a pass name. Returns nullptr if not found.
        Shared<FramebufferHandle> GetPassFramebuffer(const std::string& passName) const;

        // Convenience: get color attachment renderer ID for pass output
        unsigned int GetPassColorAttachmentRendererID(const std::string& passName, unsigned int idx = 0) const;

    private:
        unsigned int m_Width, m_Height;
        std::vector<std::unique_ptr<RenderPass>> m_Passes;
        FramebufferPool m_Pool;

        // Map of last used framebuffer handle per pass name (populated during Execute)
        std::unordered_map<std::string, Shared<FramebufferHandle>> m_PassFramebuffers;
    };

};
