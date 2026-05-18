#pragma once

#include "pch.h"
#include "Buffer.h"
#include "RenderContext.h"
#include "RenderView.h"
#include "Render/Renderer.h"

namespace Engine
{

    class TimeStep;
    class RenderGraph;

    struct PassOutput
    {
        std::string name;
        FramebufferTextureFormat format;
        bool isDepth = false;
        // Index among color attachments only (undefined when isDepth).
        unsigned int colorAttachmentIndex = 0;
    };

    class RenderPass
    {
    public:
        using ConditionFn = std::function<bool(const RenderView&)>;

        RenderPass(const std::string& name) : m_Name(name) {}
        virtual ~RenderPass() = default;

        virtual void Setup(RenderGraph& graph) = 0;
        virtual void Execute(RenderContext& ctx, const RenderView& view) = 0;

        // Declare a named output resource. Each call appends an attachment to the pass's
        // framebuffer in declaration order. Returns the color attachment index for color
        // formats, or ~0u for depth formats.
        unsigned int Output(const std::string& resourceName, FramebufferTextureFormat format)
        {
            PassOutput out;
            out.name = resourceName;
            out.format = format;
            out.isDepth = (format == FramebufferTextureFormat::DEPTH24STENCIL8 ||
                           format == FramebufferTextureFormat::Depth);
            if (out.isDepth)
                out.colorAttachmentIndex = ~0u;
            else
                out.colorAttachmentIndex = m_ColorAttachmentCount++;
            m_Outputs.push_back(out);
            return out.colorAttachmentIndex;
        }

        // Declare a read dependency on a named resource produced by another pass.
        RenderPass& Input(const std::string& resourceName)
        {
            m_Inputs.push_back(resourceName);
            return *this;
        }

        RenderPass& SetTargetType(BufferTextureType type)
        {
            m_TargetType = type;
            return *this;
        }

        // Predicate evaluated each frame; pass is skipped when it returns false.
        RenderPass& SetCondition(ConditionFn fn)
        {
            m_Condition = std::move(fn);
            return *this;
        }

        RenderPass& SetClearColor(const glm::vec4& color)
        {
            m_ClearColor = color;
            m_HasClearColor = true;
            return *this;
        }

        // attachmentIndex is the color-attachment index returned by Output().
        RenderPass& SetClearInt(unsigned int attachmentIndex, int value)
        {
            m_ClearInts[attachmentIndex] = value;
            return *this;
        }

        void SetViewportSize(unsigned int width, unsigned int height)
        {
            m_Width = width;
            m_Height = height;
        }

        // Flatten declared outputs into a format list (for FramebufferPool).
        std::vector<FramebufferTextureFormat> BuildAttachmentList() const
        {
            std::vector<FramebufferTextureFormat> v;
            v.reserve(m_Outputs.size());
            for (auto& o : m_Outputs) v.push_back(o.format);
            return v;
        }

        bool EvaluateCondition(const RenderView& view) const
        {
            return m_Condition ? m_Condition(view) : true;
        }

        unsigned int GetWidth() const { return m_Width; }
        unsigned int GetHeight() const { return m_Height; }
        const std::string& GetName() const { return m_Name; }
        const std::vector<PassOutput>& GetOutputs() const { return m_Outputs; }
        const std::vector<std::string>& GetInputs() const { return m_Inputs; }
        BufferTextureType GetTargetType() const { return m_TargetType; }
        const glm::vec4& GetClearColor() const { return m_ClearColor; }
        bool HasClearColor() const { return m_HasClearColor; }
        const std::unordered_map<unsigned int, int>& GetClearInts() const { return m_ClearInts; }
        bool HasCondition() const { return static_cast<bool>(m_Condition); }

    protected:
        std::string m_Name;
        RenderGraph* m_RenderGraph = nullptr;
        std::vector<PassOutput> m_Outputs;
        std::vector<std::string> m_Inputs;
        unsigned int m_ColorAttachmentCount = 0;
        BufferTextureType m_TargetType = BufferTextureType::Texture2D;
        glm::vec4 m_ClearColor = glm::vec4(0.0f);
        bool m_HasClearColor = false;
        std::unordered_map<unsigned int, int> m_ClearInts;
        unsigned int m_Width = 0;
        unsigned int m_Height = 0;
        ConditionFn m_Condition;
    };

};
