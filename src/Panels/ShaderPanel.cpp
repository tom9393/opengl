#include "ShaderPanel.h"
#include "../Render/Shader.h"
#include <imgui.h>

namespace Engine {

    static const char* GLTypeToString(uint32_t type)
    {
        switch (type)
        {
            case GL_FLOAT:             return "float";
            case GL_FLOAT_VEC2:        return "vec2";
            case GL_FLOAT_VEC3:        return "vec3";
            case GL_FLOAT_VEC4:        return "vec4";
            case GL_FLOAT_MAT2:        return "mat2";
            case GL_FLOAT_MAT3:        return "mat3";
            case GL_FLOAT_MAT4:        return "mat4";
            case GL_INT:               return "int";
            case GL_INT_VEC2:          return "ivec2";
            case GL_INT_VEC3:          return "ivec3";
            case GL_INT_VEC4:          return "ivec4";
            case GL_UNSIGNED_INT:      return "uint";
            case GL_BOOL:              return "bool";
            case GL_SAMPLER_2D:        return "sampler2D";
            case GL_SAMPLER_3D:        return "sampler3D";
            case GL_SAMPLER_CUBE:      return "samplerCube";
            default:                   return "Unknown";
        }
    }

    void ShaderPanel::OnImGuiRender()
    {
        ImGui::Begin("Shader Inspector");

        auto& shaders = ShaderLibrary::GetShaders();

        for (const auto& [name, shader] : shaders)
        {
            if (ImGui::TreeNode(name.c_str()))
            {
                // --- 1. RENDER VERTEX ATTRIBUTES ---
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Vertex Attributes");
                auto& attributes = shader->GetActiveAttributes();
                
                if (attributes.empty()) {
                    ImGui::TextDisabled("No active attributes found.");
                } else {
                    if (ImGui::BeginTable("AttributesTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
                    {
                        ImGui::TableSetupColumn("Location", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                        ImGui::TableSetupColumn("Name");
                        ImGui::TableSetupColumn("Type (GLenum)");
                        ImGui::TableSetupColumn("Size");
                        ImGui::TableHeadersRow();

                        for (const auto& attr : attributes)
                        {
                            ImGui::TableNextRow();
                            
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("%d", attr.Location);
                            
                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text("%s", attr.Name.c_str());
                            
                            ImGui::TableSetColumnIndex(2);
                            ImGui::Text("%s", GLTypeToString(attr.Type));
                            
                            ImGui::TableSetColumnIndex(3);
                            ImGui::Text("%d", attr.Size);
                        }
                        ImGui::EndTable();
                    }
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // --- 2. RENDER UNIFORMS ---
                ImGui::TextColored(ImVec4(0.4f, 0.6f, 0.9f, 1.0f), "Uniforms");
                auto& uniforms = shader->GetActiveUniforms();

                if (uniforms.empty()) {
                    ImGui::TextDisabled("No active uniforms found.");
                } else {
                    if (ImGui::BeginTable("UniformsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
                    {
                        ImGui::TableSetupColumn("Name");
                        ImGui::TableSetupColumn("Type (GLenum)");
                        ImGui::TableSetupColumn("Size");
                        ImGui::TableHeadersRow();

                        for (const auto& uniform : uniforms)
                        {
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("%s", uniform.Name.c_str());
                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text("%s", GLTypeToString(uniform.Type));
                            ImGui::TableSetColumnIndex(2);
                            ImGui::Text("%d", uniform.Size);
                        }
                        ImGui::EndTable();
                    }
                }
                
                ImGui::TreePop();
            }
        }

        ImGui::End();
    }

}