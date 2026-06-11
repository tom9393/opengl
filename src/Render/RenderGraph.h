#pragma once

#include "pch.h"
#include "Core/TimeStep.h"
#include "FramebufferHandle.h"
#include "Passes/RenderPass.h"
#include "RenderContext.h"
#include "RenderView.h"

namespace Engine
{

    class RenderPass;

    // A true render graph: passes declare named input and output resources, and the graph
    // resolves their dependencies into a topologically sorted execution order. Passes that
    // do not transitively contribute to a presented resource are culled. Each pass can also
    // attach a runtime predicate that skips its execution for a given frame.
    class RenderGraph
    {
    public:
        RenderGraph(unsigned int width, unsigned int height);
        ~RenderGraph();

        // Add a pass. Setup() is invoked immediately so the pass can declare its
        // inputs/outputs/condition. The compiled state is invalidated. Extra
        // arguments are forwarded to the pass constructor after the name.
        template <typename Pass, typename... Args>
        RenderPass& AddPass(const std::string& name, Args&&... args)
        {
            std::unique_ptr<RenderPass> pass = std::make_unique<Pass>(name, std::forward<Args>(args)...);
            pass->Setup(*this);
            m_Passes.push_back(std::move(pass));
            m_Dirty = true;
            return *m_Passes.back();
        }

        // Mark a named output as a presented (external) resource. The producing pass and
        // every transitive dependency survive culling. When no resource is marked, the
        // graph keeps all passes (insertion-time topological order).
        void MarkPresent(const std::string& resourceName);

        // Resolve inputs to producers, validate, detect cycles, topologically sort,
        // cull passes not contributing to a presented resource, and allocate the
        // framebuffers of the surviving passes. Idempotent; runs on the next Execute
        // when dirty.
        void Compile();

        // Execute the compiled graph for the given view. Conditions are evaluated per pass.
        void Execute(RenderContext& context, const RenderView& view);

        void Resize(unsigned int width, unsigned int height);

        // ---- Pass / resource lookup ----

        std::unordered_map<std::string, Shared<FramebufferHandle>>& GetFramebuffers() { return m_PassFramebuffers; }

        Shared<FramebufferHandle> GetPassFramebuffer(const std::string& passName) const;
        unsigned int GetPassColorAttachmentRendererID(const std::string& passName, unsigned int idx = 0) const;

        // Resolve a resource name to the framebuffer of its producing pass. Returns null
        // when the resource isn't declared, hasn't run yet this frame, or its producer was
        // skipped by its condition.
        Shared<FramebufferHandle> GetResource(const std::string& resourceName) const;

        // True when a declared input resource is available this frame.
        bool HasInput(const std::string& resourceName) const;

        // Bind a declared input resource to a texture slot. Picks color or depth binding
        // from the producer's declaration. No-op when the resource isn't available.
        void BindInput(const std::string& resourceName, unsigned int slot) const;

        // ---- Debugging ----

        // Emit the compiled graph as Graphviz DOT. Culled passes are shown dashed in gray;
        // presented resources are highlighted.
        std::string ExportDot() const;

    private:
        struct ResourceInfo
        {
            RenderPass* producer = nullptr;
            // Pass whose framebuffer physically holds the resource (differs from the
            // producer for alias outputs, which render in place into another pass's target).
            RenderPass* physicalPass = nullptr;
            unsigned int colorAttachmentIndex = 0;
            FramebufferTextureFormat format = FramebufferTextureFormat::None;
            bool isDepth = false;
        };

        void BuildResourceMap();
        void AllocateFramebuffers();
        bool IsDepthFormat(FramebufferTextureFormat fmt) const
        {
            return fmt == FramebufferTextureFormat::DEPTH24STENCIL8 || fmt == FramebufferTextureFormat::Depth;
        }

        unsigned int m_Width, m_Height;
        std::vector<std::unique_ptr<RenderPass>> m_Passes;

        // Compiled state. Framebuffers are allocated at compile time and resized in
        // place; viewport-sized targets follow Resize, fixed-size ones are untouched.
        std::vector<RenderPass*> m_ExecutionOrder;
        std::unordered_map<std::string, ResourceInfo> m_Resources;
        std::unordered_set<std::string> m_Presented;
        std::unordered_map<std::string, Shared<FramebufferHandle>> m_PassFramebuffers;
        bool m_Dirty = true;

        // Per-frame state: resources whose producer has executed this frame.
        std::unordered_set<std::string> m_AvailableResources;
    };

};
