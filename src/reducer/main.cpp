// reducer_mt.cpp
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <future>
#include <algorithm>
#include <type_traits>

#include <common/thread_pool/queue.hpp>
#include <common/thread_pool/thread_pool.hpp>
#include <common/thread_pool/submit.hpp>

struct Stats {
    double revenue = 0.0;
    long long quantity = 0;
};


using map_type = std::unordered_map<std::string, Stats>;

map_type reduce_worker(tp::exe::Queue<std::string>& lines_queue)
{
    map_type local_map;
    // std::string line;

    std::vector<std::string> lines;
    while (lines_queue.Pop(lines))
    {
        for (auto& line : lines)
        {
            if (line.empty()) continue;
    
            std::stringstream ss(line);
            std::string category;
            std::string revenue_str, quantity_str;
    
            if (!std::getline(ss, category, '\t')) continue;
            if (!std::getline(ss, revenue_str, '\t')) continue;
            if (!std::getline(ss, quantity_str, '\t')) continue;
    
            double revenue = 0.0;
            long long qty = 0;
            try {
                revenue = std::stod(revenue_str);
                qty = std::stoll(quantity_str);
            } catch (...) {
                continue;
            }
    
            auto &st = local_map[category];
            st.revenue += revenue;
            st.quantity += qty;
        }
    }

    return local_map;
}

int main(int argc, const char* argv[]) 
{
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    size_t workers = std::thread::hardware_concurrency();
    
    if (argc > 1)
    {
        try
        {
            workers = std::stoul(argv[1]);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            workers = std::thread::hardware_concurrency();
        }
    }

    // pool and queue
    tp::exe::Queue<std::string> lines_queue;
    tp::exe::ThreadPool pool{workers};

    // futures
    std::vector<std::future<map_type>> futures;
    futures.reserve(pool.WorkerCount());

    // worker for pool
    std::function<map_type()> func = [&lines_queue](){
        return reduce_worker(lines_queue);
    };

    // starting workers
    for (unsigned int i = 0; i < pool.WorkerCount(); ++i) {
        futures.push_back(tp::exe::Submit(pool, func));
    }

    // parsing input stdin from hadoop
    std::string line;
    while (std::getline(std::cin, line)) 
    {
        if (line.empty()) continue;
        lines_queue.Push(std::move(line));
    }

    // stop waiting for new Push into lines_queue
    lines_queue.SetFinished();

    // merging chunsk
    map_type map;
    for (auto& fut : futures) 
    {
        auto chunk = fut.get();
        for (auto &kv : chunk) 
        {
            auto &dst = map[kv.first];
            dst.revenue += kv.second.revenue;
            dst.quantity += kv.second.quantity;
        }
    }

    std::vector<std::pair<std::string, Stats>> items;
    items.reserve(map.size());
    for (auto& kv : map) 
    {
        items.emplace_back(kv.first, kv.second);
    }

    // sorting
    std::sort(items.begin(), items.end(),
              [](const auto &a, const auto &b) {
                  return a.second.revenue > b.second.revenue;
              });

    // Printing result in format: category \t totalRevenue \t totalQuantity
    for (const auto& kv : items) 
    {
        const std::string &category = kv.first;
        const Stats &st = kv.second;
        std::cout << category << '\t'
                  << st.revenue << '\t'
                  << st.quantity << '\n';
    }

    return 0;
}

