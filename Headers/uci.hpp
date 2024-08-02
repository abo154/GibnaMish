#pragma once
#ifndef _UCI_HPP_
#define _UCI_HPP_

#include "core.hpp"
#include "../pystring/pystring.hpp"

class UCI
{
public:
	void Response(const std::string&);
	void Main_Loop();
private:
	Core core;

	void Process_Position_Command(const std::string&);
	void Process_Set_Options_Command(const std::string&);
	void Process_Go_Command(const std::string&);
};

#endif // !_UCI_HPP_