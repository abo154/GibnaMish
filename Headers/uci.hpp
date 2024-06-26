#pragma once
#ifndef _UCI_HPP_
#define _UCI_HPP_

#include <utility>
#include <iostream>

#include "core.hpp"
#include "../pystring/pystring.hpp"

class UCI
{
public:
	void Response(const std::string&);
	inline void Main_Loop();
private:
	Core core;

	inline void Process_Position_Command(const std::string&);
	inline void Process_Set_Options_Command(const std::string&);
	inline void Process_Go_Command(const std::string&);
};

#endif // !_UCI_HPP_