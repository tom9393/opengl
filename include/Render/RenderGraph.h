#pragma once

#include <vector>
#include <memory>
#include <string>
#include "RenderPass.h"
#include "FramebufferPool.h"
#include "Renderer.h"
#include "../TimeStep.h"

namespace Engine
{

    // Simple RenderGraph: linear list of passes that execute in order.
    // For now it's intentionally minimal: each pass describes its attachments and a callback.
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

    private:
        unsigned int m_Width, m_Height;
        std::vector<std::unique_ptr<RenderPass>> m_Passes;
        FramebufferPool m_Pool;
    };

};
