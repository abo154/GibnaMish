#include <iostream>
#include <utility>

#include "../Headers/uci.hpp"
#include "../Headers/options.hpp"
#include "../Headers/str_utils.hpp"

constexpr uint64_t UCI_MAX_HASH_MB = static_cast<uint64_t>(TranspositionTable::MAXHASH_MiB * (1024 * 1024) / (1e6));
extern TranspositionTable tt;
extern ThreadPool Threads;
Options options;
static std::string convertScore(Score);

UCI::UCI()
{
	options = Options();
	this->Board = chess::Board();

	options.add_option(Option{"Hash", "spin", "16", "16", "1", std::to_string(UCI_MAX_HASH_MB)});  // Size in MB
	options.add_option(Option{"Threads", "spin", "1", "1", "1", "256"});

	this->ApplyOptions();
}

void UCI::Main_Loop()
{
	std::string message;

	while (message != "quit")
	{
		std::getline(std::cin, message);
		this->Response(message);
	}
	Threads.kill();
}

void UCI::Response(const std::string& message)
{
	std::vector<std::string> tokens = str_utils::split(message, ' ');

	if (!tokens.empty())
	{
		if (tokens[0] == "uci")
			this->Uci();

		else if (tokens[0] == "ucinewgame")
			this->Uci_New_Game();

		else if (tokens[0] == "isready")
			std::cout << "readyok" << std::endl;

		else if (tokens[0] == "position")
			this->Process_Position_Command(message);

		else if (tokens[0] == "go")
			this->Process_Go_Command(message);

		else if (tokens[0] == "setoption")
			this->Process_Set_Options_Command(message);

		else if (tokens[0] == "eval")
			std::cout << convertScore(Evaluate::evaluate(this->Board)) << std::endl;

		else if (tokens[0] == "d" || tokens[0] == "draw")
		{
			#ifdef _WIN32
				system("cls");
			#else
				system("clear");
			#endif
			std::cout << this->Board;
		}

		else if (tokens[0] == "cls" || tokens[0] == "clear")
		{
			#ifdef _WIN32
				system("cls");
			#else
				system("clear");
			#endif
		}

		else if (tokens[0] == "stop")
			Threads.kill();
	}
}

void UCI::Uci()
{
	std::cout << "id author abo154\n" << std::endl;
	options.print_options();
	std::cout << "uciok" << std::endl;
}

void UCI::Uci_New_Game()
{
	this->Board = chess::Board();
	tt.reset();
	Threads.kill();
}

void UCI::Process_Position_Command(const std::string& command)
{
	const auto fen_range = str_utils::find_range(command, "fen", "moves");

	const auto fen = str_utils::is_contain(command, "fen") ? command.substr(command.find("fen") + 3, fen_range) : chess::constants::STARTPOS;

	const auto moves = str_utils::is_contain(command, "moves") ? command.substr(command.find("moves") + 6) : "";
	const auto moves_vec = str_utils::split(moves, ' ');

	this->Board.setFen(fen);

	for (const auto& move : moves_vec)
		this->Board.makeMove(chess::uci::uciToMove(this->Board, move));
}

void UCI::Process_Set_Options_Command(const std::string& command)
{
	options.set_option(command);
	this->ApplyOptions();
}

void UCI::Process_Go_Command(const std::string& command)
{
	Threads.kill();
	Limits limit;

	const auto tokens = str_utils::split(command, ' ');

	if (tokens.size() == 1) limit.infinite = true;

	limit.depth = str_utils::find_element_value<int>(tokens, "depth").value_or(MAX_PLY - 1);
	limit.infinite = str_utils::find_element_value<std::string>(tokens, "go").value_or("") == "infinite";
	limit.nodes = str_utils::find_element_value<int64_t>(tokens, "nodes").value_or(0);
	limit.time = str_utils::find_element_value<int64_t>(tokens, "movetime").value_or(0);

	std::string uci_time = this->Board.sideToMove() == chess::Color::WHITE ? "wtime" : "btime";
	std::string uci_inc = this->Board.sideToMove() == chess::Color::WHITE ? "winc" : "binc";

	if (str_utils::is_contain(command, uci_time))
	{
		auto time = str_utils::find_element_value<int>(tokens, uci_time).value_or(0);
		auto inc = str_utils::find_element_value<int>(tokens, uci_inc).value_or(0);
		auto mtg = str_utils::find_element_value<int>(tokens, "movestogo").value_or(0);

		limit.time = this->timer.get_correct_time(time, inc, 40);
	}

	if (str_utils::is_contain(command, "searchmoves"))
	{
		const auto searchmoves = str_utils::find_element_value<std::string>(tokens, "searchmoves").value_or("");
		const auto moves = str_utils::split(searchmoves, ' ');

		this->Searchmoves.clear();

		for (const auto& move : moves)
			this->Searchmoves.add(chess::uci::uciToMove(this->Board, move));
	}

	Threads.start(this->Board, limit, this->Searchmoves, this->worker_threads, this->use_tb);
}

void UCI::ApplyOptions()
{
	this->worker_threads = options.get<int>("Threads");

	tt.resize(options.get<int>("Hash"));
}

std::string convertScore(Score score)
{
	constexpr int NormalizeToPawnValue = 131;

	if (std::abs(score) <= 4) score = 0;

	if (score >= VALUE_MATE_IN_PLY)
		return "mate " + std::to_string(((VALUE_MATE - score) / 2) + ((VALUE_MATE - score) & 1));
	else if (score <= VALUE_MATED_IN_PLY)
		return "mate " + std::to_string(-((VALUE_MATE + score) / 2) + ((VALUE_MATE + score) & 1));
	else
		return "cp " + std::to_string(score * 100 / NormalizeToPawnValue);
}
