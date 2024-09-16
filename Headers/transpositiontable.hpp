#pragma once
#ifndef _TRANSPOSITION_TABLE_HPP_
#define _TRANSPOSITION_TABLE_HPP_

#include <vector>

#include "chesslib/chess.hpp"
#include "types.hpp"

using Age = uint32_t;

enum class TTNodeType: uint8_t
{
	NONEBOUND,
	UPPERBOUND,
	LOWERBOUND,
	EXACTBOUND,
};

struct TTEntry
{
	uint64_t key;
	Score value;
	Move move;
	Depth depth;
	TTNodeType nodeType;
	Age age;

	TTEntry();
	TTEntry(uint64_t, Score, Depth, TTNodeType, Move, Age);
};

struct TTData
{
	const TTEntry* tt_entry;
	Move ttMove;
	bool tt_hit;
};

class TranspositionTable
{
	using Board = chess::Board;
	using Move = chess::Move;
public:
	TranspositionTable();

	void reset();
	void resize(uint64_t);
	TTData probe(uint64_t);
	void new_search();
	void store(uint64_t, Depth, Score, Move, TTNodeType);
	uint32_t hashfull() const;

	static constexpr uint64_t MAXHASH_MiB = (1ull << 32) * sizeof(TTEntry) / (1024 * 1024);
private:
	uint64_t count;
	Age currentAge;
	std::vector<TTEntry> entries;
};

#endif // !_TRANSPOSITION_TABLE_HPP_