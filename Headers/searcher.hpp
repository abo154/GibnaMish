#pragma once
#ifndef _SEARCHER_HPP_
#define _SEARCHER_HPP_

#include <limits>
#include <chrono>

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
	Searcher();
	MoveValue IterativeDeepening(chess::Board&, const uint32_t);
	inline void init_NNUE(const char*);
	inline bool is_avilable() const;
	inline void clear();

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
	TranspositionTable tt;
	uint8_t seldepth;
	uint64_t nodes;
	uint8_t pv_length[global::MAX_PLY];
	Move pv_table[global::MAX_PLY][global::MAX_PLY];

	inline const std::string get_pv();
	inline const bool exit_early();
	MoveValue ASearch(chess::Board&, const uint32_t, const bool, const MoveValue);
	MoveValue Search(chess::Board&, const uint32_t, int, int, int, const bool, const uint32_t, const bool);
	MoveValue QSearch(chess::Board&, const uint32_t, int, int, const bool, const uint32_t);
};

inline void Searcher::init_NNUE(const char* evalFile)
{
	this->evaluation.init_NNUE(evalFile);
}

inline const std::string Searcher::get_pv()
{
	std::string line = "";
	for (int i = 0; i < this->pv_length[0]; i++)
	{
		line += chess::uci::moveToUci(this->pv_table[0][i]);
		line += " ";
	}
	return line;
}

inline const bool Searcher::exit_early()
{
	if (this->is_search_canceled) { return true; }
	if (this->nodes & 2047 && this->WaitTime != 0)
	{
		auto t1 = std::chrono::high_resolution_clock::now();
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t1).count();
		if (ms >= this->WaitTime)
		{
			this->is_search_canceled = true;
			return true;
		}
	}
	return false;
}

inline bool Searcher::is_avilable() const
{
	return this->evaluation.is_avilable();
}

inline void Searcher::clear()
{
	this->OrderingMove.clear();
	this->tt.clear();
}

#endif // !_SEARCHER_HPP_