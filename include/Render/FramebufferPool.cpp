#include "FramebufferPool.h"
#include "../Buffer.h"
#include <sstream>

namespace Engine
{

	Shared<FramebufferHandle> FramebufferPool::AcquireFramebuffer(const std::vector<FramebufferTextureFormat>& attachments, unsigned int width, unsigned int height)
	{
	    std::string key = BuildKey(attachments, width, height);
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

	std::string FramebufferPool::BuildKey(const std::vector<FramebufferTextureFormat>& attachments, unsigned int width, unsigned int height) const
	{
	    std::ostringstream oss;
	    oss << width << "x" << height << ":";
	    for (auto a : attachments)
		oss << static_cast<int>(a) << ",";
	    return oss.str();
	}

};
