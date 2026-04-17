#include "RenderGraph.h"
#include "RenderCommand.h"
#include "Renderer.h"

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

	void RenderGraph::Execute(RenderContext &context)
	{
	    m_PassFramebuffers.clear();

	    for (auto& passPtr : m_Passes)
	    {
			RenderPass& pass = *passPtr;
			const auto& attachments = pass.GetAttachments();
			BufferTextureType type = pass.GetTargetType();

			unsigned int passWidth = pass.GetWidth() > 0 ? pass.GetWidth() : m_Width;
        	unsigned int passHeight = pass.GetHeight() > 0 ? pass.GetHeight() : m_Height;

			Shared<FramebufferHandle> fbHandle = m_Pool.AcquireFramebuffer(pass.GetName(), attachments, type, m_Width, m_Height);
			if (!fbHandle)
				continue;

			// Store handle so client code can query after Execute
			m_PassFramebuffers[pass.GetName()] = fbHandle;

			// Bind and clear
			{
				// Using underlying fb->Bind directly is fine; if you prefer RAII use ScopedFramebufferBind here.
				fbHandle->GetFramebuffer()->Bind();

				if (pass.HasClearColor())
				{
					RenderCommand::SetClearColor(pass.GetClearColor());
					RenderCommand::Clear();
				}
				else
				{
					// Default clear (we still clear depth/stencil to be safe)
					RenderCommand::Clear();
				}

				// Clear integer attachments specified by the pass
				for (auto& kv : pass.GetClearInts())
				{
					unsigned int attachmentIndex = kv.first;
					int value = kv.second;
					fbHandle->ClearAttachment(attachmentIndex, value);
				}
			}

			// Execute the pass callback
			auto cb = pass.GetCallback();
			if (cb)
				cb(context);

			fbHandle->GetFramebuffer()->Unbind();
	    }
	}

	void RenderGraph::Resize(unsigned int width, unsigned int height)
	{
	    if (m_Width == width && m_Height == height)
		return;
	    m_Width = width;
	    m_Height = height;
	    m_Pool.Clear();
	}

	Shared<FramebufferHandle> RenderGraph::GetPassFramebuffer(const std::string& passName) const
	{
	    auto it = m_PassFramebuffers.find(passName);
	    if (it == m_PassFramebuffers.end())
		return nullptr;
	    return it->second;
	}

	unsigned int RenderGraph::GetPassColorAttachmentRendererID(const std::string& passName, unsigned int idx) const
	{
	    auto fb = GetPassFramebuffer(passName);
	    if (!fb) return 0;
	    return fb->GetColorAttachmentRendererID(idx);
	}

};
