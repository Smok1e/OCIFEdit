#pragma once
#include <deque>
#include <string>
#include <cstdlib>
#include <format>

//===========================================

// Class for debug logging and displaying log in ImGui window
class DebugLog
{
public:
	DebugLog() = default;
	DebugLog(const DebugLog& copy) = delete;

	void append(std::string_view text);

	template<typename ...Args>
	void print(std::string_view tag, std::string_view format, Args&&... args);

	template<typename ...Args>
	void info(std::string_view text, Args&&... args);

	template<typename ...Args>
	void warn(std::string_view text, Args&&... args);

	void draw(bool* open);

protected:
	static constexpr size_t s_buffer_limit = 100;

	std::deque<std::string> m_buffer {};
	bool                    m_should_scroll_to_bottom { false };

};

//===========================================

template<typename ...Args>
void DebugLog::print(std::string_view tag, std::string_view format, Args&&... args)
{
	append(std::format("[{}]: {}", tag, std::vformat(format, std::make_format_args(args...))));
}

template<typename ...Args>
void DebugLog::info(std::string_view format, Args&&... args)
{
	print("INFO", format, args...);
}

template<typename ...Args>
void DebugLog::warn(std::string_view format, Args&&... args)
{
	print("WARN", format, args...);
}

//===========================================