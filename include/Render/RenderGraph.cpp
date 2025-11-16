#include "RenderGraph.h"
#include "RenderCommand.h"
#include "Renderer.h"
#include "../Buffer.h"

namespace Engine
{

	RenderGraph::RenderGraph(unsigned int width, unsigned int height)
	    : m_Width(width), m_Height(height)
	{
	}

	RenderGraph::~RenderGraph() = default;

	RenderPass& RenderGraph::AddPass(const std::string& name)
	{
	    m_Passes.emplace_back(std::make_unique<RenderPass>(name));
	    return *m_Passes.back();
	}

	void RenderGraph::Execute(TimeStep ts)
	{
	    // Execute passes in-order.
	    for (auto& passPtr : m_Passes)
	    {
		RenderPass& pass = *passPtr;
		const auto& attachments = pass.GetAttachments();

		// Acquire framebuffer for this pass
		Shared<Framebuffer> fb = m_Pool.AcquireFramebuffer(attachments, m_Width, m_Height);
		if (!fb)
		    continue;

		fb->Bind();

		// If pass specified clear color, use it and clear. Otherwise keep current clear.
		if (pass.HasClearColor())
		{
		    RenderCommand::SetClearColor(pass.GetClearColor());
		    RenderCommand::Clear();
		}
		else
		{
		    // Even if no clear color specified, we'll still clear depth buffer by default to be safe
		    // If you want to avoid extra clears, make this configurable.
		    RenderCommand::Clear();
		}

		// Clear integer attachments (e.g., entity id) using the framebuffer helper
		for (auto& kv : pass.GetClearInts())
		{
		    unsigned int attachmentIndex = kv.first;
		    int value = kv.second;
		    fb->ClearAttachment(attachmentIndex, value);
		}

		// Call the pass callback (client code will issue Renderer::BeginScene / submit geometry)
		auto cb = pass.GetCallback();
		if (cb)
		    cb(ts);

		fb->Unbind();
	    }
	}

	void RenderGraph::Resize(unsigned int width, unsigned int height)
	{
	    if (m_Width == width && m_Height == height)
		return;
	    m_Width = width;
	    m_Height = height;
	    m_Pool.Clear(); // Drop cached framebuffers (they will be recreated with new size on next Acquire)
	}

};
