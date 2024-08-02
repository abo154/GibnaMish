#pragma once
#ifndef _SEARCHER_HPP_
#define _SEARCHER_HPP_

#include <chrono>

#include "global.hpp"
#include "evaluate.hpp"
#include "moveordering.hpp"
#include "chesslib/chess.hpp"
#include "transpositiontable.hpp"

#define LONG_LONG_TIME_NOW_MS std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count()
inline constexpr int INFINITE = 2147483645;

class Searcher
{
	using Move = chess::Move;
	using Color = chess::Color;
	using Square = chess::Square;
	using Moves = chess::Movelist;
	using movegen = chess::movegen;
	using MoveValue = std::pair<Move, int>;
	using size_type = chess::Movelist::size_type;
	using MoveGenType = chess::movegen::MoveGenType;
	using TT_NodeType = TranspositionTable::NodeType;
	using time_point = std::chrono::system_clock::time_point;
public:
	Searcher(TranspositionTable&);
	MoveValue IterativeDeepening(chess::Board&, const uint32_t);
	void clear();

	time_t WaitTime;
	time_t StartTime;
	bool is_inf;
	bool is_search_finished;
	bool is_search_canceled;
private:
	enum class NodeType : uint8_t
	{
		NonPV,
		PV,
		Root
	};

	Evaluate evaluation;
	MoveOrdering OrderingMove;
	TranspositionTable& tt;
	uint8_t seldepth;
	uint64_t nodes;
	uint8_t pv_length[global::MAX_PLY];
	Move pv_table[global::MAX_PLY][global::MAX_PLY];

	std::string get_pv();
	bool exit_early();
	MoveValue ASearch(chess::Board&, const uint32_t, const bool, const MoveValue);
	MoveValue Search(chess::Board&, const uint32_t, int, int, int, const bool, const uint32_t, const bool);
	MoveValue QSearch(chess::Board&, const uint32_t, int, int, const bool, const uint32_t);
};

#endif // !_SEARCHER_HPP_