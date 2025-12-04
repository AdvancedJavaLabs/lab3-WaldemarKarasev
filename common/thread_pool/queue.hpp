#pragma once

// std
#include <mutex>
#include <condition_variable>
#include <optional>
#include <queue>
#include <vector>

namespace tp::exe
{

template <typename T>
class Queue
{
public:
    using Result = T;

    void Push(Result result)
    {
        {
            std::lock_guard lock(mutex_);
            queue_.push(std::move(result));
        }
        cv_.notify_one();
    }

    bool TryPop(Result& value)
    {
        if (mutex_.try_lock())
        {
            if (!queue_.empty())
            {
                value = queue_.front();
                queue_.pop();
                mutex_.unlock();
                return true;
            }

            mutex_.unlock();
            return false;
        }

        return false;
    }

    bool TryPop(std::vector<Result>& vec_of_results, size_t batch_size)
    {
        if (mutex_.try_lock())
        {
            if (!queue_.empty())
            {
                vec_of_results.clear(); // for clean result
                while (!queue_.empty())
                {
                    vec_of_results.push_back(queue_.front());
                    queue_.pop();
                }
                mutex_.unlock();
                return true;
            }

            mutex_.unlock();
            return false;
        }

        return false;
    }

    bool Pop(T &out) 
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [&]{ return !queue_.empty() || finished_; });

        if (queue_.empty()) {
            return false;
        }

        out = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    bool Pop(std::vector<T>& out, size_t batch_size = 100'000)  
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [&]{ return !queue_.empty() || finished_; });

        if (queue_.empty()) {
            return false;
        }

        out.clear(); // for clean result
        while (!queue_.empty() && batch_size > 0)
        {
            out.push_back(std::move(queue_.front()));
            queue_.pop();
            --batch_size;
        }

        return true;
    }

    void SetFinished() 
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            finished_ = true;
        }
        cv_.notify_all();
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<Result> queue_;
    bool finished_ = false;
};

} // namespace tp::exe