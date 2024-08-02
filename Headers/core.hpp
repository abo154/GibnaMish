#pragma once
#ifndef _CORE_HPP_
#define _CORE_HPP_

#include "time.hpp"
#include "searcher.hpp"
#include "chesslib/chess.hpp"

class Core
{
	using Move = chess::Move;
	using MoveValue = std::pair<Move, int>;
public:
	Core();
	void setMoves(const std::vector<std::string>&);
	void set_fen(std::string&);
	void get_score_move(const uint32_t, const size_t, const size_t, const size_t, const size_t, const bool);
	void draw() const;
	void Stop_Thinking();
	void reset_board();
private:
	TranspositionTable tt;
	chess::Board Board;
	Searcher searcher;
	Time Timer;
};

#endif // !_CORE_HPP_