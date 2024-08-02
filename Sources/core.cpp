#include <limits>

#include "../Headers/global.hpp"
#include "../Headers/core.hpp"

Core::Core():
	tt(16),
	searcher(tt)
{}

void Core::setMoves(const std::vector<std::string>& Moves)
{
	for (const std::string& move : Moves)
	{
		this->Board.makeMove(chess::uci::uciToMove(this->Board, move));
	}
}

void Core::reset_board()
{
	this->Board = chess::Board();
}

void Core::set_fen(std::string& FEN)
{
	this->Board.setFen(FEN);
}

void Core::get_score_move(const uint32_t DEPTH, const size_t wtime, const size_t winc, const size_t btime, const size_t binc, const bool is_inf)
{
	const size_t Time = (this->Board.sideToMove() == chess::Color::WHITE ? wtime : btime);
	const size_t inc = this->Board.sideToMove() == chess::Color::WHITE ? winc : binc;

	this->searcher.is_search_canceled = false;
	this->searcher.is_inf = is_inf;
	this->searcher.WaitTime = this->Timer.get_correct_time(Time, inc);
	this->searcher.StartTime = LONG_LONG_TIME_NOW_MS;

	const int REAL_DEPTH = DEPTH > 0 ? DEPTH : global::MAX_DEPTH;
	const MoveValue best_score = this->searcher.IterativeDeepening(this->Board, REAL_DEPTH);

	// Start Little Check Moves
	if (best_score.first == chess::Move::NO_MOVE || best_score.first == chess::Move::NULL_MOVE)
	{
		chess::Movelist moves;
		chess::movegen::legalmoves(moves, this->Board);
		std::cout << "bestmove " << chess::uci::moveToUci(moves[0]) << std::endl;
		return;
	}
	// End Little Check Moves

	const int& score = best_score.second;
	const std::string move = chess::uci::moveToUci(best_score.first);
	const int mate_score = global::is_mate_score(score);

	if (mate_score) { std::cout << "info score mate " << mate_score << std::endl; }
	else { std::cout << "info score cp " << score << std::endl; }

	std::cout << "bestmove " << move << std::endl;
}

void Core::Stop_Thinking()
{
	this->searcher.is_search_canceled = true;
}

void Core::draw() const
{
	system("cls");
	std::cout << this->Board;
}
