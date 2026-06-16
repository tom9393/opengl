#pragma once

#include "pch.h"
#include "Asset/AssetManager.h"
#include "Scene/Components.h"
#include "Scene/Entity.h"

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
				Shared<Mesh> mesh = AssetManager::GetAsset<Mesh>(component.MeshHandle);
				
				if (mesh)
				{
					auto vertexArray = mesh->GetVAO(); 
					auto indexBuffer = vertexArray->GetIndexBuffer();

					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();

					const auto& vertexBuffers = vertexArray->GetVertexBuffers();
					
					uint32_t globalAttributeIndex = 0; 

					for (size_t i = 0; i < vertexBuffers.size(); i++)
					{
						auto& vertexBuffer = vertexBuffers[i];
						
						// Push a unique ID for this specific VBO block so ImGui doesn't mix up the tables
						ImGui::PushID((int)i); 
						
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

									globalAttributeIndex++;
								}
								ImGui::EndTable();
							}
							ImGui::TreePop();
						}
						ImGui::PopID();
						ImGui::Spacing();
					}
				}
				else
				{
					ImGui::TextDisabled("No Mesh Loaded");
				}
				ImGui::TreePop();
			}
		};
}