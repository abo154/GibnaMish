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

class Options {
public:
    void print() const {
        for (const auto &[name, option]: options_) {
            std::cout << "option name " << name << " type " << option.type << " default "
                    << option.default_value;
            if (!option.min.empty()) {
                std::cout << " min " << option.min << " max " << option.max;
            }
            std::cout << std::endl;
        }
    }

    void add(const Option &option) { options_[option.name] = option; }

    void set(const std::string &line) {
        std::vector<std::string> tokens = str_util::splitString(line, ' ');

        if (tokens.size() < 5) {
            std::cout << ("Invalid option command");
            std::exit(1);
        }

        if (tokens[1] != "name") {
            std::cout << ("Invalid option command");
            std::exit(1);
        }

        if (tokens[3] != "value") {
            std::cout << ("Invalid option command");
            std::exit(1);
        }

        set(tokens[2], line.substr(line.find("value") + 6));
    }

    template<typename T>
    [[nodiscard]] T get(const std::string &name) {
        if constexpr (std::is_same_v<T, int>)
            return std::stoi(options_[name].value);
        else if constexpr (std::is_same_v<T, float>)
            return std::stof(options_[name].value);
        else if constexpr (std::is_same_v<T, uint64_t>)
            return std::stoull(options_[name].value);
        else if constexpr (std::is_same_v<T, bool>)
            return options_[name].value == "true";
        else
            return options_[name].value;
    }

private:
    void set(const std::string &name, const std::string &value) {
        if (options_.find(name) == options_.end()) {
            std::cout << ("Unrecognized option: " + name) << std::endl;
            return;
        }

        if (options_[name].type == "check") {
            if (value == "true" || value == "false") {
                options_[name].value = value;
            } else {
                std::cout << ("Invalid value for option " + name + ": " + value);
                std::exit(1);
            }
        } else if (options_[name].type == "spin") {
            int32_t val = std::stoi(value);
            if (val >= std::stoi(options_[name].min) && val <= std::stoi(options_[name].max)) {
                options_[name].value = value;
            } else {
                std::cout << ("Invalid value for option " + name + ": " + value);
                std::exit(1);
            }
        } else if (options_[name].type == "string") {
            options_[name].value = value;
        }
    }

    std::unordered_map<std::string, Option> options_;
};

#endif // !_OPTIONS_HPP_