#pragma once
#ifndef _MOVE_ORDERING_HPP_
#define _MOVE_ORDERING_HPP_

#include <array>
#include <algorithm>

#include "chesslib/chess.hpp"
#include "transpositiontable.hpp"

class MoveOrdering
{
	using Move = chess::Move;
	using Color = chess::Color;
	using Piece = chess::Piece;
	using Square = chess::Square;
	using Movelist = chess::Movelist;
	using PieceType = chess::PieceType;
	using TT_NodeType = TranspositionTable::NodeType;
public:
	struct Killers
	{
		Move moveA;
		Move moveB;

		Killers();
		inline void Add(const Move);
		inline const bool Match(const Move) const;
	};

	MoveOrdering(TranspositionTable&);
	void Order(const chess::Board&, chess::Movelist&, const bool, const uint32_t);
	inline void clear_history();
	inline void clear_killer_moves();
	inline void clear();
	std::array<Killers, global::MAX_PLY + 1> KillerMoves;
	std::array<std::array<std::array<int, 64>, 64>, 2> History;
	~MoveOrdering();
private:
	enum class Biases : uint16_t
	{
		STANDARD = 10,
		regularBias = 0 * STANDARD,
		losingCaptureBias = 1 * STANDARD,
		killerBias = 2 * STANDARD,
		promoteBias = 3 * STANDARD,
		winningCaptureBias = 4 * STANDARD,
		hashMoveScore = 50 * STANDARD
	};
	const uint32_t maxKillerMovePly;
	TranspositionTable& tt;
};

inline void MoveOrdering::Killers::Add(const Move move)
{
	if (move.score() != moveA.score())
	{
		moveB = moveA;
		moveA = move;
	}
}

inline const bool MoveOrdering::Killers::Match(const Move move) const
{
	return (move.score() == moveA.score() || move.score() == moveB.score());
}

inline void MoveOrdering::clear_history()
{
	for (std::array<std::array<int, 64>, 64>& arr1 : this->History) { for (std::array<int, 64>& arr2 : arr1) { arr2.fill({}); } }
}

inline void MoveOrdering::clear_killer_moves()
{
	this->KillerMoves.fill({});
}

inline void MoveOrdering::clear()
{
	this->clear_history();
	this->clear_killer_moves();
}

#endif // !_MOVE_ORDERING_HPP_