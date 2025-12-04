#pragma once

// std
#include <functional>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstddef>

namespace tp::exe
{

class ThreadPool
{
public:
    using Task = std::function<void()>;

public:
    ThreadPool(size_t workers_count = std::thread::hardware_concurrency());
    ~ThreadPool();

    size_t WorkerCount() const { return workers_.size(); }

    void EnqueueTask(Task task);

private:
    bool stop_pool_ = false;
    std::condition_variable cond_flag_;
    std::vector<std::thread> workers_;
    std::mutex task_queue_mutex_;
    std::queue<Task> tasks_;
};

} // namespace tp::exe