#include "FramebufferPool.h"
#include "Buffer.h"

namespace Engine
{

	Shared<FramebufferHandle> FramebufferPool::AcquireFramebuffer(const std::string& passName, const std::vector<FramebufferTextureFormat>& attachments, BufferTextureType type, unsigned int width, unsigned int height)
	{
	    std::string key = BuildKey(passName, attachments, type, width, height);
	    auto it = m_Pool.find(key);
	    if (it != m_Pool.end())
		return it->second;

	    std::vector<FramebufferTextureSpecification> specs;
	    for (auto &attachment : attachments)
		{
			specs.emplace_back(attachment);
		}

	    FramebufferSpecification spec;
	    spec.width = width;
	    spec.height = height;
	    spec.samples = 1;
		spec.layers = 64;
		spec.type = type;
	    spec.attachments = specs;

	    Shared<Framebuffer> fb = Framebuffer::Create(spec);
	    if (!fb)
		return nullptr;

	    Shared<FramebufferHandle> handle = MakeShared<FramebufferHandle>(fb);
	    m_Pool.emplace(key, handle);
	    return handle;
	}

	void FramebufferPool::Clear()
	{
	    m_Pool.clear();
	}

	std::string FramebufferPool::BuildKey(const std::string& passName, const std::vector<FramebufferTextureFormat>& attachments, BufferTextureType type, unsigned int width, unsigned int height) const
	{
		std::string key = passName + "_" + std::to_string(width) + "x" + std::to_string(height) + "_type" + std::to_string((int)type) + "_";
		for (auto format : attachments)
			key += std::to_string((int)format) + "_";
		return key;
	}

};
