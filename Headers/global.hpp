#pragma once
#ifndef _GLOBAL_HPP_
#define _GLOBAL_HPP_

#include "chesslib/chess.hpp"

namespace global
{
	inline constexpr int CHECKMATE = 2000000000;
	inline constexpr int MAX_PLY = 120;
	inline constexpr int VALUE_MATE = CHECKMATE - MAX_PLY;
	inline constexpr unsigned short MAX_DEPTH = 100;
	inline constexpr int lmpM[4] = { 0, 8, 12, 24 };

	bool is_checkmate(const chess::Board&, const size_t);
	bool is_Draw(const chess::Board&, const size_t);
	int is_mate_score(const int score);
	bool NonPawnMaterial(const chess::Board&, const chess::Color);
}

#endif // !_GLOBAL_HPP_