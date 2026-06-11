#include "RenderGraph.h"
#include "RenderCommand.h"
#include "Passes/RenderPass.h"

#include <cstdlib>
#include <iostream>
#include <queue>
#include <set>
#include <sstream>

namespace Engine
{

	// A misdeclared graph is a programming error; abort loudly at compile time
	// (typically startup) in every build configuration instead of limping on
	// with a broken frame.
	[[noreturn]] static void GraphError(const std::string& message)
	{
	    std::cerr << "RenderGraph: " << message << std::endl;
	    std::abort();
	}

	RenderGraph::RenderGraph(unsigned int width, unsigned int height)
	    : m_Width(width), m_Height(height)
	{
	}

	RenderGraph::~RenderGraph()
	{
	    for (auto& kv : m_PassTimers)
	        for (unsigned int id : kv.second.ids)
	            if (id) RenderCommand::DeleteTimerQuery(id);
	}

	void RenderGraph::MarkPresent(const std::string& resourceName)
	{
	    m_Presented.insert(resourceName);
	    m_Dirty = true;
	}

	void RenderGraph::BuildResourceMap()
	{
	    m_Resources.clear();
	    for (auto& passPtr : m_Passes)
	    {
	        RenderPass* pass = passPtr.get();
	        for (auto& out : pass->GetOutputs())
	        {
	            auto existing = m_Resources.find(out.name);
	            if (existing != m_Resources.end())
	            {
	                GraphError("resource '" + out.name + "' declared as output by both '"
	                           + existing->second.producer->GetName() + "' and '" + pass->GetName() + "'.");
	            }
	            ResourceInfo info;
	            info.producer = pass;
	            if (!out.aliasOf.empty())
	            {
	                // Alias outputs share the source resource's texture. Passes are
	                // registered in insertion order, so the source must come from an
	                // earlier-added pass. Chains resolve to the owning root.
	                auto src = m_Resources.find(out.aliasOf);
	                if (src == m_Resources.end())
	                {
	                    GraphError("pass '" + pass->GetName() + "' aliases unknown resource '"
	                               + out.aliasOf + "'.");
	                }
	                info.aliasOf = src->second.aliasOf.empty() ? out.aliasOf : src->second.aliasOf;
	                info.format = src->second.format;
	                info.isDepth = src->second.isDepth;
	            }
	            else
	            {
	                info.format = out.format;
	                info.isDepth = out.isDepth;
	            }
	            m_Resources[out.name] = info;
	        }
	    }
	}

	void RenderGraph::Compile()
	{
	    if (!m_Dirty) return;

	    BuildResourceMap();

	    // Validate inputs have producers.
	    for (auto& passPtr : m_Passes)
	    {
	        for (auto& inputName : passPtr->GetInputs())
	        {
	            if (m_Resources.find(inputName) == m_Resources.end())
	            {
	                GraphError("pass '" + passPtr->GetName() + "' declares input '"
	                           + inputName + "' but no pass produces it.");
	            }
	        }
	    }

	    // Build adjacency (producer -> consumers) and in-degree counts.
	    std::unordered_map<RenderPass*, std::vector<RenderPass*>> adj;
	    std::unordered_map<RenderPass*, int> inDegree;
	    std::set<std::pair<RenderPass*, RenderPass*>> edges;
	    for (auto& p : m_Passes)
	    {
	        inDegree[p.get()] = 0;
	        adj[p.get()];
	    }
	    auto addEdge = [&](RenderPass* from, RenderPass* to)
	    {
	        if (from == to) return;
	        if (!edges.insert({ from, to }).second) return;
	        adj[from].push_back(to);
	        inDegree[to]++;
	    };

	    // Read-after-write: a consumer runs after the producer of each input.
	    for (auto& passPtr : m_Passes)
	    {
	        RenderPass* consumer = passPtr.get();
	        for (auto& inputName : consumer->GetInputs())
	        {
	            auto it = m_Resources.find(inputName);
	            if (it == m_Resources.end()) continue;
	            addEdge(it->second.producer, consumer);
	        }
	    }

	    // Write-after-read: a pass that renders in place into another resource
	    // must wait for every other reader of that resource, or those readers
	    // would sample the overwritten contents.
	    for (auto& passPtr : m_Passes)
	    {
	        RenderPass* writer = passPtr.get();
	        for (auto& out : writer->GetOutputs())
	        {
	            if (out.aliasOf.empty()) continue;
	            for (auto& readerPtr : m_Passes)
	            {
	                RenderPass* reader = readerPtr.get();
	                if (reader == writer) continue;
	                for (auto& inputName : reader->GetInputs())
	                {
	                    if (inputName != out.aliasOf) continue;
	                    addEdge(reader, writer);
	                    break;
	                }
	            }
	        }
	    }

	    // Kahn's topological sort, seeded in insertion order for stability.
	    std::vector<RenderPass*> sorted;
	    sorted.reserve(m_Passes.size());
	    std::queue<RenderPass*> ready;
	    for (auto& p : m_Passes)
	    {
	        if (inDegree[p.get()] == 0) ready.push(p.get());
	    }
	    while (!ready.empty())
	    {
	        RenderPass* p = ready.front(); ready.pop();
	        sorted.push_back(p);
	        for (RenderPass* next : adj[p])
	        {
	            if (--inDegree[next] == 0) ready.push(next);
	        }
	    }

	    if (sorted.size() != m_Passes.size())
	    {
	        GraphError("cycle detected during topological sort.");
	    }

	    // Cull: reverse-walk from presented resources.
	    std::unordered_set<RenderPass*> required;
	    std::queue<RenderPass*> rq;
	    for (auto& name : m_Presented)
	    {
	        auto it = m_Resources.find(name);
	        if (it == m_Resources.end())
	        {
	            GraphError("presented resource '" + name + "' has no producer.");
	        }
	        if (required.insert(it->second.producer).second) rq.push(it->second.producer);
	    }
	    while (!rq.empty())
	    {
	        RenderPass* p = rq.front(); rq.pop();
	        for (auto& inputName : p->GetInputs())
	        {
	            auto it = m_Resources.find(inputName);
	            if (it == m_Resources.end()) continue;
	            if (required.insert(it->second.producer).second) rq.push(it->second.producer);
	        }
	    }

	    m_ExecutionOrder.clear();
	    if (m_Presented.empty())
	    {
	        // No present markers: run every pass (back-compat).
	        m_ExecutionOrder = sorted;
	    }
	    else
	    {
	        for (RenderPass* p : sorted)
	        {
	            if (required.count(p)) m_ExecutionOrder.push_back(p);
	        }
	    }

	    AllocateResources();
	    ComposeFramebuffers();

	    m_Dirty = false;
	}

	void RenderGraph::AllocateResources()
	{
	    std::unordered_set<RenderPass*> live(m_ExecutionOrder.begin(), m_ExecutionOrder.end());

	    for (auto& kv : m_Resources)
	    {
	        ResourceInfo& info = kv.second;
	        if (!info.aliasOf.empty()) continue;
	        if (!live.count(info.producer)) continue;

	        RenderPass* producer = info.producer;
	        RenderTargetSpecification spec;
	        spec.format = info.format;
	        spec.type = producer->GetTargetType() == BufferTextureType::None
	                        ? BufferTextureType::Texture2D : producer->GetTargetType();
	        float scale = producer->GetSizeScale();
	        spec.width = producer->GetWidth() > 0 ? producer->GetWidth()
	                        : std::max(1u, (unsigned int)((float)m_Width * scale));
	        spec.height = producer->GetHeight() > 0 ? producer->GetHeight()
	                        : std::max(1u, (unsigned int)((float)m_Height * scale));
	        spec.layers = producer->GetLayers();

	        info.target = RenderTarget::Create(spec);
	        if (!info.target)
	            std::cerr << "RenderGraph: failed to create texture for resource '"
	                      << kv.first << "'." << std::endl;
	    }

	    // Alias resources share their root's texture.
	    for (auto& kv : m_Resources)
	    {
	        ResourceInfo& info = kv.second;
	        if (info.aliasOf.empty()) continue;
	        auto root = m_Resources.find(info.aliasOf);
	        if (root != m_Resources.end()) info.target = root->second.target;
	    }
	}

	void RenderGraph::ComposeFramebuffers()
	{
	    for (RenderPass* pass : m_ExecutionOrder)
	    {
	        FramebufferSpecification spec;
	        for (auto& out : pass->GetOutputs())
	        {
	            auto it = m_Resources.find(out.name);
	            if (it == m_Resources.end() || !it->second.target) continue;
	            spec.attachments.push_back(it->second.target);
	        }

	        // A declared depth input becomes the pass's depth attachment; Execute masks
	        // depth writes while the pass runs, so it stays read-only.
	        if (pass->HasDepthInput())
	        {
	            auto it = m_Resources.find(pass->GetDepthInput());
	            if (it != m_Resources.end() && it->second.target)
	                spec.attachments.push_back(it->second.target);
	        }

	        pass->m_Framebuffer = Framebuffer::Create(spec);
	        if (!pass->m_Framebuffer)
	            std::cerr << "RenderGraph: failed to create framebuffer for pass '"
	                      << pass->GetName() << "'." << std::endl;

	        // Resolve declared inputs to their textures.
	        pass->m_CompiledInputs.assign(pass->GetInputs().size(), nullptr);
	        for (size_t i = 0; i < pass->GetInputs().size(); i++)
	        {
	            auto it = m_Resources.find(pass->GetInputs()[i]);
	            if (it != m_Resources.end()) pass->m_CompiledInputs[i] = it->second.target;
	        }
	    }
	}

	static void ApplyPipelineState(const PipelineState& state)
	{
	    RenderCommand::SetDepthTest(state.depthTest);
	    RenderCommand::SetDepthWrite(state.depthWrite);
	    switch (state.blend)
	    {
	        case PassBlend::Off:      RenderCommand::SetAdditiveBlend(false); break; // disables blending
	        case PassBlend::Alpha:    RenderCommand::SetAlphaBlend(true);     break;
	        case PassBlend::Additive: RenderCommand::SetAdditiveBlend(true);  break;
	    }
	}

	void RenderGraph::Execute(RenderContext& context, const RenderView& renderView)
	{
	    if (m_Dirty) Compile();

	    auto camAlloc = context.InstancePool->Allocate(&renderView.Camera, sizeof(CameraData));
	    if (camAlloc.Size > 0) context.InstancePool->BindAsUBO(0, camAlloc.Offset, camAlloc.Size);

	    auto lightAlloc = context.InstancePool->Allocate(&renderView.LightData, sizeof(SceneLightData));
	    if (lightAlloc.Size > 0) context.InstancePool->BindAsUBO(1, lightAlloc.Offset, lightAlloc.Size);

	    m_PassTimings.clear();

	    for (RenderPass* pass : m_ExecutionOrder)
	    {
	        if (!pass->EvaluateCondition(renderView))
	        {
	            m_PassTimings.push_back({ pass->GetName(), 0.0f });
	            continue;
	        }

	        const Shared<Framebuffer>& fb = pass->GetFramebuffer();
	        if (!fb) continue;

	        // Harvest the elapsed time recorded in this slot a few frames ago,
	        // then reuse its query for this frame.
	        PassTimerQueries& timer = m_PassTimers[pass];
	        unsigned int& query = timer.ids[timer.next];
	        if (query == 0) query = RenderCommand::CreateTimerQuery();
	        if (timer.used[timer.next])
	        {
	            uint64_t ns = 0;
	            if (RenderCommand::GetTimerQueryResult(query, ns))
	                timer.lastMs = (float)((double)ns / 1.0e6);
	        }
	        RenderCommand::BeginTimerQuery(query);

	        fb->Bind();

	        // The declared state is the pass's baseline; a declared depth input is
	        // attached read-only, so depth writes are forced off, which both
	        // protects the producer's depth and keeps sampling it legal.
	        PipelineState state = pass->GetPipelineState();
	        if (pass->HasDepthInput()) state.depthWrite = false;
	        ApplyPipelineState(state);

	        // Owning passes clear their attachments individually; aliased outputs
	        // load the existing contents, and an externally attached depth is
	        // never cleared (it isn't an output).
	        if (pass->OwnsAnOutput() && !pass->IsClearDisabled())
	        {
	            unsigned int colorIdx = 0;
	            for (auto& out : pass->GetOutputs())
	            {
	                if (!out.aliasOf.empty()) { colorIdx++; continue; }
	                if (out.isDepth)
	                {
	                    fb->ClearDepthAttachment();
	                    continue;
	                }
	                auto intClear = pass->GetClearInts().find(colorIdx);
	                if (intClear != pass->GetClearInts().end())
	                    fb->ClearAttachment(colorIdx, intClear->second);
	                else
	                    fb->ClearColorAttachment(colorIdx, pass->GetClearValue(colorIdx));
	                colorIdx++;
	            }
	        }

	        RenderCommand::PushDebugGroup(pass->GetName().c_str());
	        pass->Execute(context, renderView);
	        RenderCommand::PopDebugGroup();

	        RenderCommand::EndTimerQuery();
	        timer.used[timer.next] = true;
	        timer.next = (timer.next + 1) % 3;
	        m_PassTimings.push_back({ pass->GetName(), timer.lastMs });

	        fb->Unbind();
	    }

	    // Leave the context in the default state for whatever renders next
	    // (UI, offscreen bakes).
	    ApplyPipelineState(PipelineState{});
	}

	void RenderGraph::Resize(unsigned int width, unsigned int height)
	{
	    if (m_Width == width && m_Height == height) return;
	    m_Width = width;
	    m_Height = height;

	    // Texture storage is respecified in place (ids stay stable), so the composed
	    // framebuffers don't need rebuilding. Fixed-size targets (shadow maps) are
	    // untouched; scaled targets follow at their scale; aliases follow their root.
	    for (auto& kv : m_Resources)
	    {
	        ResourceInfo& info = kv.second;
	        if (!info.aliasOf.empty() || !info.target) continue;
	        if (info.producer->GetWidth() > 0) continue;
	        float scale = info.producer->GetSizeScale();
	        info.target->Resize(std::max(1u, (unsigned int)((float)width * scale)),
	                            std::max(1u, (unsigned int)((float)height * scale)));
	    }
	}

	unsigned int RenderGraph::GetResourceTextureID(const std::string& resourceName) const
	{
	    auto it = m_Resources.find(resourceName);
	    if (it == m_Resources.end() || !it->second.target) return 0;
	    return it->second.target->GetID();
	}

	std::string RenderGraph::ExportDot() const
	{
	    std::unordered_set<const RenderPass*> inExec;
	    for (auto* p : m_ExecutionOrder) inExec.insert(p);

	    std::ostringstream ss;
	    ss << "digraph RenderGraph {\n";
	    ss << "  rankdir=LR;\n";
	    ss << "  node [shape=box, style=rounded];\n";

	    for (auto& passPtr : m_Passes)
	    {
	        const RenderPass* p = passPtr.get();
	        bool culled = inExec.find(p) == inExec.end();
	        ss << "  \"" << p->GetName() << "\" [label=\"" << p->GetName();
	        if (p->HasCondition()) ss << "\\n(conditional)";
	        ss << "\"";
	        if (culled) ss << ", style=\"dashed,rounded\", color=gray, fontcolor=gray";
	        ss << "];\n";
	    }

	    for (auto& kv : m_Resources)
	    {
	        const std::string& name = kv.first;
	        const ResourceInfo& info = kv.second;
	        ss << "  \"res_" << name << "\" [shape=ellipse, label=\"" << name << "\"";
	        if (m_Presented.count(name)) ss << ", style=filled, fillcolor=lightblue";
	        ss << "];\n";
	        ss << "  \"" << info.producer->GetName() << "\" -> \"res_" << name << "\";\n";
	    }

	    for (auto& passPtr : m_Passes)
	    {
	        for (auto& inputName : passPtr->GetInputs())
	            ss << "  \"res_" << inputName << "\" -> \"" << passPtr->GetName() << "\";\n";
	    }

	    ss << "}\n";
	    return ss.str();
	}

};
