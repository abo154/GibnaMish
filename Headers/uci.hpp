#pragma once
#ifndef _UCI_HPP_
#define _UCI_HPP_

#include "time.hpp"
#include "threadmanager.hpp"

class UCI
{
public:
	UCI();
	void Main_Loop();
private:
	chess::Board Board;
	chess::Movelist Searchmoves;
	int worker_threads;
	bool use_tb = false;

	void Response(const std::string&);
	void Process_Position_Command(const std::string&);
	void Process_Set_Options_Command(const std::string&);
	void Process_Go_Command(const std::string&);
	void Uci_New_Game();
	void Uci();
	void ApplyOptions();
};

#endif // !_UCI_HPP_