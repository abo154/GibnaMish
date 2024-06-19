#pragma once
#ifndef _NNUE_HPP_
#define _NNUE_HPP_

#include <fstream>

#include "../nnue/nnue.h"
#include "chesslib/chess.hpp"

struct NNUE
{
private:
	using US = chess::Square::underlying;
	using U8 = uint8_t;
public:
	NNUE();
	inline bool init(const void*, const size_t);
	inline void init(const char*);
	inline int evaluate(const int, int*, int*);
	inline int evaluate_fen(const char*);

	bool Avilable;
	const int nnue_pieces[12] = { 6, 5, 4, 3, 2, 1, 12, 11, 10, 9, 8, 7 };
	const int nnue_squares[64] = {
		U8(US::SQ_A1), U8(US::SQ_B1), U8(US::SQ_C1), U8(US::SQ_D1), U8(US::SQ_E1), U8(US::SQ_F1), U8(US::SQ_G1), U8(US::SQ_H1),
		U8(US::SQ_A2), U8(US::SQ_B2), U8(US::SQ_C2), U8(US::SQ_D2), U8(US::SQ_E2), U8(US::SQ_F2), U8(US::SQ_G2), U8(US::SQ_H2),
		U8(US::SQ_A3), U8(US::SQ_B3), U8(US::SQ_C3), U8(US::SQ_D3), U8(US::SQ_E3), U8(US::SQ_F3), U8(US::SQ_G3), U8(US::SQ_H3),
		U8(US::SQ_A4), U8(US::SQ_B4), U8(US::SQ_C4), U8(US::SQ_D4), U8(US::SQ_E4), U8(US::SQ_F4), U8(US::SQ_G4), U8(US::SQ_H4),
		U8(US::SQ_A5), U8(US::SQ_B5), U8(US::SQ_C5), U8(US::SQ_D5), U8(US::SQ_E5), U8(US::SQ_F5), U8(US::SQ_G5), U8(US::SQ_H5),
		U8(US::SQ_A6), U8(US::SQ_B6), U8(US::SQ_C6), U8(US::SQ_D6), U8(US::SQ_E6), U8(US::SQ_F6), U8(US::SQ_G6), U8(US::SQ_H6),
		U8(US::SQ_A7), U8(US::SQ_B7), U8(US::SQ_C7), U8(US::SQ_D7), U8(US::SQ_E7), U8(US::SQ_F7), U8(US::SQ_G7), U8(US::SQ_H7),
		U8(US::SQ_A8), U8(US::SQ_B8), U8(US::SQ_C8), U8(US::SQ_D8), U8(US::SQ_E8), U8(US::SQ_F8), U8(US::SQ_G8), U8(US::SQ_H8)
	};
};

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	inline bool NNUE::init(const void* evalData, const size_t size)
	{
		const bool success = verify_net(evalData, size);
		if (success) { init_weights(evalData); }
		return success;
	}

	inline void NNUE::init(const char* evalFile)
	{
		std::ifstream file(evalFile, std::ios::binary);
		if (file.is_open())
		{
			std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			bool suc = this->init(content.c_str(), content.size());
			this->Avilable = suc;
			if (suc) std::cout << "file loded" << std::endl;
			else std::cout << "not file loded" << std::endl;
		}
		else { std::cout << "file not found" << std::endl; }
		file.close();
	}

	inline int NNUE::evaluate(const int player, int* pieces, int* squares)
	{
		return nnue_evaluate(player, pieces, squares);
	}

	inline int NNUE::evaluate_fen(const char* fen)
	{
		return nnue_evaluate_fen(fen);
	}

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_NNUE_HPP_