#include "thread_pool.hpp"

// #include "thread_pool.hpp"

// std
#include <iostream>

namespace tp::exe
{   

ThreadPool::ThreadPool(size_t workers_count)
{
    // std::cout << "thread_pool workers=" << workers_count << std::endl;
    for (size_t i = 0; i < workers_count; ++i)
    {
        workers_.push_back(std::thread([this](){

            while (true)
            {
                Task task;

                {
                    std::unique_lock lock(task_queue_mutex_);

                    cond_flag_.wait(lock, [this](){
                        return stop_pool_ || !tasks_.empty();
                    });

                    if (stop_pool_ && tasks_.empty())
                    {
                        // std::cout << "breaking" << std::endl;
                        break;
                    }

                    task = std::move(tasks_.front());
                    tasks_.pop();
                }

                task();
            }

        }));
    }
}

ThreadPool::~ThreadPool()
{
    // std::cout << "~ThreadPool()" << std::endl;
    {
        std::unique_lock lock(task_queue_mutex_);
        stop_pool_ = true;
    }

    cond_flag_.notify_all();

    for (auto& thread : workers_)
    {
        thread.join();
    }
}

void ThreadPool::EnqueueTask(Task task)
{
    std::lock_guard lock(task_queue_mutex_);
    if (!stop_pool_)
    {
        tasks_.push(std::move(task));
    }

    cond_flag_.notify_all();
}

} // namespace tp::exe
