#pragma once
#ifndef _STR_UTILS_HPP_
#define _STR_UTILS_HPP_

#include <optional>
#include <sstream>
#include <vector>
#include <string>

namespace str_utils
{
	[[nodiscard]] inline std::vector<std::string> split(const std::string& string, char delim)
	{
		std::vector<std::string> splitted;
		std::string line;
		std::stringstream ss(string);

		while (std::getline(ss, line, delim))
			splitted.emplace_back(line);

		return splitted;
	}

	[[nodiscard]] inline int64_t find_range(const std::string& string, const std::string& start, const std::string& end)
	{
		const auto start_pos = string.find(start) + start.length();
		const auto end_pos = string.find(end);

		if (start_pos == std::string::npos || end_pos == std::string::npos)
			return -1;

		return end_pos - start_pos;
	}

	[[nodiscard]] inline bool is_contain(const std::vector<std::string>& elements, std::string_view element)
	{
		return std::find(elements.begin(), elements.end(), element) != elements.end();
	}

	[[nodiscard]] inline bool is_contain(std::string_view string, std::string_view small_string)
	{
		return string.find(small_string) != std::string::npos;
	}

	template <typename T>
	[[nodiscard]] inline std::optional<T> find_element_value(const std::vector<std::string>& tokens, std::string_view element)
	{
		const auto element_pos = std::find(tokens.begin(), tokens.end(), element);
		const auto value_index = (element_pos - tokens.begin()) + 1;

		if (element_pos == tokens.end() || value_index >= tokens.size()) return std::nullopt;

		const std::string& value = tokens[value_index];

		if constexpr (std::is_same_v<T, int>)
			return std::stoi(value);
		else if constexpr (std::is_same_v<T, float>)
			return std::stof(value);
		else if constexpr (std::is_same_v<T, uint64_t>)
			return std::stoull(value);
		else if constexpr (std::is_same_v<T, int64_t>)
			return std::stoll(value);
		else if constexpr (std::is_same_v<T, bool>)
			return value == "true";
		else if (size_t(value_index) < tokens.size())
			return value;

		return std::nullopt;
	}
}

#endif // !_STR_UTILS_HPP_