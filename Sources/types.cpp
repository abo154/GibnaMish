#include "../Headers/types.hpp"

bool is_checkmate(const chess::Board& Board, const bool is_moves_empty)
{
	const chess::Square white_king_sqare = Board.kingSq(chess::Color::WHITE);
	const chess::Square black_king_sqare = Board.kingSq(chess::Color::BLACK);
	const bool is_white_king_attacked = Board.isAttacked(white_king_sqare, chess::Color::BLACK);
	const bool is_black_king_attacked = Board.isAttacked(black_king_sqare, chess::Color::WHITE);
	const bool is_any_king_attacked = is_white_king_attacked || is_black_king_attacked;
	return (is_moves_empty && is_any_king_attacked);
}

bool is_Draw(const chess::Board& Board, const bool is_moves_empty)
{
	// SETUP
	const bool is_White_King_attaked = Board.isAttacked(Board.kingSq(chess::Color::WHITE), chess::Color::BLACK);
	const bool is_Black_King_attaked = Board.isAttacked(Board.kingSq(chess::Color::BLACK), chess::Color::WHITE);

	// DRAW
	return (
			(is_moves_empty && !is_White_King_attaked && !is_Black_King_attaked) || // STALEMATE
			Board.isRepetition(1) || // REPETITION
			Board.isInsufficientMaterial() || // INSUFFICIENT MATERIAL
			(Board.isHalfMoveDraw() && !is_White_King_attaked && !is_Black_King_attaked) // HALF MOVE DRAW
		);
}

int is_mate_score(const Score score)
{
	int result = 0;

	if (score > VALUE_MATE_IN_PLY) { result = VALUE_MATE - score; }
	else if (score < VALUE_MATED_IN_PLY) { result = -VALUE_MATE - score; }
	if (result % 2 != 0) { result++; }

	return result / 2;
}

bool NonPawnMaterial(const chess::Board& Board, const chess::Color color)
{
	const uint64_t Queens = Board.pieces(chess::PieceType::QUEEN, color).getBits();
	const uint64_t Rooks = Board.pieces(chess::PieceType::ROOK, color).getBits();
	const uint64_t Bishops = Board.pieces(chess::PieceType::BISHOP, color).getBits();
	const uint64_t Knights = Board.pieces(chess::PieceType::KNIGHT, color).getBits();
	const uint64_t NonPawnMaterials = Queens | Rooks | Bishops | Knights;
	return NonPawnMaterials;
}
