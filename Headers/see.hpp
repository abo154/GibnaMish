#pragma once
#ifndef _SEE_HPP_
#define _SEE_HPP_

#define CHESS_NO_EXCEPTIONS
#include "chesslib/chess.hpp"
#undef CHESS_NO_EXCEPTIONS

#include "piecesbouns.hpp"

namespace see
{
	using Board = chess::Board;
	using Bitboard = chess::Bitboard;
	using PieceType = chess::PieceType;
	using Square = chess::Square;
	using Color = chess::Color;
	using attacks = chess::attacks;
	using Move = chess::Move;
	using Piece = chess::Piece;

	[[nodiscard]] inline Bitboard attackersForSide(const Board& board, Color attacker_color, Square sq, Bitboard occupied_bb)
	{
		const Bitboard attacking_bishops = board.pieces(PieceType::BISHOP, attacker_color);
		const Bitboard attacking_rooks = board.pieces(PieceType::ROOK, attacker_color);
		const Bitboard attacking_queens = board.pieces(PieceType::QUEEN, attacker_color);
		const Bitboard attacking_knights = board.pieces(PieceType::KNIGHT, attacker_color);
		const Bitboard attacking_king = board.pieces(PieceType::KING, attacker_color);
		const Bitboard attacking_pawns = board.pieces(PieceType::PAWN, attacker_color);

		const Bitboard inter_cardinal_rays = attacks::bishop(sq, occupied_bb);
		const Bitboard cardinal_rays_rays = attacks::rook(sq, occupied_bb);

		Bitboard attackers = inter_cardinal_rays & (attacking_bishops | attacking_queens);
		attackers |= cardinal_rays_rays & (attacking_rooks | attacking_queens);
		attackers |= attacks::knight(sq) & attacking_knights;
		attackers |= attacks::king(sq) & attacking_king;
		attackers |= attacks::pawn(~attacker_color, sq) & attacking_pawns;
		return attackers;
	}

	[[nodiscard]] inline Bitboard allAttackers(const Board& board, Square sq, Bitboard occupied_bb)
	{
		return attackersForSide(board, Color::WHITE, sq, occupied_bb) |
			attackersForSide(board, Color::BLACK, sq, occupied_bb);
	}

	// Static Exchange Evaluation (SEE), logical based on Weiss
	// https://github.com/TerjeKir/weiss/blob/6772250059e82310c913a0d77a4c94b05d1e18e6/src/board.c#L310
	// licensed under GPL-3.0

	[[nodiscard]] inline bool see(const Board& board, Move move, int threshold)
	{
		Square to_sq = move.to();
		PieceType victim = board.at<PieceType>(to_sq);
		int swap = piecesbouns::PicesValuesClassical[victim] - threshold;
		if (swap < 0) return false;

		Square from_sq = move.from();
		PieceType attacker = board.at<PieceType>(from_sq);
		swap -= piecesbouns::PicesValuesClassical[attacker];
		if (swap >= 0) return true;

		Bitboard occ = (board.all() ^ (1ULL << from_sq.index())) | (1ULL << to_sq.index());
		Bitboard attackers = allAttackers(board, to_sq, occ) & occ;

		Bitboard queens = board.pieces(PieceType::QUEEN, Color::WHITE) | board.pieces(PieceType::QUEEN, Color::BLACK);

		Bitboard bishops = board.pieces(PieceType::BISHOP, Color::WHITE) | board.pieces(PieceType::BISHOP, Color::BLACK) | queens;
		Bitboard rooks = board.pieces(PieceType::ROOK, Color::WHITE) | board.pieces(PieceType::ROOK, Color::BLACK) | queens;

		Color sT = ~(from_sq.is_light() ? Color::WHITE : Color::BLACK);

		while (true)
		{
			attackers &= occ;
			Bitboard my_attackers = attackers & board.us(sT);
			if (!my_attackers) break;

			int pt;
			for (pt = 0; pt <= 5; pt++) {
				if (my_attackers &
					(
						board.pieces(static_cast<Piece>(Piece::underlying(pt)).type(), static_cast<Piece>(Piece::underlying(pt)).color()) |
						board.pieces(static_cast<Piece>(Piece::underlying(pt + 6)).type(), static_cast<Piece>(Piece::underlying(pt + 6)).color())
						))
					break;
			}
			sT = ~sT;
			if ((swap = -swap - 1 - piecesbouns::PicesValues[0][pt]) >= 0)
			{
				if (pt == chess::KING && (attackers & board.us(sT))) sT = ~sT;
				break;
			}

			occ ^= (
						1ULL << (my_attackers & (board.pieces(static_cast<Piece>(Piece::underlying(pt)).type(), static_cast<Piece>(Piece::underlying(pt)).color()) |
						board.pieces(static_cast<Piece>(Piece::underlying(pt + 6)).type(), static_cast<Piece>(Piece::underlying(pt + 6)).color())))
						.lsb()
					);

			if (pt == chess::PAWN || pt == chess::BISHOP || pt == chess::QUEEN)
				attackers |= attacks::bishop(to_sq, occ) & bishops;

			if (pt == chess::ROOK || pt == chess::QUEEN)
				attackers |= attacks::rook(to_sq, occ) & rooks;
		}
		return sT != (from_sq.is_light() ? Color::WHITE : Color::BLACK);
	}
}  // namespace see
#endif // ! _SEE_HPP_