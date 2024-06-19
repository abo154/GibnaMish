#include "../Headers/global.hpp"
#include "../Headers/searcher.hpp"
#include "../Headers/piecesbouns.hpp"

Searcher::Searcher():
	nodes(0),
	seldepth(0),
	StartTime(0),
	WaitTime(0),
	is_inf(true),
	is_search_finished(true),
	is_search_canceled(false),
	tt(16),
	OrderingMove(tt)
{}

Searcher::MoveValue Searcher::Search(chess::Board& Board, const uint32_t depth, int alpha, int beta, int num_extent, const bool is_max, const uint32_t PlyD, const bool null)
{
	Moves moves;
	movegen::legalmoves(moves, Board);

	this->pv_length[PlyD] = PlyD;

	if (global::is_Draw(Board, moves.size())) { return { Move::NO_MOVE, 0 }; }
	if (global::is_checkmate(Board, moves.size())) { return { Move::NO_MOVE, -global::CHECKMATE + PlyD }; }
	if (PlyD > global::MAX_PLY - 1) { return { Move::NO_MOVE, (is_max ? 1 : -1) * evaluation.evaluate(Board) }; }
	if (depth <= 0) { return this->QSearch(Board, 14, alpha, beta, is_max, PlyD); }
	if (((LONG_LONG_TIME_NOW_MS - this->StartTime > this->WaitTime) && !is_inf) || this->is_search_canceled) { this->is_search_finished = false; return { Move::NO_MOVE, 0 }; }
	if (PlyD > this->seldepth) { this->seldepth = PlyD; }

	// Start Transposition Table
	int ttEval = tt.LookupEvaluation(Board.zobrist(), depth, PlyD, alpha, beta);
	if (ttEval != int(TT_NodeType::LookupFailed))
	{
		if (PlyD == 0)
		{
			const uint64_t hash_key = Board.zobrist();
			const int best_score = this->tt[hash_key % tt.get_count()].value;
			const Move best_move = tt.TryGetStoredMove(hash_key);

			return { best_move, best_score };
		}
		return { Move::NO_MOVE, ttEval };
	}
	// End Transposition Table

	const bool in_check = Board.inCheck();
	const bool PvNode = beta - alpha > 1;
	const bool RootNode = PlyD == 0;

	// Start Null Move Pruning
	if (global::NonPawnMaterial(Board, Board.sideToMove()) && depth >= 3 && null && !in_check)
	{
		const int r = depth > 6 ? 3 : 2;
		Board.makeNullMove();
		int null_score = -this->Search(Board, depth - 1 - r, -beta, -beta + 1, num_extent, !is_max, PlyD + 1, false).second;
		Board.unmakeNullMove();
		if (null_score >= beta) { return { chess::Move::NO_MOVE, beta }; }
	}
	// End Null Move Pruning

	this->OrderingMove.Order(Board, moves, false, PlyD);

	Move Final_Move = moves.empty() ? Move::NO_MOVE : moves[0];
	int Final_Score = -INFINITE;
	bool do_full_search = true;
	uint16_t madeMoves = 0;

	for (size_type i = 0; i < moves.size(); i++)
	{
		const Move& move = moves[i];
		const bool is_capture = Board.isCapture(move);
		madeMoves++;
		this->nodes++;

		Board.makeMove(move);

		// Start Late Move Pruning/Movecount Pruning
		if (
			depth <= 3
			&& !PvNode
			&& !in_check
			&& madeMoves > global::lmpM[depth]
			&& !(Board.isAttacked(Board.kingSq(!Board.sideToMove()), Board.sideToMove()))
			&& !is_capture
			&& move != Move::PROMOTION
		)
		{
			Board.unmakeMove(move);
			continue;
		}
		// End Late Move Pruning/Movecount Pruning

		int score = -this->Search(Board, depth - 1, -beta, -alpha, num_extent, !is_max, PlyD + 1, true).second;
		Board.unmakeMove(move);

		if (score > Final_Score)
		{
			Final_Score = score;
			Final_Move = move;

			// Start Updating PV
			this->pv_table[PlyD][PlyD] = move;
			for (int next_ply = PlyD + 1; next_ply < this->pv_length[PlyD + 1]; next_ply++) { this->pv_table[PlyD][next_ply] = pv_table[PlyD + 1][next_ply]; }
			pv_length[PlyD] = pv_length[PlyD + 1];
			// End Updating PV

			if (score > alpha)
			{
				alpha = score;

				// Start Updating History Table
				if (!is_capture) { this->OrderingMove.History[Board.sideToMove()][move.from().index()][move.to().index()] += depth; }
				// End Updating History Table

				if (score >= beta)
				{
					// Start Updating Killer Moves
					if(!is_capture) { this->OrderingMove.KillerMoves[PlyD].Add(move); }
					// End Updating Killer Moves
					return { move, score };
				}
			}
		}
	}

	// Start Trransposition Table
	TT_NodeType node_type = TT_NodeType::Exact;

	if (Final_Score <= alpha) { node_type = TT_NodeType::UpperBound; }
	else if (Final_Score >= beta) { node_type = TT_NodeType::LowerBound; }

	tt.StoreEvaluation(Board.zobrist(), depth, PlyD, Final_Score, int(node_type), Final_Move);
	// End Trransposition Table

	return { Final_Move, Final_Score };
}

Searcher::MoveValue Searcher::QSearch(chess::Board& Board, const uint32_t depth, int alpha, int beta, const bool is_max, const uint32_t PlyD)
{
	Moves moves;
	movegen::legalmoves<MoveGenType::CAPTURE>(moves, Board);
	const int evaluate = evaluation.evaluate(Board);
	Move Final_Move = moves.empty() ? Move::NO_MOVE : moves[0];

	if (PlyD > global::MAX_PLY - 1) { return { Move::NO_MOVE, evaluate }; }
	if (((LONG_LONG_TIME_NOW_MS - this->StartTime > this->WaitTime) && !is_inf) || this->is_search_canceled) { this->is_search_finished = false; return { Move::NO_MOVE, 0 }; }
	if (evaluate >= beta) { return { Move::NO_MOVE, beta }; }
	if (evaluate > alpha) { alpha = evaluate; }
	if (depth <= 0) { return { Move::NO_MOVE, alpha }; }

	this->OrderingMove.Order(Board, moves, true, PlyD);

	for (size_type i = 0; i < moves.size(); i++)
	{
		const Move& move = moves[i];

		// Start Delta Pruning
		// if the move + a large margin is still less then alpha we can safely skip this
		const chess::PieceType captured_piece = Board.at(move.to()).type();
		if (evaluate + 400 + piecesbouns::PicesValues[int(piecesbouns::PHASE::EG)][captured_piece] < alpha && move != Move::PROMOTION && global::NonPawnMaterial(Board, Board.sideToMove())) { continue; }
		// End Delta Pruning

		this->nodes++;

		Board.makeMove(move);
		int score = -this->QSearch(Board, depth - 1, -beta, -alpha, !is_max, PlyD + 1).second;
		Board.unmakeMove(move);

		if (score >= beta) { return { move, beta }; }
		if (score > alpha) { alpha = score; Final_Move = move; }
	}

	return { Final_Move, alpha };
}

Searcher::MoveValue Searcher::ASearch(chess::Board& Board, const uint32_t depth, const bool is_max, const MoveValue prev_eval)
{
	int alpha = -INFINITE;
	int beta = INFINITE;
	MoveValue result;
	int PlyD = 0;
	// Start search with full sized window
	if (depth == 1) { result = this->Search(Board, 1, -INFINITE, INFINITE, 0, is_max, PlyD, true); }
	else {
		// Use previous evaluation as a starting point and search with a smaller window
		alpha = prev_eval.second - 100;
		beta = prev_eval.second + 100;
		result = this->Search(Board, depth, alpha, beta, 0, is_max, PlyD, true);
	}
	// In case the result is outside the bounds we have to do a research
	if (result.second <= alpha || result.second >= beta) { return this->Search(Board, depth, -INFINITE, INFINITE, 0, is_max, PlyD, true); }
	return result;
}

Searcher::MoveValue Searcher::IterativeDeepening(chess::Board& Board, const uint32_t DEPTH)
{
	MoveValue final_result;
	this->seldepth = 0;
	this->nodes = 0;
	const time_point t1 = std::chrono::high_resolution_clock::now();

	for (uint32_t depth = 1; depth <= DEPTH; depth++)
	{
		const MoveValue result = this->ASearch(Board, depth, true, final_result);
		if (!this->is_search_finished || this->is_search_canceled) { this->is_search_canceled = false; this->is_search_finished = true; break; }
		final_result = result;

		const int& score = result.second;
		const int mate_score = global::is_mate_score(score);
		const std::string score_info = " score " + (mate_score ? "mate " + std::to_string(mate_score) : "cp " + std::to_string(score));
		const time_point t2 = std::chrono::high_resolution_clock::now();
		const int64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

		std::cout << "info depth " << signed(depth)
		<< " seldepth " << signed(this->seldepth)
		<< score_info
		<< " nodes " << this->nodes
		<< " nps " << signed((this->nodes / (ms + 1)) * 1000)
		<< " hashfull " << signed(this->tt.hashfull())
		<< " time " << ms
		<< " pv " << this->get_pv() << std::endl;
	}

	this->clear();
	return final_result;
}
