#pragma once
#ifndef _HISTORY_HPP_
#define _HISTORY_HPP_

#include "searcher.hpp"

enum HistoryType: int8_t
{
    KILLER,
    COUNTER,
    HISTORY
};

Move get_counter(const Searcher& searcher, const Stack* ss)
{
    return searcher.Counters[(ss - 1)->currentmove.from().index()][(ss - 1)->currentmove.to().index()];
}

Score get_history(const Searcher& searcher, Move move)
{
    return searcher.History[searcher.Board.sideToMove()][move.from().index()][move.to().index()];
}

Killers get_killer(const Searcher& searcher, const Stack* ss)
{
    return searcher.KillerMoves[ss->ply];
}

namespace history
{
    void update(Searcher& searcher, Move best, Depth depth, Move* quiets, uint16_t quiets_count, Stack* ss)
    {
        searcher.History[searcher.Board.sideToMove()][best.from().index()][best.to().index()] += depth;
        searcher.Counters[(ss - 1)->currentmove.from().index()][(ss - 1)->currentmove.to().index()] = best;
        searcher.KillerMoves[ss->ply].Add(best);
    }

    template<HistoryType ht>
    auto get(const Searcher& searcher, Move move, const Stack* ss)
    {
        if constexpr(ht == HistoryType::KILLER) return get_killer(searcher, ss);
        else if constexpr(ht == HistoryType::COUNTER) return get_counter(searcher, ss);
        else if constexpr(ht == HistoryType::HISTORY) return get_history(searcher, move);
    }
} // namespace history
#endif // !_HISTORY_HPP_