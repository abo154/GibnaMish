#include <iostream>

#include "Headers/uci.hpp"

TranspositionTable tt;
ThreadPool Threads;

int main(void)
{
	UCI Engine;
	Engine.Main_Loop();
}