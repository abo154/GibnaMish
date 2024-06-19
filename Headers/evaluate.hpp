#pragma once
#ifndef _EVALUATE_HPP_
#define _EVALUATE_HPP_

#include "nnue.hpp"
#include "chesslib/chess.hpp"

class Evaluate
{
public:
	Evaluate();
	inline int evaluate(const chess::Board&);
	inline void init_NNUE(const char*);
	inline bool is_avilable() const;
private:
	using MG_EG = std::pair<int, int>;
	using PieceType = chess::PieceType;
	using Color = chess::Color;

	int pieces[33];
	int squares[33];
	NNUE nnue;

	inline const int ForceKingToCenter(const chess::Board&, const chess::Color);
	inline const MG_EG Calc_adjustment(const chess::Board&);
	inline const int NNUE_evaluate(const chess::Board&);
	const int Manuale_evaluate(const chess::Board&);
	void Handle_NNUE_Inputs(const chess::Board&);
};

inline void Evaluate::init_NNUE(const char* evalFile)
{
	this->nnue.init(evalFile);
}

inline bool Evaluate::is_avilable() const
{
	return this->nnue.Avilable;
}

inline const int Evaluate::NNUE_evaluate(const chess::Board& Board)
{
	this->Handle_NNUE_Inputs(Board);
	return this->nnue.evaluate(Board.sideToMove(), this->pieces, this->squares);
}

inline int Evaluate::evaluate(const chess::Board& Board)
{
	return this->nnue.Avilable ? this->NNUE_evaluate(Board) : this->Manuale_evaluate(Board);
}

#endif // !_EVALUATE_HPP_