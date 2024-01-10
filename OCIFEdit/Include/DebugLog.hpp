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
	enum MessageType
	{
		Info,
		Warning,
		Error
	};

	DebugLog() = default;
	DebugLog(const DebugLog& copy) = delete;

	void append(MessageType type, std::string_view text);

	template<typename ...Args>
	void print(MessageType type, std::string_view format, Args&&... args);

	template<typename ...Args>
	constexpr void info(std::string_view format, Args&&... args)
	{
		print(Info, format, args...);
	}

	template<typename ...Args>
	constexpr void warn(std::string_view format, Args&&... args)
	{
		print(Warning, format, args...);
	}

	template<typename ...Args>
	constexpr void error(std::string_view format, Args&&... args)
	{
		print(Error, format, args...);
	}

	void draw(bool* open);

protected:
	static constexpr size_t s_buffer_limit = 100;

	std::deque<std::pair<MessageType, std::string>> m_buffer;
	bool m_should_scroll_to_bottom { false };

};

//===========================================

template<typename ...Args>
void DebugLog::print(MessageType type, std::string_view format, Args&&... args)
{
	append(type, std::vformat(format, std::make_format_args(args...)));
}

//===========================================