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
