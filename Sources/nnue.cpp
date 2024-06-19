#include "../Headers/nnue.hpp"
#include "../incbin/incbin.h"

INCBIN(__INIT__NNUE__, "nn-04cf2b4ed1da.nnue");

NNUE::NNUE():
	Avilable(false)
{
	this->Avilable = this->init(g__INIT__NNUE__Data, g__INIT__NNUE__Size);
}