#include "RenderGraph.h"
#include "RenderCommand.h"
#include "Renderer.h"
#include "Passes/RenderPass.h"

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
	                continue;
	            }
	            ResourceInfo info;
	            info.producer = pass;
	            info.format = out.format;
	            info.isDepth = out.isDepth;
	            info.colorAttachmentIndex = out.colorAttachmentIndex;
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

	    m_Dirty = false;
	}

	void RenderGraph::Execute(RenderContext& context, const RenderView& renderView)
	{
	    if (m_Dirty) Compile();

	    m_PassFramebuffers.clear();
	    m_AvailableResources.clear();

	    for (RenderPass* pass : m_ExecutionOrder)
	    {
	        if (!pass->EvaluateCondition(renderView)) continue;

	        auto attachments = pass->BuildAttachmentList();
	        BufferTextureType type = pass->GetTargetType();
	        if (type == BufferTextureType::None) type = BufferTextureType::Texture2D;

	        unsigned int passWidth = pass->GetWidth() > 0 ? pass->GetWidth() : m_Width;
	        unsigned int passHeight = pass->GetHeight() > 0 ? pass->GetHeight() : m_Height;

	        Shared<FramebufferHandle> fbHandle = m_Pool.AcquireFramebuffer(
	            pass->GetName(), attachments, type, passWidth, passHeight);
	        if (!fbHandle) continue;

	        m_PassFramebuffers[pass->GetName()] = fbHandle;
	        for (auto& out : pass->GetOutputs())
	            m_AvailableResources.insert(out.name);

	        fbHandle->GetFramebuffer()->Bind();
	        if (pass->HasClearColor())
	        {
	            RenderCommand::SetClearColor(pass->GetClearColor());
	            RenderCommand::Clear();
	        }
	        else
	        {
	            RenderCommand::Clear();
	        }
	        for (auto& kv : pass->GetClearInts())
	            fbHandle->ClearAttachment(kv.first, kv.second);

	        pass->Execute(context, renderView);

	        fbHandle->GetFramebuffer()->Unbind();
	    }
	}

	void RenderGraph::Resize(unsigned int width, unsigned int height)
	{
	    if (m_Width == width && m_Height == height) return;
	    m_Width = width;
	    m_Height = height;
	    m_Pool.Clear();
	}

	Shared<FramebufferHandle> RenderGraph::GetPassFramebuffer(const std::string& passName) const
	{
	    auto it = m_PassFramebuffers.find(passName);
	    if (it == m_PassFramebuffers.end()) return nullptr;
	    return it->second;
	}

	unsigned int RenderGraph::GetPassColorAttachmentRendererID(const std::string& passName, unsigned int idx) const
	{
	    auto fb = GetPassFramebuffer(passName);
	    if (!fb) return 0;
	    return fb->GetColorAttachmentRendererID(idx);
	}

	Shared<FramebufferHandle> RenderGraph::GetResource(const std::string& resourceName) const
	{
	    auto rit = m_Resources.find(resourceName);
	    if (rit == m_Resources.end()) return nullptr;
	    if (!m_AvailableResources.count(resourceName)) return nullptr;
	    auto it = m_PassFramebuffers.find(rit->second.producer->GetName());
	    if (it == m_PassFramebuffers.end()) return nullptr;
	    return it->second;
	}

	bool RenderGraph::HasInput(const std::string& resourceName) const
	{
	    return m_AvailableResources.count(resourceName) > 0;
	}

	void RenderGraph::BindInput(const std::string& resourceName, unsigned int slot) const
	{
	    auto rit = m_Resources.find(resourceName);
	    if (rit == m_Resources.end())
	    {
	        std::cerr << "RenderGraph::BindInput: unknown resource '" << resourceName << "'." << std::endl;
	        return;
	    }
	    auto fb = GetResource(resourceName);
	    if (!fb) return;
	    if (rit->second.isDepth)
	        fb->AttachToTextureSlotDepth(slot);
	    else
	        fb->AttachToTextureSlot(slot, rit->second.colorAttachmentIndex);
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
