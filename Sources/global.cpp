#include "../Headers/global.hpp"

bool global::is_checkmate(const chess::Board& Board, const size_t moves_size)
{
	const chess::Square white_king_sqare = Board.kingSq(chess::Color::WHITE);
	const chess::Square black_king_sqare = Board.kingSq(chess::Color::BLACK);
	const bool is_white_king_attacked = Board.isAttacked(white_king_sqare, chess::Color::BLACK);
	const bool is_black_king_attacked = Board.isAttacked(black_king_sqare, chess::Color::WHITE);
	const bool is_any_king_attacked = is_white_king_attacked || is_black_king_attacked;
	return (moves_size == 0 && is_any_king_attacked);
}

bool global::is_Draw(const chess::Board& Board, const size_t moves_size)
{
	//const Square White_King = Board.kingSq(chess::Color::WHITE);
	//const Square Black_King = Board.kingSq(chess::Color::BLACK);

	// SETUP
	const bool is_White_King_attaked = Board.isAttacked(Board.kingSq(chess::Color::WHITE), chess::Color::BLACK);
	const bool is_Black_King_attaked = Board.isAttacked(Board.kingSq(chess::Color::BLACK), chess::Color::WHITE);

	// DRAW
	return (
			(moves_size == 0 && !is_White_King_attaked && !is_Black_King_attaked) || // STALEMATE
			Board.isRepetition(1) || // REPETITION
			Board.isInsufficientMaterial() || // INSUFFICIENT MATERIAL
			(Board.isHalfMoveDraw() && !is_White_King_attaked && !is_Black_King_attaked) // HALF MOVE DRAW
		);
}

int global::is_mate_score(const int score)
{
	int result = 0;

	if (score > VALUE_MATE) { result = CHECKMATE - score; }
	else if (score < -VALUE_MATE) { result = -CHECKMATE - score; }
	if (result % 2 != 0) { result++; }

	return result / 2;
}

bool global::NonPawnMaterial(const chess::Board& Board, const chess::Color color)
{
	const uint64_t Queens = Board.pieces(chess::PieceType::QUEEN, color).getBits();
	const uint64_t Rooks = Board.pieces(chess::PieceType::ROOK, color).getBits();
	const uint64_t Bishops = Board.pieces(chess::PieceType::BISHOP, color).getBits();
	const uint64_t Knights = Board.pieces(chess::PieceType::KNIGHT, color).getBits();
	const uint64_t NonPawnMaterials = Queens | Rooks | Bishops | Knights;
	return NonPawnMaterials;
}
