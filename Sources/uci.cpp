#include "../Headers/uci.hpp"

void UCI::Response(const std::string& message)
{
	if (message == "uci")
	{
		std::cout
			<< "option name Threads type spin default 1 min 1 max 1024" << std::endl
			<< "option name EvalFile type string default nothing.nnue " << std::endl
			<< "uciok" << std::endl;
	}
	else if (message == "ucinewgame") {}
	else if (message == "isready")
	{
		std::cout << "readyok" << std::endl;
	}
	else if (message.find("position") != std::string::npos)
	{
		this->Process_Position_Command(message);
	}
	else if (message.find("go") != std::string::npos)
	{
		this->Process_Go_Command(message);
	}
	else if (message.find("setoption") != std::string::npos)
	{
		this->Process_Set_Options_Command(message);
	}
	else if (message == "stop") { this->core.Stop_Thinking(); }
	else if (message == "d") { core.draw(); }
}

inline void UCI::Main_Loop()
{
	std::string message;

	while (message != "quit")
	{
		std::getline(std::cin, message);
		this->Response(message);
	}
}

inline void UCI::Process_Position_Command(const std::string& command)
{
	std::string FEN = chess::constants::STARTPOS;
	std::vector<std::string> MOVES;
	size_t fen_pos = command.find("fen");
	size_t moves_pos = command.find("moves");

	if (fen_pos != std::string::npos)
	{
		FEN = pystring::strip(std::string((command.begin() + fen_pos + 3),
			moves_pos != std::string::npos ? (command.begin() + moves_pos) : command.end()));
	}
	if (moves_pos != std::string::npos)
	{
		pystring::split(pystring::strip(std::string((command.begin() + moves_pos + 5), command.end())), MOVES);
	}

	core.set_fen(FEN);
	core.setMoves(MOVES);
}

inline void UCI::Process_Set_Options_Command(const std::string& command)
{
	const std::vector<std::string> text = pystring::split(command);
	const std::pair< std::string, std::string> Option(pystring::strip(text[2]), pystring::strip(text.back()));

	if (Option.first == "EvalFile")
	{
		//if (text.size() != 4) { this->core.init_NNUE("nn-04cf2b4ed1da.nnue"); }
		//else { this->core.init_NNUE(Option.second.c_str()); }
	}
}

inline void UCI::Process_Go_Command(const std::string& command)
{
	if (this->core.is_avilable()) { std::cout << "info string NNUE evaluation using nn-04cf2b4ed1da.nnue enabled" << std::endl; }

	if (command.find("infinite") != std::string::npos) { this->core.get_score_move(-1, 0, 0, 0, 0, true); }
	else if (command.find("depth") != std::string::npos) { this->core.get_score_move(unsigned(std::atoll(pystring::split(command).back().c_str())), 0, 0, 0, 0, true); }
	else if (command.find("movetime") != std::string::npos)
	{
		const size_t Time = std::atoll(pystring::split(command).back().c_str());
		this->core.get_score_move(-1, Time, 0, Time, 0, false);
	}
	else
	{
		std::vector<std::string> Splitted = pystring::split(command);
		size_t wtime, winc, btime, binc;

		if ((command.find("winc") != std::string::npos))
		{
			// go wtime 189600 btime 189600 winc 9000 binc 9000
			wtime = std::atoll(Splitted[2].c_str());
			btime = std::atoll(Splitted[4].c_str());
			winc = std::atoll(Splitted[6].c_str());
			binc = std::atoll(Splitted.back().c_str());
		}
		else
		{
			// go wtime 600 btime 600
			wtime = std::atoll(Splitted[2].c_str());
			btime = std::atoll(Splitted.back().c_str());
			winc = binc = 0;
		}

		this->core.get_score_move(-1, wtime, winc, btime, binc, false);
	}
}
