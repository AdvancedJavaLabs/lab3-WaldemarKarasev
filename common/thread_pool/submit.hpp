#pragma once

// common
#include "thread_pool.hpp"

#include <future>
#include <functional>
#include <memory>

namespace tp::exe
{

template <typename Task>    
void Submit(ThreadPool& pool, Task&& task)
{
    // std::cout << "Submit" << std::endl;
    pool.EnqueueTask(task);
}

template <typename T>
std::future<T> Submit(ThreadPool& pool, std::function<T()> task)
{
    std::shared_ptr<std::promise<T>> p = std::make_shared<std::promise<T>>();
    std::future<T> f = p->get_future();
    pool.EnqueueTask([task = std::move(task), p](){
        try
        {
            T v = task();
            p->set_value(std::move(v));
        }
        catch(const std::exception& e)
        {
            p->set_exception(std::current_exception());
        }
    });
    
    return f;
}

} // namespace tp::exe
