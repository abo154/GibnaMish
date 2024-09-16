#include "../Headers/threadmanager.hpp"

SearchInstance::SearchInstance():
    search(std::make_unique<Searcher>())
{}

SearchInstance::SearchInstance(const SearchInstance &other):
    search(std::make_unique<Searcher>(*other.search))
{}

SearchInstance& SearchInstance::operator=(const SearchInstance &other)
{
    // protect against invalid self-assignment
    if (this != &other)
        search = std::make_unique<Searcher>(*other.search);

    // by convention, always return *this
    return *this;
}

void SearchInstance::start() const
{
    this->search->StartThinking();
}

ThreadPool::U64 ThreadPool::getNodes() const
{
    U64 total = 0;

    for (auto &th : this->pool)
        total += th.search->nodes;

    return total;
}

void ThreadPool::start(const chess::Board &board, const Limits &limit, const chess::Movelist &searchmoves,
                        int worker_count, bool use_tb)
{
    assert(running_threads.size() == 0);

    this->stop = false;

    SearchInstance mainThread;

    if (!this->pool.empty()) mainThread = pool[0];

    this->pool.clear();

    // update with info
    mainThread.search->id = 0;
    mainThread.search->Board = board;
    mainThread.search->limit = limit;
    mainThread.search->use_tb = use_tb;
    mainThread.search->nodes = 0;
    mainThread.search->node_effort.reset();
    mainThread.search->searchmoves = searchmoves;

    this->pool.emplace_back(mainThread);

    // start at index 1 to keep "mainthread" data alive
    mainThread.search->History.reset();
    mainThread.search->Counters.reset();

    for (int i = 1; i < worker_count; i++)
    {
        mainThread.search->id = i;

        this->pool.emplace_back(mainThread);
    }

    for (int i = 0; i < worker_count; i++)
        this->running_threads.emplace_back(&SearchInstance::start, std::ref(pool[i]));
}

void ThreadPool::kill()
{
    this->stop = true;

    for (auto &th : this->running_threads)
        if (th.joinable()) th.join();

    this->pool.clear();
    this->running_threads.clear();
}
