#pragma once

#include "../../pch.h"
#include "../../Scene/Components.h"
#include "../../Scene/Entity.h"
#include "../../Asset/AssetManager.h"

namespace Engine {

    inline static void DrawMesh(Entity &entity, MeshComponent &component)
    {
			ImGui::Button("Mesh", ImVec2(100.0f, 0.0f));
			if (ImGui::BeginDragDropTarget())
			{
			    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetType::Mesh"))
			    {
					UUID handle = *(UUID*)payload->Data;
					component.MeshHandle = handle;
			    }
			    ImGui::EndDragDropTarget();
			}

			if (ImGui::TreeNodeEx("Mesh Attributes", ImGuiTreeNodeFlags_DefaultOpen))
			{
				// 1. Fetch the mesh from the AssetManager using the component's handle
				Shared<Mesh> mesh = AssetManager::GetAsset<Mesh>(component.MeshHandle);
				
				if (mesh)
				{
					auto vertexArray = mesh->GetVAO(); 
					auto indexBuffer = vertexArray->GetIndexBuffer();

					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();

					// 2. Fetch ALL Vertex Buffers
					const auto& vertexBuffers = vertexArray->GetVertexBuffers();
					
					// This tracks the OpenGL layout(location = X) index across ALL buffers
					uint32_t globalAttributeIndex = 0; 

					// 3. Iterate through every VBO
					for (size_t i = 0; i < vertexBuffers.size(); i++)
					{
						auto& vertexBuffer = vertexBuffers[i];
						
						// Push a unique ID for this specific VBO block so ImGui doesn't mix up the tables
						ImGui::PushID((int)i); 
						
						// Create a collapsible header for each VBO
						if (ImGui::TreeNodeEx("VBO_Node", ImGuiTreeNodeFlags_DefaultOpen, "Vertex Buffer %zu", i))
						{
							if (ImGui::BeginTable("BufferLayoutTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
							{
								ImGui::TableSetupColumn("Active", ImGuiTableColumnFlags_WidthFixed, 40.0f);
								ImGui::TableSetupColumn("Name");
								ImGui::TableSetupColumn("Offset");
								ImGui::TableSetupColumn("Size");
								ImGui::TableHeadersRow();

								auto& layout = vertexBuffer->GetLayout(); 

								for (auto& element : layout)
								{
									ImGui::TableNextRow();
									ImGui::TableSetColumnIndex(0);
									
									// Push the global attribute index as the ID for the checkbox
									ImGui::PushID(globalAttributeIndex);
									
									if (ImGui::Checkbox("##enabled", const_cast<bool*>(&element.Enabled)))
									{
										vertexArray->Bind();
										if (element.Enabled) {
											vertexBuffer->EnableAttribute(globalAttributeIndex);
										} else {
											vertexBuffer->DisableAttribute(globalAttributeIndex);
										}
										vertexArray->Unbind();
									}
									
									ImGui::PopID(); // Pop Checkbox ID

									ImGui::TableSetColumnIndex(1);
									ImGui::Text("%s", element.Name.c_str());

									ImGui::TableSetColumnIndex(2);
									ImGui::Text("%d", element.Offset);

									ImGui::TableSetColumnIndex(3);
									ImGui::Text("%d bytes", element.Size);

									// Increment layout location for the next attribute!
									globalAttributeIndex++;
								}
								ImGui::EndTable();
							}
							ImGui::TreePop(); // Pop VBO TreeNode
						}
						ImGui::PopID(); // Pop VBO Block ID
						ImGui::Spacing();
					}
				}
				else
				{
					ImGui::TextDisabled("No Mesh Loaded");
				}
				ImGui::TreePop(); // Pop main Mesh Attributes TreeNode
			}
		};
}