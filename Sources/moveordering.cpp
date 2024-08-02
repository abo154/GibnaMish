#include <algorithm>

#include "../Headers/piecesbouns.hpp"
#include "../Headers/moveordering.hpp"

MoveOrdering::MoveOrdering(TranspositionTable& TT):
	tt(TT),
	maxKillerMovePly(global::MAX_PLY + 1)
{}

MoveOrdering::Killers::Killers():
	moveA(Move::NO_MOVE),
	moveB(Move::NO_MOVE)
{}

void MoveOrdering::Order(const chess::Board& Board, Movelist& Moves, const bool inQSearch, const uint32_t PlyD)
{
	if (Moves.size() <= 0) { return; }
	for (Move& move : Moves)
	{
		int16_t Score = 0;
		const Square Start = move.from();
		const Square Target = move.to();
		const Piece CurrentPiece = Board.at(Start);
		const Piece CapturedPiece = Board.at(Target);
		const uint8_t CurrentPiece_value = uint8_t(CurrentPiece.type().internal());
		const uint8_t CapturedPiece_value = uint8_t(CapturedPiece.type().internal());
		const bool is_capture = CapturedPiece != PieceType::NONE;

		if (is_capture)
		{
			const int16_t captureMaterialDelta = CapturedPiece_value - CurrentPiece_value;
			const bool opponentCanRecapture = Board.isAttacked(Target, CapturedPiece.color());
			if (opponentCanRecapture) { Score += int16_t((captureMaterialDelta >= 0 ? Biases::winningCaptureBias : Biases::losingCaptureBias)) + captureMaterialDelta; }
			else { Score += int16_t(Biases::winningCaptureBias) + captureMaterialDelta; }
		}
		if (!is_capture)
		{
			const bool isKiller = !inQSearch && PlyD < this->maxKillerMovePly && this->KillerMoves[PlyD].Match(move);
			Score += int16_t(isKiller ? Biases::killerBias : Biases::regularBias);
			Score += this->History[Board.sideToMove()][move.from().index()][move.to().index()];
		}
		if (move == Move::PROMOTION) { Score += int16_t(uint8_t(move.promotionType().internal())); }

		move.setScore(Score);
	}

	std::sort(Moves.begin(), Moves.end(), [](const Move& c1, const Move& c2) { return c1.score() > c2.score(); });
}

void MoveOrdering::Killers::Add(const Move move)
{
	if (move.score() != moveA.score())
	{
		moveB = moveA;
		moveA = move;
	}
}

const bool MoveOrdering::Killers::Match(const Move move) const
{
	return (move.score() == moveA.score() || move.score() == moveB.score());
}

void MoveOrdering::clear_history()
{
	for (std::array<std::array<int, 64>, 64>& arr1 : this->History) { for (std::array<int, 64>& arr2 : arr1) { arr2.fill({}); } }
}

void MoveOrdering::clear_killer_moves()
{
	this->KillerMoves.fill({});
}

void MoveOrdering::clear()
{
	this->clear_history();
	this->clear_killer_moves();
}

MoveOrdering::~MoveOrdering()
{
	this->clear();
}
