#include <cmath>

#include "../Headers/transpositiontable.hpp"

TranspositionTable::TranspositionTable():
	currentAge(0)
{
	this->resize(16);
}

TTEntry::TTEntry():
	key(0),
	value(0),
	depth(0),
	nodeType(TTNodeType::NONEBOUND),
	move(Move::NO_MOVE),
	age(0)
{}

TTEntry::TTEntry(uint64_t key, Score value, Depth depth, TTNodeType nodeType, Move move, Age age):
	key(key),
	value(value),
	depth(depth),
	nodeType(nodeType),
	move(move),
	age(age)
{}

TTData TranspositionTable::probe(uint64_t hash)
{
	const TTEntry& entry = this->entries[hash % this->count];
	const bool tt_hit = entry.key == hash;
	const Move ttMove = entry.move;
	return { &entry, ttMove, tt_hit };
}

void TranspositionTable::store(uint64_t hash, Depth depth, Score best_score, Move best_move, TTNodeType node_type)
{
	const uint64_t index = hash % count;
	// const TTEntry& old_entry = this->entries[index];

	// if (old_entry.age < this->currentAge || old_entry.depth < depth)
		this->entries[index] = TTEntry(hash, best_score, depth, node_type, best_move, this->currentAge);
}

void TranspositionTable::reset()
{
	this->currentAge = 0;
	std::fill(this->entries.begin(), this->entries.end(), TTEntry());
}

void TranspositionTable::new_search()
{
	this->currentAge++;
}

void TranspositionTable::resize(uint64_t sizeMB)
{
	int desiredTableSizeInBytes = (sizeMB * 1024 * 1024);
	uint64_t numEntries = desiredTableSizeInBytes / sizeof(TTEntry);

	this->count = numEntries;
	this->entries.resize(numEntries);
}

uint32_t TranspositionTable::hashfull() const
{
	uint64_t size = 0;
	for (const TTEntry& entry: this->entries)
	{
		if(!(
			entry.key == 0
			&& entry.depth == 0
			&& entry.value == 0
			&& entry.nodeType == TTNodeType::NONEBOUND
			&& entry.move == Move::NO_MOVE
		)) size++;
	}
	return ((size * 1000) / this->count);
}
