#include <cmath>

#include "../Headers/global.hpp"
#include "../Headers/transpositiontable.hpp"

TranspositionTable::TranspositionTable(int sizeMB):
	enabled(true)
{
	int ttEntrySizeBytes = sizeof(Entry);
	int desiredTableSizeInBytes = sizeMB * 1024 * 1024;
	uint64_t numEntries = desiredTableSizeInBytes / ttEntrySizeBytes;

	this->count = numEntries;
	this->entries.resize(numEntries);
}

TranspositionTable::Entry::Entry():
	key(0),
	value(0),
	depth(0),
	nodeType(0),
	move(Move::NO_MOVE)
{}

TranspositionTable::Entry::Entry(uint64_t key, int value, uint32_t depth, uint8_t nodeType, Move move):
	key(key),
	value(value),
	depth(depth),
	nodeType(nodeType),
	move(move)
{}

void TranspositionTable::StoreEvaluation(uint64_t zobrist, int depth, int numPlySearched, int eval, int evalType, Move move)
{
	if (!enabled) { return; }
	uint64_t index = zobrist % count;
	Entry entry(zobrist, CorrectMateScoreForStorage(eval, numPlySearched), depth, evalType, move);
	entries[index] = entry;
}

int TranspositionTable::LookupEvaluation(uint64_t zobrist, int depth, int plyFromRoot, int alpha, int beta)
{
	if (!enabled) { return int(NodeType::LookupFailed); }
	const Entry& entry = entries[zobrist % count];

	if (entry.key == zobrist)
	{
		if (entry.depth >= depth)
		{
			int correctedScore = CorrectRetrievedMateScore(entry.value, plyFromRoot);
			if (entry.nodeType == int(NodeType::Exact)) { return correctedScore; }
			if (entry.nodeType == int(NodeType::UpperBound) && correctedScore <= alpha) { return correctedScore; }
			if (entry.nodeType == int(NodeType::LowerBound) && correctedScore >= beta) { return correctedScore; }
		}
	}
	return int(NodeType::LookupFailed);
}

void TranspositionTable::clear()
{
	this->entries.clear();
}

void TranspositionTable::resize(uint64_t sizeMB)
{
	//this->enabled = true;
	int ttEntrySizeBytes = sizeof(Entry);
	int desiredTableSizeInBytes = sizeMB * 1024 * 1024;
	uint64_t numEntries = desiredTableSizeInBytes / ttEntrySizeBytes;

	this->count = numEntries;
	this->entries.resize(numEntries);
}

TranspositionTable::Move TranspositionTable::TryGetStoredMove(uint64_t zobrist) const
{
	return entries[zobrist % count].move;
}

bool TranspositionTable::TryLookupEvaluation(int depth, int plyFromRoot, int alpha, int beta, int& eval)
{
	eval = 0;
	return false;
}

const std::vector<TranspositionTable::Entry>& TranspositionTable::get_entries() const
{
	return this->entries;
}

const TranspositionTable::Entry& TranspositionTable::operator[](size_t Index) const
{
	return this->entries[Index];
}

uint64_t TranspositionTable::get_count() const
{
	return this->count;
}

int TranspositionTable::CorrectMateScoreForStorage(int score, int numPlySearched)
{
	if (global::is_mate_score(score))
	{
		int sign = std::signbit(float(score)) ? -1 : 1;
		return (score * sign + numPlySearched) * sign;
	}
	return score;
}

int TranspositionTable::CorrectRetrievedMateScore(int score, int numPlySearched)
{
	if (global::is_mate_score(score))
	{
		int sign = std::signbit(float(score)) ? -1 : 1;
		return (score * sign - numPlySearched) * sign;
	}
	return score;
}

uint32_t TranspositionTable::hashfull() const
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
