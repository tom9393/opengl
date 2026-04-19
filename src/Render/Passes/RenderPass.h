#pragma once

#include <functional>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "Buffer.h"
#include "RenderContext.h"
#include "RenderView.h"

namespace Engine
{

    class TimeStep;
    class RenderGraph;

    // Very small RenderPass description for the RenderGraph
    class RenderPass
    {
    public:
        RenderPass(const std::string& name) : m_Name(name) {}

        virtual void Setup(RenderGraph& graph) = 0;
        virtual void Execute(RenderContext& ctx, const RenderView& view) = 0;

        // Attachments is the list of FramebufferTextureFormat in the same order the framebuffer expects.
        RenderPass& SetAttachments(const std::vector<FramebufferTextureFormat>& attachments, BufferTextureType type = BufferTextureType::None)
        {
            m_Attachments = attachments;
            m_TargetType = type;
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

        unsigned int GetWidth() const { return m_Width; }
        unsigned int GetHeight() const { return m_Height; }

        void SetViewportSize(unsigned int width, unsigned int height)
        {
            m_Width = width;
            m_Height = height;
        }

        const std::string& GetName() const { return m_Name; }
        const std::vector<FramebufferTextureFormat>& GetAttachments() const { return m_Attachments; }
        BufferTextureType GetTargetType() const { return m_TargetType; }
        const glm::vec4& GetClearColor() const { return m_ClearColor; }
        bool HasClearColor() const { return m_HasClearColor; }
        const std::unordered_map<unsigned int, int>& GetClearInts() const { return m_ClearInts; }

    protected:
        std::string m_Name;
        RenderGraph* m_RenderGraph;
        std::vector<FramebufferTextureFormat> m_Attachments;
        BufferTextureType m_TargetType = BufferTextureType::None;
        glm::vec4 m_ClearColor = glm::vec4(0.0f);
        bool m_HasClearColor = false;
        std::unordered_map<unsigned int, int> m_ClearInts;
        unsigned int m_Width = 0;
        unsigned int m_Height = 0;
    };

};
