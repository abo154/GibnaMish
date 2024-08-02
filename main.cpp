#include <iostream>

#include "Headers/uci.hpp"

int main(void)
{
	UCI Engine;
	// Engine.Main_Loop();

	std::string message;

	while (message != "quit")
	{
		std::getline(std::cin, message);
		Engine.Response(message);
	}
}