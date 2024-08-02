#include "../Headers/piecesbouns.hpp"

void piecesbouns::Reverse_Copy(int sarr[], const int arr[], const int size)
{
	for (size_t i = 0; i < 8; ++i)
		for (size_t j = 0; j < 8; ++j)
			sarr[i * 8 + j] = arr[(7 - i) * 8 + j];
}

void piecesbouns::init_pieces_bouns()
{
	for (uint8_t i = 0; i < 2; i++)
		for (uint8_t j = uint8_t(Punderlying::PAWN); j < uint8_t(Punderlying::NONE); j++)
			Reverse_Copy(pieceSquareScore[i][j][0], pieceSquareScore[i][j][1], 64);
}