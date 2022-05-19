#pragma once

#include "thread_helper.hh"

template <typename Callable, typename... Args>
void run_on_threads(size_t y_range, Callable f, Args... args)
{
    for (size_t i = 0; i < threads.size(); i++)
    {
        threads[i] = std::thread(f, args..., i * y_range, (i + 1) * y_range);
    }
    for (size_t i = 0; i < threads.size(); i++)
    {
        threads[i].join();
    }
}
