#pragma once
#ifndef _THREAD_MANAGER_HPP_
#define _THREAD_MANAGER_HPP_

#include <thread>
#include <atomic>

#include "searcher.hpp"

class SearchInstance
{
public:
    SearchInstance();
    SearchInstance(const SearchInstance &other);
    ~SearchInstance() = default;
    SearchInstance &operator=(const SearchInstance &other);
    void start() const;
    std::unique_ptr<Searcher> search;
};

class ThreadPool
{
    using U64 = uint64_t;
public:
    [[nodiscard]] U64 getNodes() const;
    [[nodiscard]] U64 getTbHits() const;
    void start(const chess::Board&, const Limits&, const chess::Movelist&, int, bool);
    void kill();
    std::atomic_bool stop;
private:
    std::vector<SearchInstance> pool;
    std::vector<std::thread> running_threads;
};

#endif // !_THREAD_MANAGER_HPP_