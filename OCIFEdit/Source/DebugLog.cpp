#include <DebugLog.hpp>
#include <imgui.h>

//===========================================

void DebugLog::append(std::string_view text)
{
	m_buffer.push_back(std::string(text));
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

	for (const auto& line: m_buffer)
		ImGui::TextUnformatted(line.c_str());

	if (m_should_scroll_to_bottom)
	{
		ImGui::SetScrollHereY(1.f);
		m_should_scroll_to_bottom = false;
	}

	ImGui::End();
}

//===========================================