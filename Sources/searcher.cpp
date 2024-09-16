#include <algorithm>
#include <limits>
#include <cmath>

#include "../Headers/see.hpp"
#include "../Headers/searcher.hpp"
#include "../Headers/movepick.hpp"
#include "../Headers/piecesbouns.hpp"
#include "../Headers/threadmanager.hpp"
#include "../Headers/transpositiontable.hpp"

extern TranspositionTable tt;
extern ThreadPool Threads;

[[nodiscard]] Score score_to_tt(Score, int);
[[nodiscard]] Score score_from_tt(Score, int);
[[nodiscard]] Score mateIn(int);
[[nodiscard]] Score matedIn(int);

int reductions[MAX_PLY][chess::constants::MAX_MOVES];

void init_reductions()
{
	reductions[0][0] = 0;

	for (Depth depth = 1; depth < MAX_PLY; depth++)
		for (int moves = 1; moves < chess::constants::MAX_MOVES; moves++)
			reductions[depth][moves] = 1 + log(depth) * log(moves) / 1.75;
}

Searcher::Searcher(const chess::Board& Board):
	Board(Board),
	nodes(0),
	seldepth(0),
	id(0),
	silent(false),
	use_tb(false)
{
	piecesbouns::init_pieces_bouns();
	init_reductions();
}

template <Searcher::NodeType node>
MoveValue Searcher::ABSearch(Depth depth, Score alpha, Score beta, const bool is_max, Stack* ss)
{
	Movelist moves;
	movegen::legalmoves(moves, this->Board);

	this->pv_length[ss->ply] = ss->ply;

	if (is_Draw(this->Board, moves.empty())) return { Move::NO_MOVE, 0 };
	if (is_checkmate(this->Board, moves.empty())) return { Move::NO_MOVE, -VALUE_MATE + ss->ply };

	const bool in_check = this->Board.inCheck();
	constexpr bool PvNode = node != NONPV;
	constexpr bool RootNode = node == ROOT;

	if (ss->ply > MAX_PLY - 1)
		return { Move::NO_MOVE, (!in_check? ((is_max ? 1 : -1) * Evaluate::evaluate(this->Board)) : 0) };

	if (in_check) depth++;
	if (depth <= 0) return this->QSearch<node>(alpha, beta, is_max, ss);

	assert(-VALUE_INFINITE <= alpha && alpha < beta && beta <= VALUE_INFINITE);
	assert(PvNode || (alpha == beta - 1));
	assert(0 < depth && depth < MAX_PLY);
	assert(ss->ply < MAX_PLY);

	(ss + 1)->excluded_move = Move::NO_MOVE;

	if (this->exit_early()) return { Move::NO_MOVE, 0 };
	if (PvNode && ss->ply > this->seldepth) this->seldepth = ss->ply;

	auto [tt_entry, ttMove, tt_hit] = tt.probe(this->Board.zobrist());
	const Score ttEval = tt_hit? score_from_tt(tt_entry->value, ss->ply) : Score(VALUE_NONE);
	const Move excluded_move = ss->excluded_move;

	// Start Looking up in Transposition Table
	if (
		!RootNode
		&& !PvNode
		&& excluded_move == Move::NO_MOVE
		&& tt_hit
		&& tt_entry->depth >= depth
		&& ttEval != VALUE_NONE
		&& (ss - 1)->currentmove != Move::NULL_MOVE
	) {
		if (tt_entry->nodeType == TTNodeType::EXACTBOUND)
			return { Move::NO_MOVE, ttEval };
		else if (tt_entry->nodeType == TTNodeType::LOWERBOUND)
			alpha = std::max(alpha, ttEval);
		else if (tt_entry->nodeType == TTNodeType::UPPERBOUND)
			beta = std::min(beta, ttEval);
		if (alpha >= beta) return { Move::NO_MOVE, ttEval };
	}
	// End Looking up in Transposition Table

	bool improving = false;
	if (in_check)
	{
		ss->eval = VALUE_NONE;
		goto startmoves;
	}
	ss->eval = tt_hit ? ttEval : (is_max ? 1 : -1) * Evaluate::evaluate(this->Board);

	// improving boolean
	improving = (ss - 2)->eval != VALUE_NONE && ss->eval > (ss - 2)->eval;

	if (RootNode) goto startmoves;

	// Start Internal Iterative Reductions (IIR)
	if (depth >= 3 && !tt_hit) depth--;
	if (PvNode && !tt_hit) depth--;
	if (depth <= 0) return this->QSearch<PV>(alpha, beta, is_max, ss);
	// End Internal Iterative Reductions (IIR)

	// Skip early pruning in Pv Nodes
	if (PvNode) goto startmoves;

	// Start Razoring
	if (depth < 3 && ss->eval + 129 < alpha)
		return this->QSearch<NONPV>(alpha, beta, is_max, ss);
	// End Razoring

	// Start Null Move Pruning
	if (
		NonPawnMaterial(this->Board, this->Board.sideToMove())
		&& depth >= 3
		&& excluded_move == Move::NO_MOVE
		&& (ss - 1)->currentmove != Move::NULL_MOVE
		&& ss->eval >= beta
	) {
		const Depth r = 5 + std::min(4, depth / 5) + std::min(3, (ss->eval - beta) / 214);
		(ss)->currentmove = Move::NULL_MOVE;

		this->Board.makeNullMove();
		Score null_score = -this->ABSearch<NONPV>(depth - r, -beta, -beta + 1, !is_max, ss + 1).second;
		this->Board.unmakeNullMove();

		if (null_score >= beta)
		{
			// dont return mate scores
			if (null_score >= VALUE_TB_WIN_IN_MAX_PLY) null_score = beta;

			return { Move::NO_MOVE, null_score };
		}
	}
	// End Null Move Pruning

startmoves:
	MovePicker<ABSEARCH> mp(*this, ss, moves, this->searchmoves, RootNode, tt_hit? ttMove : Move::NO_MOVE);
	ss->move_count = moves.size();

	Move Final_Move = moves.empty() ? Move::NO_MOVE : moves[0];
	Move quiets[64];
	Score Final_Score = -VALUE_INFINITE;
	Score score = VALUE_NONE;
	bool do_full_search = false;
	uint16_t quiet_count = 0;
	uint16_t madeMoves = 0;

	Move move = Move::NO_MOVE;
	while ((move = mp.nextMove()) != Move::NO_MOVE)
	{
		if (move == excluded_move) continue;

		const bool is_capture = this->Board.at(move.to()) != chess::Piece::NONE;
		madeMoves++;
		this->nodes++;
		int extension = 0;

		// Start Late Move Pruning/Movecount Pruning
		if (
			depth <= 3
			&& !PvNode
			&& !in_check
			&& madeMoves > lmpM[depth]
			&& !(this->Board.isAttacked(this->Board.kingSq(!this->Board.sideToMove()), this->Board.sideToMove()))
			&& !is_capture
			&& move.typeOf() != Move::PROMOTION
		) continue;
		// End Late Move Pruning/Movecount Pruning

		// Start Singular extensions
		if (
			!RootNode
			&& tt_hit
			&& excluded_move == Move::NO_MOVE
			&& ttMove == move
			&& std::abs(ttEval) < VALUE_TB_WIN_IN_MAX_PLY
			&& tt_entry->nodeType == TTNodeType::LOWERBOUND
			&& tt_entry->depth >= depth - 3
			&& depth >= 8
		) {
			Depth s_depth = (depth - 1) / 2;
			Score s_beta = ttEval - (54 + 76 * (!PvNode)) * depth / 64;

			ss->excluded_move = move;
			const Score s_value = this->ABSearch<NONPV>(s_depth, s_beta - 1, s_beta, !is_max, ss).second;
			ss->excluded_move = Move::NO_MOVE;

			if (s_value < s_beta) extension = 1;
			else if (s_beta >= beta) return { Move::NO_MOVE, s_beta };
		}
		// End Singular extensions

		// One reply extensions
		if (in_check && (ss - 1)->move_count == 1 && ss->move_count == 1) extension++;
		const int newDepth = depth - 1 + extension;

		if (this->id == 0 && RootNode && elapsed() > 10000 && !Threads.stop.load(std::memory_order_relaxed) && !this->silent)
			std::cout << "info depth " << depth << " currmove " << chess::uci::moveToUci(move) << " currmovenumber " << madeMoves << "\n";

		const uint64_t node_count = this->nodes;

		this->Board.makeMove(move);
		(ss)->currentmove = move;

		// Start late move reduction and PVS search
		// late move reduction
		if (depth >= 3 && !PvNode && !in_check && madeMoves > 3 + 2 * RootNode)
		{
			Depth rdepth = reductions[depth][madeMoves] - (PvNode + is_capture + (id % 2)) + improving;
			rdepth = std::clamp(newDepth - rdepth, 1, newDepth + 1);

			score = -this->ABSearch<NONPV>(rdepth, -alpha - 1, -alpha, !is_max, ss + 1).second;
			do_full_search = score > alpha && rdepth < newDepth;
		}
		else
			do_full_search = !PvNode || madeMoves > 1;

		// do a full research if lmr failed or lmr was skipped
		if (do_full_search)
			score = -this->ABSearch<NONPV>(newDepth, -alpha - 1, -alpha, !is_max, ss + 1).second;

		// PVS search
		if (PvNode && ((score > alpha && score < beta) || madeMoves == 1))
			score = -this->ABSearch<PV>(newDepth, -beta, -alpha, !is_max, ss + 1).second;
		// End late move reduction and PVS search

		this->Board.unmakeMove(move);

		assert(score > -VALUE_INFINITE && score < VALUE_INFINITE);

		if (this->id == 0) this->node_effort[move.from().index()][move.to().index()] += nodes - node_count;

		if (score > Final_Score)
		{
			Final_Score = score;

			if (score > alpha)
			{
				alpha = score;
				Final_Move = move;

				// Start Updating PV
				this->pv_table[ss->ply][ss->ply] = move;

				for (int next_ply = ss->ply + 1; next_ply < this->pv_length[ss->ply + 1]; next_ply++)
				{
					assert(ss->ply < MAX_PLY + 1);
					assert(next_ply < MAX_PLY + 1);
					assert(ss->ply + 1 < MAX_PLY + 1);
					this->pv_table[ss->ply][next_ply] = this->pv_table[ss->ply + 1][next_ply];
				}

				this->pv_length[ss->ply] = this->pv_length[ss->ply + 1];
				// End Updating PV

				if (score >= beta)
				{
					// Start Updating Killers, Counters and History
					if(!is_capture)
						history::update(*this, Final_Move, depth, quiets, quiet_count, ss);
					// End Updating Killers, Counters and History

					break;
				}
			}
		}
		if (!is_capture) quiets[quiet_count++] = move;
	}
	ss->move_count = madeMoves;

	// Start Trransposition Table
	const TTNodeType node_type =
		Final_Score >= beta ? TTNodeType::LOWERBOUND:
		(PvNode && Final_Move != Move::NO_MOVE ? TTNodeType::EXACTBOUND : TTNodeType::UPPERBOUND);

	if (excluded_move == Move::NO_MOVE && !Threads.stop.load(std::memory_order_relaxed))
		tt.store(this->Board.zobrist(), depth, score_to_tt(Final_Score, ss->ply), Final_Move, node_type);
	// End Trransposition Table

	assert(Final_Score > -VALUE_INFINITE && Final_Score < VALUE_INFINITE);

	return { Final_Move, Final_Score };
}

template <Searcher::NodeType node>
MoveValue Searcher::QSearch(Score alpha, Score beta, const bool is_max, Stack* ss)
{
	if (ss->ply > MAX_PLY - 1) return { Move::NO_MOVE, Evaluate::evaluate(this->Board) };
	if (this->exit_early()) return { Move::NO_MOVE, 0 };

	Movelist moves;
	movegen::legalmoves<MoveGenType::CAPTURE>(moves, this->Board);
	Move Final_Move = moves.empty() ? Move::NO_MOVE : moves[0];

	constexpr bool PvNode = node == PV;
	const bool in_check = this->Board.inCheck();

	assert(alpha >= -VALUE_INFINITE && alpha < beta && beta <= VALUE_INFINITE);
	assert(PvNode || (alpha == beta - 1));

	auto [tt_entry, ttMove, tt_hit] = tt.probe(this->Board.zobrist());
	const Score ttEval = tt_hit? score_from_tt(tt_entry->value, ss->ply) : Score(VALUE_NONE);

	// Start Transposition Table
	if (
		!PvNode
		&& tt_hit
		&& ttEval != VALUE_NONE
		&& tt_entry->nodeType != TTNodeType::NONEBOUND
	) {
		if (tt_entry->nodeType == TTNodeType::EXACTBOUND)
			return { Move::NO_MOVE, ttEval };
		else if (tt_entry->nodeType == TTNodeType::LOWERBOUND && ttEval >= beta)
			return { Move::NO_MOVE, ttEval };
		else if (tt_entry->nodeType == TTNodeType::UPPERBOUND && ttEval <= alpha)
			return { Move::NO_MOVE, ttEval };
	}
	// End Transposition Table

	Score Final_Score = Evaluate::evaluate(this->Board);
	if (Final_Score >= beta) return { Move::NO_MOVE, Final_Score };
	if (Final_Score > alpha) alpha = Final_Score;

	MovePicker<QSEARCH> mp(*this, ss, moves, ttMove);

	Move move = Move::NO_MOVE;
	while ((move = mp.nextMove()) != Move::NO_MOVE)
	{
		// Start Delta Pruning
		// if the move + a large margin is still less then alpha we can safely skip this
		const chess::PieceType captured_piece = this->Board.at(move.to()).type();
		if (Final_Score > VALUE_TB_LOSS_IN_MAX_PLY)
		{
			// delta pruning, if the move + a large margin is still less then alpha we can safely skip this
			if (
				captured_piece != chess::PieceType::NONE
				&& !in_check
				&& Final_Score + 400 + piecesbouns::PicesValues[int(piecesbouns::PHASE::EG)][captured_piece] < alpha
				&& move.typeOf() != Move::PROMOTION
				&& NonPawnMaterial(this->Board, this->Board.sideToMove())
				) continue;

			// see based capture pruning
			if (!in_check && !see::see(this->Board, move, 0)) continue;
		}
		// End Delta Pruning

		this->nodes++;

		this->Board.makeMove(move);
		Score score = -this->QSearch<node>(-beta, -alpha, !is_max, ss + 1).second;
		this->Board.unmakeMove(move);

		assert(score > -VALUE_INFINITE && score < VALUE_INFINITE);

		if (score > Final_Score)
		{
			Final_Score = score;

			if (score > alpha)
			{
				alpha = score;
				Final_Move = move;

				if (score >= beta) break;
			}
		}
	}

	// Start Trransposition Table
	const TTNodeType node_type = Final_Score >= beta ? TTNodeType::LOWERBOUND : TTNodeType::UPPERBOUND;

	if (!Threads.stop.load(std::memory_order_relaxed))
		tt.store(this->Board.zobrist(), 0, score_to_tt(Final_Score, ss->ply), Final_Move, node_type);
	// End Trransposition Table

	assert(Final_Score > -VALUE_INFINITE && Final_Score < VALUE_INFINITE);

	return { Final_Move, Final_Score };
}

MoveValue Searcher::ASearch(const Depth depth, const bool is_max, const MoveValue prev_eval, Stack* ss)
{
	Score alpha = -VALUE_INFINITE;
	Score beta = VALUE_INFINITE;
	Depth ply = 0;
	int delta = 30;
	MoveValue result;

	// Start search with full sized window
	if (depth == 1) result = this->ABSearch<ROOT>(1, -VALUE_INFINITE, VALUE_INFINITE, is_max,ss);
	else {
		// Use previous evaluation as a starting point and search with a smaller window
		alpha = prev_eval.second - 100;
		beta = prev_eval.second + 100;
		result = this->ABSearch<ROOT>(depth, alpha, beta, is_max, ss);
	}

	// In case the result is outside the bounds we have to do a research
	if (result.second <= alpha || result.second >= beta)
		return this->ABSearch<ROOT>(depth, -VALUE_INFINITE, VALUE_INFINITE, is_max, ss);

	return result;
}

void Searcher::IterativeDeepening()
{
	MoveValue final_result;

	Stack stack[MAX_PLY + 4], *ss = stack + 2;
	for (int i = 2; i > 0; --i)
	{
		(ss - i)->ply = i;
		(ss - i)->move_count = 0;
		(ss - i)->currentmove = Move::NO_MOVE;
		(ss - i)->eval = 0;
		(ss - i)->excluded_move = Move::NO_MOVE;
	}

	for (int i = 0; i <= MAX_PLY + 1; ++i)
	{
		(ss + i)->ply = i;
		(ss + i)->move_count = 0;
		(ss + i)->currentmove = Move::NO_MOVE;
		(ss + i)->eval = 0;
		(ss + i)->excluded_move = Move::NO_MOVE;
	}

	this->node_effort.reset();
	this->pv_length.reset();
	this->pv_table.reset();

	const auto t1 = this->StartTime = TimePoint::now();

	for (Depth depth = 1; depth <= this->limit.depth; depth++)
	{
		this->seldepth = 0;

		const MoveValue result = this->ASearch(depth, true, final_result, ss);
		if (this->exit_early()) break;

		final_result = result;

		if (this->id == 0 && !this->silent)
		{
			const int mate_score = is_mate_score(result.second);
			const std::string score_info = " score " + (mate_score ? "mate " + std::to_string(mate_score) : "cp " + std::to_string(result.second));
			const auto t2 = TimePoint::now();
			const int64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

			{
				std::stringstream ss;

				ss << "info depth " << signed(depth)
				<< " seldepth " << signed(this->seldepth)
				<< score_info
				<< " nodes " << Threads.getNodes()
				<< " nps " << signed((Threads.getNodes() / (ms + 1)) * 1000)
				<< " hashfull " << signed(tt.hashfull())
				<< " time " << ms
				<< " pv " << this->get_pv();

				std::cout << ss.str() << std::endl;
			}
		}
	}

	if (this->id == 0 && !this->silent)
	{
		// Start Little Check Moves
		if (final_result.first == Move::NO_MOVE || final_result.first == Move::NULL_MOVE)
		{
			chess::Movelist moves;
			chess::movegen::legalmoves(moves, this->Board);
			std::cout << "bestmove " << chess::uci::moveToUci(moves[0]) << std::endl;
			return;
		}
		// End Little Check Moves

		const std::string move = chess::uci::moveToUci(final_result.first);
		const int mate_score = is_mate_score(final_result.second);

		std::cout << "bestmove " << move << std::endl;
	}

	this->reset();
}

std::string Searcher::get_pv()
{
	std::stringstream ss;
	for (int i = 0; i < this->pv_length[0]; i++)
		ss << chess::uci::moveToUci(this->pv_table[0][i]) << ' ';

	return ss.str();
}

bool Searcher::exit_early()
{
	if (Threads.stop.load(std::memory_order_relaxed) || (this->limit.nodes != 0 && this->nodes >= this->limit.nodes))
		return true;

	if (this->limit.time != 0 && !this->limit.infinite)
	{
		auto t1 = TimePoint::now();
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - this->StartTime).count();
		if (ms >= this->limit.time)
		{
			Threads.stop = true;
			return true;
		}
	}
	return false;
}

void Searcher::StartThinking()
{
	tt.new_search();
	this->IterativeDeepening();
}

long long Searcher::elapsed()
{
	auto t1 = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(t1 - this->StartTime).count();
}

void Searcher::reset()
{
	this->nodes = 0;
	this->node_effort.reset();
	this->KillerMoves.reset();
	this->History.reset();
	this->Counters.reset();
}

// Transposition Table functions

Score score_to_tt(Score s, int ply)
{
	assert(s != VALUE_NONE);

	return (s >= VALUE_TB_WIN_IN_MAX_PLY    ? s + ply
			: s <= VALUE_TB_LOSS_IN_MAX_PLY ? s - ply
											: s);
}

Score score_from_tt(Score s, int ply)
{
	if (s == VALUE_NONE) return VALUE_NONE;

	return (s >= VALUE_TB_WIN_IN_MAX_PLY    ? s - ply
			: s <= VALUE_TB_LOSS_IN_MAX_PLY ? s + ply
											: s);
}

Score mateIn(int ply)
{
	return (VALUE_MATE - ply);
}

Score matedIn(int ply)
{
	return (ply - VALUE_MATE);
}

Killers::Killers():
	moveA(Move::NO_MOVE),
	moveB(Move::NO_MOVE)
{}

void Killers::Add(const Move move)
{
	if (move.score() != moveA.score())
	{
		moveB = moveA;
		moveA = move;
	}
}

const bool Killers::Match(const Move move) const
{
	return (move.score() == moveA.score() || move.score() == moveB.score());
}
