#pragma once
#ifndef _TRANSPOSITION_TABLE_HPP_
#define _TRANSPOSITION_TABLE_HPP_

#include <vector>

#include "chesslib/chess.hpp"

class TranspositionTable
{
	using Board = chess::Board;
	using Move = chess::Move;
public:
	enum class NodeType : int8_t
	{
		LookupFailed = -1,
		Exact = 0,
		LowerBound = 1,
		UpperBound = 2
	};
	struct Entry
	{
		uint64_t key;
		int value;
		Move move;
		uint32_t depth;
		uint8_t nodeType;

		Entry();
		Entry(uint64_t, int, uint32_t, uint8_t, Move);
	};

	TranspositionTable(int);

	void clear();
	void resize(uint64_t);
	Move TryGetStoredMove(uint64_t) const;
	bool TryLookupEvaluation(int, int, int, int, int&);
	const std::vector<Entry>& get_entries() const;
	const Entry& operator[](size_t) const;
	uint64_t get_count() const;
	uint32_t hashfull() const;

	int LookupEvaluation(uint64_t, int, int, int, int);
	void StoreEvaluation(uint64_t, int, int, int, int, Move);

private:
	bool enabled;
	uint64_t count;
	std::vector<Entry> entries;

	int CorrectMateScoreForStorage(int, int);
	int CorrectRetrievedMateScore(int, int);
};

#endif // !_TRANSPOSITION_TABLE_HPP_