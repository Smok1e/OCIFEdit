#include <DebugLog.hpp>
#include <imgui.h>

//===========================================

void DebugLog::append(MessageType type, std::string_view text)
{
	m_buffer.emplace_back(type, text);
	if (m_buffer.size() > s_buffer_limit)
		m_buffer.pop_front();

	m_should_scroll_to_bottom = true;
}

//===========================================

void DebugLog::draw(bool* open)
{
	if (!ImGui::Begin("Debug log", open, ImGuiWindowFlags_HorizontalScrollbar))
	{
		ImGui::End();
		return;
	}

	for (const auto& [type, text]: m_buffer)
	{
		const char* tag = "WTF?";
		ImVec4 color(1.f, 1.f, 1.f, 1.f);

		switch (type)
		{
			case Info:
				tag = "INFO";
				color = ImVec4(0.f, .7f, 1.f, 1.f);
				break;

			case Warning:
				tag = "WARN";
				color = ImVec4(1.f, .8f, 0.f, 1.f);
				break;

			case Error:
				tag = "ERROR";
				color = ImVec4(1.f, .3f, .3f, 1.0);
				break;
		}

		ImGui::TextColored(color, "[%s]: ", tag);
		ImGui::SameLine();
		ImGui::Text(text.c_str());
	}

	if (m_should_scroll_to_bottom)
	{
		ImGui::SetScrollHereY(1.f);
		m_should_scroll_to_bottom = false;
	}

	ImGui::End();
}

//===========================================