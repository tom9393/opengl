#pragma once

#include "pch.h"

namespace Engine {

	inline static bool DrawVec3(const std::string &label, glm::vec3 &values, float resetValue = 1.0f, float columnWidth = 100.0f)
	{
		bool modified = false;

		ImGuiIO &io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		// Persist selection per-control (keyed by label)
		static std::unordered_map<std::string, int> s_selectedAxis;

		// ensure a default of -1 (no axis selected)
		if (s_selectedAxis.find(label) == s_selectedAxis.end())
		s_selectedAxis[label] = -1;

		int &selected = s_selectedAxis[label];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::TextUnformatted(label.c_str());
		ImGui::NextColumn();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{4, 4});

		// button sizing
		float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 4.0f, lineHeight };

		const char* axisNames[3] = { "X", "Y", "Z" };
		float* axisVals[3] = { &values.x, &values.y, &values.z };
		ImVec4 axisColors[3] = {
		ImVec4(0.75f, 0.12f, 0.12f, 1.0f), // red-ish X
		ImVec4(0.12f, 0.75f, 0.12f, 1.0f), // green-ish Y
		ImVec4(0.12f, 0.25f, 0.75f, 1.0f)  // blue-ish Z
		};

		// Render three inline axis controls: [Button(X)] [DragFloat]
		for (int i = 0; i < 3; ++i)
		{
			ImGui::PushID(i);

			bool isSelected = (selected == i);

			// color the button if selected (and slightly darker when not)
			ImVec4 baseBtnColor = ImGui::GetStyleColorVec4(ImGuiCol_Button);
			ImVec4 btnColor = isSelected ? axisColors[i] : ImVec4(baseBtnColor.x * 0.7f, baseBtnColor.y * 0.7f, baseBtnColor.z * 0.7f, baseBtnColor.w);

			ImGui::PushStyleColor(ImGuiCol_Button, btnColor);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(btnColor.x + 0.08f, btnColor.y + 0.08f, btnColor.z + 0.08f, btnColor.w));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(btnColor.x - 0.08f, btnColor.y - 0.08f, btnColor.z - 0.08f, btnColor.w));
			ImGui::PushFont(boldFont);

			if (ImGui::Button(axisNames[i], buttonSize))
			{
				// toggle selection (click same axis again to deselect)
				selected = (isSelected ? -1 : i);
				// optional: reset when selecting
				if (!isSelected)
				{
					*axisVals[i] = resetValue;
					modified = true;
				}
			}

			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();

			// unique ID for the DragFloat. Use hidden label with the main label to avoid collisions.
			char dragLabel[64];
			std::snprintf(dragLabel, sizeof(dragLabel), "##%s_%c", label.c_str(), 'X' + i);

			// set width and render DragFloat
			ImGui::SetNextItemWidth(120.0f);
			if (ImGui::DragFloat(dragLabel, axisVals[i], 0.1f))
				modified = true;

			ImGui::PopID();

			if (i < 2) ImGui::SameLine();
		}

		ImGui::PopStyleVar(); // ItemSpacing
		ImGui::Columns(1);
		ImGui::PopID();

		return modified;
	}
}