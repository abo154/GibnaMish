#pragma once
#ifndef _OPTIONS_HPP_
#define _OPTIONS_HPP_

#include <unordered_map>
#include <iostream>
#include <cstdint>
#include <string>

#include "str_utils.hpp"
#include "types.hpp"

struct Option
{
	std::string name;
	std::string type;
	std::string value;
	std::string default_value;
	std::string min;
	std::string max;
};

class Options
{
public:
	void print_options() const;
	void add_option(const Option&);
	void set_option(const std::string&);
	template<typename T> [[nodiscard]] T get(const std::string&);
private:
	void handle_set_option(const std::string&, const std::string&);
private:
	std::unordered_map<std::string, Option> options;
};

void Options::handle_set_option(const std::string& name, const std::string& value)
{
	if (this->options.find(name) == this->options.end())
	{
		std::cout << ("Unrecognized option: " + name) << std::endl;
		return;
	}

	if (this->options[name].type == "check")
	{
		if (value == "true" || value == "false")
			this->options[name].value = value;
		else
		{
			std::cout << ("Invalid value for option " + name + ": " + value);
			std::exit(1);
		}
	}
	else if (this->options[name].type == "spin")
	{
		int32_t val = std::stoi(value);
		if (val >= std::stoi(this->options[name].min) && val <= std::stoi(this->options[name].max))
			this->options[name].value = value;
		else
		{
			std::cout << ("Invalid value for option " + name + ": " + value);
			std::exit(1);
		}
	}
	else if (this->options[name].type == "string")
		this->options[name].value = value;
}

void Options::set_option(const std::string& line)
{
	std::vector<std::string> tokens = str_utils::split(line, ' ');

	if (tokens.size() < 5)
	{
		std::cout << ("Invalid option command");
		std::exit(1);
	}

	if (tokens[1] != "name")
	{
		std::cout << ("Invalid option command");
		std::exit(1);
	}

	if (tokens[3] != "value")
	{
		std::cout << ("Invalid option command");
		std::exit(1);
	}

	this->handle_set_option(tokens[2], line.substr(line.find("value") + 6));
}

void Options::add_option(const Option& option)
{
	this->options[option.name] = option;
}

template<typename T>
T Options::get(const std::string& name)
{
	if constexpr (std::is_same_v<T, int>)
		return std::stoi(this->options[name].value);

	else if constexpr (std::is_same_v<T, float>)
		return std::stof(this->options[name].value);

	else if constexpr (std::is_same_v<T, uint64_t>)
		return std::stoull(this->options[name].value);

	else if constexpr (std::is_same_v<T, bool>)
		return this->options[name].value == "true";

	else
		return this->options[name].value;
}

void Options::print_options() const
{
	for (const auto& [name, option] : this->options)
	{
		std::cout << "option name " << name << " type " << option.type << " default " << option.default_value;

		if (!option.min.empty())
			std::cout << " min " << option.min << " max " << option.max;

		std::cout << std::endl;
	}
}

#endif // !_OPTIONS_HPP_