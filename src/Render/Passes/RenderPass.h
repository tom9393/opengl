#pragma once

#include "pch.h"
#include "Buffer.h"
#include "RenderContext.h"
#include "RenderView.h"
#include "Render/RenderCommand.h"
#include "Render/Renderer.h"

namespace Engine
{

    class TimeStep;
    class RenderGraph;

    // Index of a declared input, returned by Input()/SetDepthInput() during Setup
    // and resolved to the producer's texture when the graph compiles.
    using InputHandle = size_t;

    struct PassOutput
    {
        std::string name;
        FramebufferTextureFormat format;
        bool isDepth = false;
        // When set, this output has no storage of its own: it names the resource
        // whose texture this pass renders into in place.
        std::string aliasOf;
    };

    enum class PassBlend { Off, Alpha, Additive };

    // Render state declared by a pass and applied by the graph before Execute,
    // so no pass inherits whatever the previous one left behind. Passes may
    // still change state mid-execution; the graph re-applies the next pass's
    // declared state regardless.
    struct PipelineState
    {
        bool depthTest = true;
        bool depthWrite = true;
        PassBlend blend = PassBlend::Off;
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
            out.isDepth = IsDepthFormat(format);
            m_Outputs.push_back(out);
            return out.isDepth ? ~0u : m_ColorAttachmentCount++;
        }

        // Declare a read dependency on a named resource produced by another pass. The
        // handle is resolved at compile time and bound with BindInput() during Execute.
        InputHandle Input(const std::string& resourceName)
        {
            m_Inputs.push_back(resourceName);
            return m_Inputs.size() - 1;
        }

        // Declare an output that renders in place into another pass's resource instead
        // of allocating its own texture. The source must be declared by a pass added
        // to the graph before this one, and is implicitly an input. The source texture
        // occupies the next color attachment slot of this pass's framebuffer; aliased
        // attachments are never cleared by the graph.
        void AliasOutput(const std::string& resourceName, const std::string& sourceResource)
        {
            PassOutput out;
            out.name = resourceName;
            out.format = FramebufferTextureFormat::None;
            out.aliasOf = sourceResource;
            m_Outputs.push_back(out);
            m_ColorAttachmentCount++;
            Input(sourceResource);
        }

        // Declare a depth resource produced by another pass to be attached as this
        // pass's depth attachment. The attachment is read-only: the graph masks depth
        // writes while the pass executes, which also keeps it legal to sample.
        InputHandle SetDepthInput(const std::string& resourceName)
        {
            m_DepthInput = resourceName;
            return Input(resourceName);
        }

        // Layer count for array target types (shadow map atlases). Ignored for plain 2D targets.
        RenderPass& SetLayers(unsigned int layers)
        {
            m_Layers = layers;
            return *this;
        }

        // Size this pass's outputs relative to the viewport (e.g. 0.5 = half
        // resolution). Scaled resources follow viewport resizes. Ignored when a
        // fixed size was set with SetViewportSize.
        RenderPass& SetSizeScale(float scale)
        {
            m_SizeScale = scale;
            return *this;
        }

        RenderPass& SetPipelineState(const PipelineState& state)
        {
            m_PipelineState = state;
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

        // Default clear value for every owned color attachment.
        RenderPass& SetClearColor(const glm::vec4& color)
        {
            m_ClearColor = color;
            return *this;
        }

        // Per-attachment clear value, overriding the pass clear color.
        // attachmentIndex is the color-attachment index returned by Output().
        RenderPass& SetClearValue(unsigned int attachmentIndex, const glm::vec4& value)
        {
            m_ClearValues[attachmentIndex] = value;
            return *this;
        }

        // attachmentIndex is the color-attachment index returned by Output().
        RenderPass& SetClearInt(unsigned int attachmentIndex, int value)
        {
            m_ClearInts[attachmentIndex] = value;
            return *this;
        }

        // Skip clearing entirely; for passes whose draws cover every pixel of
        // every attachment (fullscreen filters).
        RenderPass& DisableClear()
        {
            m_ClearDisabled = true;
            return *this;
        }

        void SetViewportSize(unsigned int width, unsigned int height)
        {
            m_Width = width;
            m_Height = height;
        }

        bool EvaluateCondition(const RenderView& view) const
        {
            return m_Condition ? m_Condition(view) : true;
        }

        unsigned int GetWidth() const { return m_Width; }
        unsigned int GetHeight() const { return m_Height; }
        unsigned int GetLayers() const { return m_Layers; }
        const std::string& GetName() const { return m_Name; }
        const std::vector<PassOutput>& GetOutputs() const { return m_Outputs; }
        const std::vector<std::string>& GetInputs() const { return m_Inputs; }
        const std::string& GetDepthInput() const { return m_DepthInput; }
        bool HasDepthInput() const { return !m_DepthInput.empty(); }
        // True when at least one output owns its texture. Alias-only passes load the
        // existing contents and are never cleared by the graph.
        bool OwnsAnOutput() const
        {
            for (auto& out : m_Outputs)
                if (out.aliasOf.empty()) return true;
            return false;
        }
        BufferTextureType GetTargetType() const { return m_TargetType; }
        float GetSizeScale() const { return m_SizeScale; }
        const PipelineState& GetPipelineState() const { return m_PipelineState; }
        bool IsClearDisabled() const { return m_ClearDisabled; }
        // Clear value for a color attachment: the per-attachment override when
        // set, otherwise the pass clear color.
        const glm::vec4& GetClearValue(unsigned int attachmentIndex) const
        {
            auto it = m_ClearValues.find(attachmentIndex);
            return it != m_ClearValues.end() ? it->second : m_ClearColor;
        }
        const std::unordered_map<unsigned int, int>& GetClearInts() const { return m_ClearInts; }
        bool HasCondition() const { return static_cast<bool>(m_Condition); }

        // The pass's framebuffer, composed by the graph at compile time from the
        // textures of its outputs (plus a read-only depth input, when declared).
        const Shared<Framebuffer>& GetFramebuffer() const { return m_Framebuffer; }

    protected:
        // Bind a declared input's texture to a sampler slot. The texture is resolved
        // at compile time and always exists while the pass is in the execution order,
        // even when its producer was skipped this frame. No-op for inputs that failed
        // to resolve.
        void BindInput(InputHandle input, unsigned int slot) const
        {
            const Shared<RenderTarget>& target = m_CompiledInputs[input];
            if (!target) return;
            RenderCommand::BindTexture(slot, target->GetID(), target->GetSpecification().type);
        }

        unsigned int GetInputTextureID(InputHandle input) const
        {
            const Shared<RenderTarget>& target = m_CompiledInputs[input];
            return target ? target->GetID() : 0;
        }

        // Current size of a declared input's texture (zero when unresolved).
        glm::uvec2 GetInputSize(InputHandle input) const
        {
            const Shared<RenderTarget>& target = m_CompiledInputs[input];
            if (!target) return { 0, 0 };
            const RenderTargetSpecification& spec = target->GetSpecification();
            return { spec.width, spec.height };
        }

        // Draw items as instanced batches of consecutive runs sharing a mesh (and a
        // material when bindMaterials), uploading per-batch transforms through the
        // frame's InstancePool.
        void DrawBatched(RenderContext& ctx, const std::vector<DrawCommand>& renderItems, bool bindMaterials)
        {
            size_t i = 0;
            while (i < renderItems.size()) {
                auto& base = renderItems[i];
                size_t batchSize = 0;

                while (i + batchSize < renderItems.size() &&
                       renderItems[i + batchSize].TargetMesh == base.TargetMesh &&
                       (!bindMaterials || renderItems[i + batchSize].TargetMaterial == base.TargetMaterial))
                    batchSize++;

                m_Transforms.clear();
                m_Transforms.reserve(batchSize);
                for (size_t j = 0; j < batchSize; j++)
                    m_Transforms.emplace_back(renderItems[i + j].Transform);

                Allocation alloc = ctx.InstancePool->Allocate(m_Transforms.data(), m_Transforms.size() * sizeof(glm::mat4));
                if (alloc.Size > 0)
                {
                    ctx.InstancePool->BindAsSSBO(0, alloc.Offset, alloc.Size);
                    Renderer::SubmitInstancedMesh(base.TargetMesh, bindMaterials ? base.TargetMaterial : nullptr, m_Transforms.size());
                }
                i += batchSize;
            }
        }

        std::string m_Name;
        RenderGraph* m_RenderGraph = nullptr;
        std::vector<PassOutput> m_Outputs;
        std::vector<std::string> m_Inputs;
        std::string m_DepthInput;
        unsigned int m_ColorAttachmentCount = 0;
        BufferTextureType m_TargetType = BufferTextureType::Texture2D;
        glm::vec4 m_ClearColor = glm::vec4(0.0f);
        bool m_ClearDisabled = false;
        std::unordered_map<unsigned int, glm::vec4> m_ClearValues;
        std::unordered_map<unsigned int, int> m_ClearInts;
        unsigned int m_Width = 0;
        unsigned int m_Height = 0;
        unsigned int m_Layers = 1;
        float m_SizeScale = 1.0f;
        PipelineState m_PipelineState;
        ConditionFn m_Condition;

    private:
        // Compiled state, filled by RenderGraph::Compile.
        friend class RenderGraph;
        std::vector<Shared<RenderTarget>> m_CompiledInputs; // parallel to m_Inputs
        Shared<Framebuffer> m_Framebuffer;

        std::vector<glm::mat4> m_Transforms;
    };

};
