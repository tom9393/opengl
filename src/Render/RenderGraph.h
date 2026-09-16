#pragma once

#include "pch.h"
#include "Core/TimeStep.h"
#include "Buffer.h"
#include "Passes/RenderPass.h"
#include "RenderContext.h"
#include "RenderView.h"

namespace Engine
{

    class RenderPass;

    // Passes declare named input and output resources -> graph resolves their dependencies into a topologically sorted execution order
    // Unneeded passes are culled; passes can attach a runtime check that skip execution for given frame
    // Graph owns the resources; each surviving resource gets a texture at compile time and each pass's framebuffer is composed from the textures of its outputs
    class RenderGraph
    {
    public:
        RenderGraph(unsigned int width, unsigned int height);
        ~RenderGraph();

        // Pass's Setup() invoked immediately so that the pass can declare its inputs/outputs/condition. Compile state invalidated
        template <typename Pass, typename... Args>
        RenderPass& AddPass(const std::string& name, Args&&... args)
        {
            std::unique_ptr<RenderPass> pass = std::make_unique<Pass>(name, std::forward<Args>(args)...);
            pass->Setup(*this);
            m_Passes.push_back(std::move(pass));
            m_Dirty = true;
            return *m_Passes.back();
        }

        // Mark a named output as a required external resource; if no resource is marked the graph keeps all passes
        void MarkPresent(const std::string& resourceName);

        void Compile();

        void Execute(RenderContext& context, const RenderView& view);

        void Resize(unsigned int width, unsigned int height);

        struct ResourceInfo
        {
            RenderPass* producer = nullptr;
            FramebufferTextureFormat format = FramebufferTextureFormat::None;
            bool isDepth = false;
            // Empty for resources that own their texture; otherwise the name of the resource whose texture this one renders into
            std::string aliasOf;
            // Allocated at compile time for resources (handle id) whose producer survives culling; alias resources share their source's texture
            Shared<RenderTarget> target;
        };

        const std::unordered_map<std::string, ResourceInfo>& GetResources() const { return m_Resources; }

        // Renderer id of the resource's backing texture, or 0 when the resource is unknown or its producer was culled
        unsigned int GetResourceTextureID(const std::string& resourceName) const;

        struct PassTiming
        {
            std::string Name;
            float GpuMs = 0.0f; // 0 for passes skipped by their condition
        };

        // GPU time per executed pass, refreshed every Execute
        const std::vector<PassTiming>& GetPassTimings() const { return m_PassTimings; }

        // For debugging: emit the compiled graph as Graphviz DOT. Culled passes are shown dashed in gray; presented resources are highlighted
        std::string ExportDot() const;

    private:
        void BuildResourceMap();
        void AllocateResources();
        void ComposeFramebuffers();

        unsigned int m_Width, m_Height;
        std::vector<std::unique_ptr<RenderPass>> m_Passes;

        // Compiled state
        std::vector<RenderPass*> m_ExecutionOrder;
        std::unordered_map<std::string, ResourceInfo> m_Resources;
        std::unordered_set<std::string> m_Presented;
        bool m_Dirty = true;

        // Per-pass GPU timer queries, ring-buffered so results from a few frames back can be read without stalling on in-flight frames
        struct PassTimerQueries
        {
            unsigned int ids[3] = { 0, 0, 0 };
            bool used[3] = { false, false, false };
            unsigned int next = 0;
            float lastMs = 0.0f;
        };
        std::unordered_map<RenderPass*, PassTimerQueries> m_PassTimers;
        std::vector<PassTiming> m_PassTimings;
    };

};
