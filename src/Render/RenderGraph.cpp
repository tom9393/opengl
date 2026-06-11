#include "RenderGraph.h"
#include "RenderCommand.h"
#include "Passes/RenderPass.h"

#include <cassert>
#include <iostream>
#include <queue>
#include <sstream>

namespace Engine
{

	RenderGraph::RenderGraph(unsigned int width, unsigned int height)
	    : m_Width(width), m_Height(height)
	{
	}

	RenderGraph::~RenderGraph() = default;

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
	                std::cerr << "RenderGraph: resource '" << out.name
	                          << "' declared as output by both '" << existing->second.producer->GetName()
	                          << "' and '" << pass->GetName() << "'. Keeping the first." << std::endl;
	                assert(false && "RenderGraph: duplicate output declaration");
	                continue;
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
	                    std::cerr << "RenderGraph: pass '" << pass->GetName()
	                              << "' aliases unknown resource '" << out.aliasOf << "'." << std::endl;
	                    assert(false && "RenderGraph: alias of unknown resource");
	                    continue;
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
	                std::cerr << "RenderGraph: pass '" << passPtr->GetName()
	                          << "' declares input '" << inputName
	                          << "' but no pass produces it." << std::endl;
	                assert(false && "RenderGraph: input without producer");
	            }
	        }
	    }

	    // Build adjacency (producer -> consumers) and in-degree counts.
	    std::unordered_map<RenderPass*, std::vector<RenderPass*>> adj;
	    std::unordered_map<RenderPass*, int> inDegree;
	    for (auto& p : m_Passes)
	    {
	        inDegree[p.get()] = 0;
	        adj[p.get()];
	    }
	    for (auto& passPtr : m_Passes)
	    {
	        RenderPass* consumer = passPtr.get();
	        std::unordered_set<RenderPass*> seenProducers;
	        for (auto& inputName : consumer->GetInputs())
	        {
	            auto it = m_Resources.find(inputName);
	            if (it == m_Resources.end()) continue;
	            RenderPass* producer = it->second.producer;
	            if (producer == consumer) continue;
	            if (!seenProducers.insert(producer).second) continue;
	            adj[producer].push_back(consumer);
	            inDegree[consumer]++;
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
	        std::cerr << "RenderGraph: cycle detected during topological sort. "
	                     "Falling back to insertion order." << std::endl;
	        assert(false && "RenderGraph: dependency cycle");
	        sorted.clear();
	        for (auto& p : m_Passes) sorted.push_back(p.get());
	    }

	    // Cull: reverse-walk from presented resources.
	    std::unordered_set<RenderPass*> required;
	    std::queue<RenderPass*> rq;
	    for (auto& name : m_Presented)
	    {
	        auto it = m_Resources.find(name);
	        if (it == m_Resources.end())
	        {
	            std::cerr << "RenderGraph: presented resource '" << name
	                      << "' has no producer." << std::endl;
	            assert(false && "RenderGraph: presented resource without producer");
	            continue;
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
	        spec.width = producer->GetWidth() > 0 ? producer->GetWidth() : m_Width;
	        spec.height = producer->GetHeight() > 0 ? producer->GetHeight() : m_Height;
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
	            // Skips outputs lost to a duplicate declaration (reported in BuildResourceMap).
	            if (it == m_Resources.end() || it->second.producer != pass || !it->second.target) continue;
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

	void RenderGraph::Execute(RenderContext& context, const RenderView& renderView)
	{
	    if (m_Dirty) Compile();

	    auto camAlloc = context.InstancePool->Allocate(&renderView.Camera, sizeof(CameraData));
	    if (camAlloc.Size > 0) context.InstancePool->BindAsUBO(0, camAlloc.Offset, camAlloc.Size);

	    auto lightAlloc = context.InstancePool->Allocate(&renderView.LightData, sizeof(SceneLightData));
	    if (lightAlloc.Size > 0) context.InstancePool->BindAsUBO(1, lightAlloc.Offset, lightAlloc.Size);

	    for (RenderPass* pass : m_ExecutionOrder)
	    {
	        if (!pass->EvaluateCondition(renderView)) continue;

	        const Shared<Framebuffer>& fb = pass->GetFramebuffer();
	        if (!fb) continue;

	        fb->Bind();

	        // A declared depth input is attached read-only: depth writes stay masked
	        // for the whole pass, which both protects the producer's depth and keeps
	        // sampling it from the pass legal.
	        bool externalDepth = pass->HasDepthInput();
	        if (externalDepth) RenderCommand::SetDepthWrite(false);

	        // Alias-only passes load the existing contents; owning passes clear, but
	        // never an externally attached depth.
	        if (pass->OwnsAnOutput())
	        {
	            if (pass->HasClearColor()) RenderCommand::SetClearColor(pass->GetClearColor());
	            if (externalDepth) RenderCommand::ClearColorOnly();
	            else               RenderCommand::Clear();
	            for (auto& kv : pass->GetClearInts())
	                fb->ClearAttachment(kv.first, kv.second);
	        }

	        RenderCommand::PushDebugGroup(pass->GetName().c_str());
	        pass->Execute(context, renderView);
	        RenderCommand::PopDebugGroup();

	        if (externalDepth) RenderCommand::SetDepthWrite(true);

	        fb->Unbind();
	    }
	}

	void RenderGraph::Resize(unsigned int width, unsigned int height)
	{
	    if (m_Width == width && m_Height == height) return;
	    m_Width = width;
	    m_Height = height;

	    // Texture storage is respecified in place (ids stay stable), so the composed
	    // framebuffers don't need rebuilding. Fixed-size targets (shadow maps) are
	    // untouched; aliases follow their root.
	    for (auto& kv : m_Resources)
	    {
	        ResourceInfo& info = kv.second;
	        if (!info.aliasOf.empty() || !info.target) continue;
	        if (info.producer->GetWidth() > 0) continue;
	        info.target->Resize(width, height);
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
