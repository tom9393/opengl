#pragma once

#include <functional>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "../Buffer.h"

namespace Engine
{

    class TimeStep;

    // Very small RenderPass description for the RenderGraph
    class RenderPass
    {
    public:
        using Callback = std::function<void(TimeStep)>;

        RenderPass(const std::string& name) : m_Name(name) {}

        // Attachments is the list of FramebufferTextureFormat in the same order the framebuffer expects.
        RenderPass& SetAttachments(const std::vector<FramebufferTextureFormat>& attachments)
        {
            m_Attachments = attachments;
            return *this;
        }

        // Set clear color for the pass (applied via RenderCommand::SetClearColor before RenderCommand::Clear())
        RenderPass& SetClearColor(const glm::vec4& color)
        {
            m_ClearColor = color;
            m_HasClearColor = true;
            return *this;
        }

        // Set clear int for a color attachment index (useful for integer attachments like RED_INTEGER)
        RenderPass& SetClearInt(unsigned int attachmentIndex, int value)
        {
            m_ClearInts[attachmentIndex] = value;
            return *this;
        }

        // Provide the draw callback (gets invoked with the TimeStep for the frame)
        RenderPass& SetCallback(Callback cb)
        {
            m_Callback = std::move(cb);
            return *this;
        }

        const std::string& GetName() const { return m_Name; }
        const std::vector<FramebufferTextureFormat>& GetAttachments() const { return m_Attachments; }
        const glm::vec4& GetClearColor() const { return m_ClearColor; }
        bool HasClearColor() const { return m_HasClearColor; }
        const std::unordered_map<unsigned int, int>& GetClearInts() const { return m_ClearInts; }
        Callback GetCallback() const { return m_Callback; }

    private:
        std::string m_Name;
        std::vector<FramebufferTextureFormat> m_Attachments;
        glm::vec4 m_ClearColor = glm::vec4(0.0f);
        bool m_HasClearColor = false;
        std::unordered_map<unsigned int, int> m_ClearInts;
        Callback m_Callback;
    };

};
