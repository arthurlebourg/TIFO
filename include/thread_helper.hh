#pragma once

#include <functional>
#include <thread>
#include <vector>

const size_t max_threads = std::thread::hardware_concurrency();
extern std::vector<std::thread> threads;

template <typename Callable, typename... Args>
void run_on_threads(size_t y_range, Callable f, Args... args);

#include "thread_helper.hxx"
