#pragma once
#ifndef _MOVE_PICKER_HPP_
#define _MOVE_PICKER_HPP_

#include <algorithm>

#include "see.hpp"
#include "types.hpp"
#include "table.hpp"
#include "history.hpp"
#include "searcher.hpp"
#include "piecesbouns.hpp"
#include "chesslib/chess.hpp"
#include "transpositiontable.hpp"

inline constexpr int mvvlvaArray[8][8] = {
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 25, 24, 23, 22, 21, 20, 0},
	{0, 35, 34, 33, 32, 31, 30, 0},
	{0, 45, 44, 43, 42, 41, 40, 0},
	{0, 55, 54, 53, 52, 51, 50, 0},
	{0, 65, 64, 63, 62, 61, 60, 0},
	{0, 75, 74, 73, 72, 71, 70, 0}
};


enum SearchType: int8_t
{
	ABSEARCH,
	QSEARCH
};

template<SearchType st>
class MovePicker
{
	using Color = chess::Color;
	using Piece = chess::Piece;
	using Square = chess::Square;
	using PieceType = chess::PieceType;
public:
	MovePicker(const Searcher&, const Stack*, Movelist&, const Move);
	MovePicker(const Searcher&, const Stack*, Movelist&, const Movelist&, const bool, const Move);
	[[nodiscard]] Move nextMove();
private:
	void score_moves();
	[[nodiscard]] int16_t score_move(const Move);
	[[nodiscard]] int16_t mvvlva(const Move) const;
private:
	enum Biases : int16_t
	{
		STANDARD = 10,

		TTBias = 50 * STANDARD,
		winningCaptureBias = 5 * STANDARD,
		promoteBias = 4 * STANDARD,
		killerBias = 3 * STANDARD,
		counterBias = 2 * STANDARD,
		losingCaptureBias = 1 * STANDARD,
		regularBias = 0 * STANDARD,
		negativeBias = -1 * TTBias
	};
	const Searcher& searcher;
	const Stack* ss;
	Movelist& moves;
	int played;
	Move tt_move;
};

template<SearchType st>
MovePicker<st>::MovePicker(const Searcher& searcher, const Stack* ss, Movelist& moves, const Move tt_move):
	played(0),
	searcher(searcher),
	ss(ss),
	moves(moves),
	tt_move(tt_move)
{
	this->score_moves();
}

template<SearchType st>
MovePicker<st>::MovePicker(const Searcher& searcher, const Stack* ss, Movelist& moves, const Movelist& searchmoves,
						const bool root_node, const Move tt_move):
	played(0),
	searcher(searcher),
	ss(ss),
	moves(moves),
	tt_move(tt_move)
{
	if (root_node && searchmoves.size() > 0) this->moves = searchmoves;
	this->score_moves();
}

template<SearchType st>
int16_t MovePicker<st>::score_move(const Move move)
{
	if constexpr(st == QSEARCH) return winningCaptureBias + this->mvvlva(move);

	if (move == this->tt_move) return Biases::TTBias;

	if (this->searcher.Board.at(move.to()) != Piece::NONE)
		return ((see::see(this->searcher.Board, move, 0) ? winningCaptureBias : losingCaptureBias) + mvvlva(move));

	const bool isKiller = history::get<HistoryType::KILLER>(this->searcher, move, this->ss).Match(move);
	const bool isCounter = history::get<HistoryType::COUNTER>(this->searcher, move, this->ss) == move;

	if (isKiller) return Biases::killerBias;
	else if (isCounter) return Biases::counterBias;

	// if (move.typeOf() == Move::PROMOTION)
	// 	if (move.promotionType() == PieceType::QUEEN)
	// 		Score += Biases::promoteBias;
	return history::get<HistoryType::HISTORY>(this->searcher, move, this->ss);
}

template<SearchType st>
int16_t MovePicker<st>::mvvlva(const Move move) const
{
	int attacker = this->searcher.Board.at<PieceType>(move.from()) + 1;
	int victim = this->searcher.Board.at<PieceType>(move.to()) + 1;
	return mvvlvaArray[victim][attacker];
}

template<SearchType st>
Move MovePicker<st>::nextMove()
{
	if (this->played >= this->moves.size()) return Move::NO_MOVE;
	return this->moves[this->played++];
}

template<SearchType st>
void MovePicker<st>::score_moves()
{
	for (Move& move : this->moves)
		move.setScore(this->score_move(move));

	std::sort(this->moves.begin(), this->moves.end(), [](const Move& c1, const Move& c2) { return c1.score() > c2.score(); });
}

#endif // !_MOVE_PICKER_HPP_