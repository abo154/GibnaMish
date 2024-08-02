#pragma once
#ifndef _EVALUATE_HPP_
#define _EVALUATE_HPP_

#include "chesslib/chess.hpp"

class Evaluate
{
public:
	Evaluate();
	int evaluate(const chess::Board&);
private:
	using MG_EG = std::pair<int, int>;
	using Indexes = std::vector<int>;
	using PieceType = chess::PieceType;
	using Color = chess::Color;

	int ForceKingToCenter(const chess::Board&, const chess::Color);
	MG_EG Calc_adjustment(const chess::Board&);
};

#endif // !_EVALUATE_HPP_