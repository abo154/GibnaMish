#pragma once
#ifndef _SEARCHER_HPP_
#define _SEARCHER_HPP_

#include <chrono>

#include "table.hpp"
#include "types.hpp"
#include "evaluate.hpp"
#include "transpositiontable.hpp"

#define CHESS_NO_EXCEPTIONS
#include "chesslib/chess.hpp"
#undef CHESS_NO_EXCEPTIONS

struct Stack
{
	Score eval;
	int move_count;
	chess::Move currentmove;
	chess::Move excluded_move;
	uint16_t ply;
};

struct Killers
{
	Move moveA;
	Move moveB;

	Killers();
	void Add(const Move);
	const bool Match(const Move) const;
};

class Searcher
{
	using Move = chess::Move;
	using Color = chess::Color;
	using Square = chess::Square;
	using movegen = chess::movegen;
	using size_type = Movelist::size_type;
	using MoveGenType = chess::movegen::MoveGenType;
	using TimePoint = std::chrono::high_resolution_clock;
public:
	Searcher(const chess::Board& = chess::Board());
	void IterativeDeepening();
	void StartThinking();
	void reset();

	TimePoint::time_point StartTime;
	Limits limit;
	uint64_t nodes;
	uint8_t id;
	Table<Move, Square::max(), Square::max()> Counters = {};
	Table<uint64_t, Square::max(), Square::max()> node_effort = {};
	Table<Score, 2, Square::max(), Square::max()> History = {};
	Table<Killers, MAX_PLY + 1> KillerMoves = {};
	chess::Board Board;
	Movelist searchmoves;
	bool silent;
	bool use_tb;
private:
	enum NodeType: uint8_t
	{
		NONPV,
		PV,
		ROOT
	};

	uint8_t seldepth;
	Table<uint8_t, MAX_PLY + 1> pv_length = {};
	Table<Move, MAX_PLY + 1, MAX_PLY + 1> pv_table = {};

	std::string get_pv();
	long long elapsed();
	bool exit_early();
	MoveValue ASearch(const Depth, const bool, const MoveValue, Stack* ss);
	template <NodeType> MoveValue ABSearch(Depth, Score, Score, const bool, Stack* ss);
	template <NodeType> MoveValue QSearch(Score, Score, const bool, Stack* ss);
};

#endif // !_SEARCHER_HPP_