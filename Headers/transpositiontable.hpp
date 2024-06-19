#pragma once
#ifndef _TRANSPOSITION_TABLE_HPP_
#define _TRANSPOSITION_TABLE_HPP_

#include <vector>
#include <cmath>

#include "global.hpp"
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

	inline void clear();
	inline void resize(uint64_t);
	inline Move TryGetStoredMove(uint64_t) const;
	inline bool TryLookupEvaluation(int, int, int, int, int&);
	inline const std::vector<Entry>& get_entries() const;
	inline const Entry& operator[](size_t) const;
	inline const uint64_t get_count() const;
	inline uint32_t hashfull() const;

	int LookupEvaluation(uint64_t, int, int, int, int);
	void StoreEvaluation(uint64_t, int, int, int, int, Move);

private:
	bool enabled;
	uint64_t count;
	std::vector<Entry> entries;

	inline int CorrectMateScoreForStorage(int, int);
	inline int CorrectRetrievedMateScore(int, int);
};

inline void TranspositionTable::clear()
{
	this->entries.clear();
}

inline void TranspositionTable::resize(uint64_t sizeMB)
{
	//this->enabled = true;
	int ttEntrySizeBytes = sizeof(Entry);
	int desiredTableSizeInBytes = sizeMB * 1024 * 1024;
	uint64_t numEntries = desiredTableSizeInBytes / ttEntrySizeBytes;

	this->count = numEntries;
	this->entries.resize(numEntries);
}

inline TranspositionTable::Move TranspositionTable::TryGetStoredMove(uint64_t zobrist) const
{
	return entries[zobrist % count].move;
}

inline bool TranspositionTable::TryLookupEvaluation(int depth, int plyFromRoot, int alpha, int beta, int& eval)
{
	eval = 0;
	return false;
}

inline const std::vector<TranspositionTable::Entry>& TranspositionTable::get_entries() const
{
	return this->entries;
}

inline const TranspositionTable::Entry& TranspositionTable::operator[](size_t Index) const
{
	return this->entries[Index];
}

inline const uint64_t TranspositionTable::get_count() const
{
	return this->count;
}

inline int TranspositionTable::CorrectMateScoreForStorage(int score, int numPlySearched)
{
	if (global::is_mate_score(score))
	{
		int sign = std::signbit(float(score)) ? -1 : 1;
		return (score * sign + numPlySearched) * sign;
	}
	return score;
}

inline int TranspositionTable::CorrectRetrievedMateScore(int score, int numPlySearched)
{
	if (global::is_mate_score(score))
	{
		int sign = std::signbit(float(score)) ? -1 : 1;
		return (score * sign - numPlySearched) * sign;
	}
	return score;
}

inline uint32_t TranspositionTable::hashfull() const
{
	uint64_t size = 0;
	for (const Entry& entry: this->entries)
	{
		const bool is_not_used = (
			entry.key == 0
			&& entry.depth == 0
			&& entry.value == 0
			&& entry.nodeType == 0
			&& entry.move == Move::NO_MOVE
		);
		if (!is_not_used) { size++; }
	}
	return ((size * 1000) / this->count);
}

#endif // !_TRANSPOSITION_TABLE_HPP_