#include "FramebufferPool.h"
#include "../Buffer.h"
#include <sstream>

namespace Engine
{

	Shared<Framebuffer> FramebufferPool::AcquireFramebuffer(const std::vector<FramebufferTextureFormat>& attachments, unsigned int width, unsigned int height)
	{
	    std::string key = BuildKey(attachments, width, height);
	    auto it = m_Pool.find(key);
	    if (it != m_Pool.end())
		return it->second;

	    // Build FramebufferSpecification consistent with the rest of your codebase
	    FramebufferSpecification spec;
	    spec.width = width;
	    spec.height = height;
	    spec.samples = 1;
	    // reinterpret existing API: earlier you set spec.attachments directly to FramebufferTextureFormat list
	    spec.attachments = attachments;

	    Shared<Framebuffer> fb = Framebuffer::Create(spec);
	    m_Pool.emplace(key, fb);
	    return fb;
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
