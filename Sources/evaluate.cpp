#include "../Headers/evaluate.hpp"
#include "../Headers/piecesbouns.hpp"

Score Evaluate::evaluate(const chess::Board& Board)
{
	using PHASE = piecesbouns::PHASE;
	using Punderlying = piecesbouns::Punderlying;

	int eval_eg = 0;
	int eval_mg = 0;
	int phase = 0;

	const chess::Bitboard wpawns_board = Board.pieces(chess::PieceType::PAWN, Color::WHITE);
	const chess::Bitboard bpawns_board = Board.pieces(chess::PieceType::PAWN, Color::BLACK);

	const int wpawns = wpawns_board.count();
	const int wknights = Board.pieces(chess::PieceType::KNIGHT, Color::WHITE).count();
	const int wbishops = Board.pieces(chess::PieceType::BISHOP, Color::WHITE).count();
	const int wrooks = Board.pieces(chess::PieceType::ROOK, Color::WHITE).count();
	const int wqueens = Board.pieces(chess::PieceType::QUEEN, Color::WHITE).count();

	const int bpawns = bpawns_board.count();
	const int bknights = Board.pieces(chess::PieceType::KNIGHT, Color::BLACK).count();
	const int bbishops = Board.pieces(chess::PieceType::BISHOP, Color::BLACK).count();
	const int brooks = Board.pieces(chess::PieceType::ROOK, Color::BLACK).count();
	const int bqueens = Board.pieces(chess::PieceType::QUEEN, Color::BLACK).count();

	phase += wknights + bknights;
	phase += wbishops + bbishops;
	phase += (wrooks + brooks) * 2;
	phase += (wqueens + bqueens) * 4;

	eval_mg +=
		((wpawns - bpawns) * piecesbouns::PicesValues[uint8_t(PHASE::MG)][uint8_t(Punderlying::PAWN)]) +
		((wknights - bknights) * piecesbouns::PicesValues[uint8_t(PHASE::MG)][uint8_t(Punderlying::KNIGHT)]) +
		((wbishops - bbishops) * piecesbouns::PicesValues[uint8_t(PHASE::MG)][uint8_t(Punderlying::BISHOP)]) +
		((wrooks - brooks) * piecesbouns::PicesValues[uint8_t(PHASE::MG)][uint8_t(Punderlying::ROOK)]) +
		((wqueens - bqueens) * piecesbouns::PicesValues[uint8_t(PHASE::MG)][uint8_t(Punderlying::QUEEN)]);

	eval_eg +=
		((wpawns - bpawns) * piecesbouns::PicesValues[uint8_t(PHASE::EG)][uint8_t(Punderlying::PAWN)]) +
		((wknights - bknights) * piecesbouns::PicesValues[uint8_t(PHASE::EG)][uint8_t(Punderlying::KNIGHT)]) +
		((wbishops - bbishops) * piecesbouns::PicesValues[uint8_t(PHASE::EG)][uint8_t(Punderlying::BISHOP)]) +
		((wrooks - brooks) * piecesbouns::PicesValues[uint8_t(PHASE::EG)][uint8_t(Punderlying::ROOK)]) +
		((wqueens - bqueens) * piecesbouns::PicesValues[uint8_t(PHASE::EG)][uint8_t(Punderlying::QUEEN)]);

	const MG_EG mg_eg = Calc_adjustment(Board);
	eval_mg += mg_eg.first;
	eval_eg += mg_eg.second;

	if (wpawns_board & uint8_t(chess::Rank::underlying::RANK_7))
	{
		eval_mg += 20;
		eval_eg += 30;
	}
	if (bpawns_board & uint8_t(chess::Rank::underlying::RANK_2))
	{
		eval_mg -= 20;
		eval_eg -= 30;
	}

	phase = 24 - phase;
	phase = (phase * 256 + (24 / 2)) / 24;

	return (((eval_mg * (256 - phase)) + (eval_eg * phase)) / 256) * (Board.sideToMove() * -2 + 1);
}

Score Evaluate::ForceKingToCenter(const chess::Board& Board, const chess::Color my_king_color)
{
	int eval = 0;

	const int l =
		((piecesbouns::PicesValues[0][3] * 2 + piecesbouns::PicesValues[0][2] + piecesbouns::PicesValues[0][1]) +
		(piecesbouns::PicesValues[1][3] * 2 + piecesbouns::PicesValues[1][2] + piecesbouns::PicesValues[1][1])) / 2;
	const double multiplayer = 1.0 / l;
	const int nonPawnPiecesCount = (
		Board.pieces(chess::PieceType::QUEEN) |
		Board.pieces(chess::PieceType::ROOK)  |
		Board.pieces(chess::PieceType::BISHOP)|
		Board.pieces(chess::PieceType::KNIGHT)
	).count();

	const double end_game_weight = 1.0f - std::min(1.0, double(nonPawnPiecesCount) * multiplayer);

	const chess::Square oppking_square = Board.kingSq(!my_king_color);
	const chess::Rank oppking_rank = oppking_square.rank();
	const chess::File oppking_file = oppking_square.file();

	const int oppking_center_file = std::max(3 - oppking_file, oppking_file - 4);
	const int oppking_center_rank = std::max(3 - oppking_rank, oppking_rank - 4);
	const int oppking_from_center = oppking_center_file + oppking_center_rank;
	eval += oppking_from_center;

	const chess::Square myking_square = Board.kingSq(my_king_color);
	const chess::Rank myking_rank = myking_square.rank();
	const chess::File myking_file = myking_square.file();

	const int dst_between_king_file = std::abs(myking_file - oppking_file);
	const int dst_between_king_rank = std::abs(myking_rank - oppking_rank);
	const int dst_between_kings = dst_between_king_file + dst_between_king_rank;
	eval += 14 - dst_between_kings;

	return int(eval * 10 * end_game_weight);
}

Evaluate::MG_EG Evaluate::Calc_adjustment(const chess::Board& Board)
{
	const uint8_t MG = uint8_t(piecesbouns::PHASE::MG);
	const uint8_t EG = uint8_t(piecesbouns::PHASE::EG);
	chess::Bitboard white_pieces = Board.us(Color::WHITE);
	chess::Bitboard black_pieces = Board.us(Color::BLACK);
	int eval_mg = 0;
	int eval_eg = 0;

	while (white_pieces)
	{
		const uint8_t square = white_pieces.pop();
		const chess::Piece piece = Board.at(square);
		const uint8_t piece_type = uint8_t(piece.type().internal());
		const int8_t piece_color = int8_t(piece.color().internal());

		eval_mg += piecesbouns::pieceSquareScore[0][piece_type][piece_color][square];
		eval_eg += piecesbouns::pieceSquareScore[1][piece_type][piece_color][square];
	}

	while (black_pieces)
	{
		const uint8_t square = black_pieces.pop();
		const chess::Piece piece = Board.at(square);
		const uint8_t piece_type = uint8_t(piece.type().internal());
		const int8_t piece_color = int8_t(piece.color().internal());

		eval_mg -= piecesbouns::pieceSquareScore[0][piece_type][piece_color][square];
		eval_eg -= piecesbouns::pieceSquareScore[1][piece_type][piece_color][square];
	}

	return { eval_mg, eval_eg };
}
