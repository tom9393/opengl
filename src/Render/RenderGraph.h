#pragma once

#include "pch.h"
#include "Core/TimeStep.h"
#include "FramebufferPool.h"
#include "Passes/RenderPass.h"
#include "RenderContext.h"
#include "RenderView.h"

namespace Engine
{

    class RenderPass;

    // Simple RenderGraph: linear list of passes that execute in order.
    // Each pass has attachments; after Execute() you can query the framebuffer handle used by a pass
    // (useful to fetch the resolved color texture ID for ImGui::Image or ReadPixel).
    class RenderGraph
    {
    public:
        RenderGraph(unsigned int width, unsigned int height);
        ~RenderGraph();

        // Add a pass (returns reference to configure)
        template <typename Pass>
        RenderPass& AddPass(const std::string& name)
        {
            std::unique_ptr<RenderPass> pass = std::make_unique<Pass>(name);
            pass->Setup(*this);
            m_Passes.push_back(std::move(pass));
            return *m_Passes.back();
        }

        std::unordered_map<std::string, Shared<FramebufferHandle>> &GetFramebuffers()
        {
            return m_PassFramebuffers;
        }

        // Execute all passes in order for the given timestep
        void Execute(RenderContext &context, const RenderView &view);

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
