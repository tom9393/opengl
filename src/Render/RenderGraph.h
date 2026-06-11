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

    // A true render graph: passes declare named input and output resources, and the graph
    // resolves their dependencies into a topologically sorted execution order. Passes that
    // do not transitively contribute to a presented resource are culled. Each pass can also
    // attach a runtime predicate that skips its execution for a given frame.
    //
    // The graph owns the resources: each surviving resource gets a texture at compile
    // time, and each pass's framebuffer is composed from the textures of its outputs
    // (plus a read-only depth input, when declared). Texture ids stay stable across
    // resizes, since storage is respecified in place.
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
        // cull passes not contributing to a presented resource, then allocate the
        // surviving resources' textures and compose each pass's framebuffer from them.
        // Idempotent; runs on the next Execute when dirty.
        void Compile();

        // Execute the compiled graph for the given view. Conditions are evaluated per pass.
        void Execute(RenderContext& context, const RenderView& view);

        void Resize(unsigned int width, unsigned int height);

        // ---- Resource lookup ----

        struct ResourceInfo
        {
            RenderPass* producer = nullptr;
            FramebufferTextureFormat format = FramebufferTextureFormat::None;
            bool isDepth = false;
            // Empty for resources that own their texture; otherwise the name of the
            // (transitively resolved) resource whose texture this one renders into.
            std::string aliasOf;
            // Allocated at compile time for resources whose producer survives culling;
            // alias resources share their source's texture.
            Shared<RenderTarget> target;
        };

        const std::unordered_map<std::string, ResourceInfo>& GetResources() const { return m_Resources; }

        // Renderer id of the resource's backing texture, or 0 when the resource is
        // unknown or its producer was culled.
        unsigned int GetResourceTextureID(const std::string& resourceName) const;

        // ---- Profiling ----

        struct PassTiming
        {
            std::string Name;
            float GpuMs = 0.0f; // 0 for passes skipped by their condition
        };

        // GPU time per executed pass, refreshed every Execute. Results lag a few
        // frames behind (queries are polled without stalling).
        const std::vector<PassTiming>& GetPassTimings() const { return m_PassTimings; }

        // ---- Debugging ----

        // Emit the compiled graph as Graphviz DOT. Culled passes are shown dashed in gray;
        // presented resources are highlighted.
        std::string ExportDot() const;

    private:
        void BuildResourceMap();
        void AllocateResources();
        void ComposeFramebuffers();

        unsigned int m_Width, m_Height;
        std::vector<std::unique_ptr<RenderPass>> m_Passes;

        // Compiled state.
        std::vector<RenderPass*> m_ExecutionOrder;
        std::unordered_map<std::string, ResourceInfo> m_Resources;
        std::unordered_set<std::string> m_Presented;
        bool m_Dirty = true;

        // Per-pass GPU timer queries, ring-buffered so results from a few frames
        // back can be read without stalling on in-flight frames.
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
