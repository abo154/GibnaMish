#pragma once
#ifndef _MOVE_ORDERING_HPP_
#define _MOVE_ORDERING_HPP_

#include <array>

#include "global.hpp"
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
		void Add(const Move);
		const bool Match(const Move) const;
	};

	MoveOrdering(TranspositionTable&);
	void Order(const chess::Board&, Movelist&, const bool, const uint32_t);
	void clear_history();
	void clear_killer_moves();
	void clear();
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

#endif // !_MOVE_ORDERING_HPP_