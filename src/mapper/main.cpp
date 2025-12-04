// mapper_mt.cpp
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <thread>
#include <vector>
#include <mutex>
#include <functional>

#include <common/thread_pool/queue.hpp>
#include <common/thread_pool/thread_pool.hpp>
#include <common/thread_pool/submit.hpp>

struct Stats 
{
    double revenue = 0.0;
    long long quantity = 0;
};

using map_type = std::unordered_map<std::string, Stats>;


std::unordered_map<std::string, Stats> map_worker(tp::exe::Queue<std::string>& lines_queue) 
{
    std::unordered_map<std::string, Stats> result_map;
    // std::string line;
    std::vector<std::string> lines;
    while (lines_queue.Pop(lines)) 
    {
        for (auto& line : lines)
        {
            if (line.empty()) continue;
    
            std::stringstream ss(line);
            std::string transaction_id, product_id, category;
            std::string price_str, quantity_str;
    
            if (!std::getline(ss, transaction_id, ',')) continue;
            if (!std::getline(ss, product_id, ',')) continue;
            if (!std::getline(ss, category, ',')) continue;
            if (!std::getline(ss, price_str, ',')) continue;
            if (!std::getline(ss, quantity_str, ',')) continue;
    
            double price = 0.0;
            long long qty = 0;
            try {
                price = std::stod(price_str);
                qty = std::stoll(quantity_str);
            } catch (...) {
                continue;
            }
    
            double revenue = price * qty;
    
            auto& st = result_map[category];
            st.revenue += revenue;
            st.quantity += qty;
        }
    }

    return result_map;
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

    tp::exe::Queue<std::string> lines_queue;
    tp::exe::ThreadPool pool{workers};


    std::function<map_type()> func = [&lines_queue](){
        return map_worker(lines_queue);
    };

    std::vector<std::future<map_type>> results;
    for (unsigned int i = 0; i < pool.WorkerCount(); ++i) 
    {
        results.push_back(tp::exe::Submit(pool, func));
    }

    std::string line;
    while (std::getline(std::cin, line)) 
    {
        if (line.empty()) continue;

        if (line.rfind("transaction_id", 0) == 0) 
        {
            continue;
        }

        lines_queue.Push(std::move(line));
    }

    lines_queue.SetFinished();

    map_type map;
    for (auto &f : results)
    {
        auto chunk = f.get();
        for (auto& kv : chunk) 
        {
            auto& dst = map[kv.first];
            dst.revenue += kv.second.revenue;
            dst.quantity += kv.second.quantity;
        }
    }    


    for (const auto& kv : map) 
    {
        const std::string& category = kv.first;
        const Stats& st = kv.second;
        std::cout << category << '\t'
                  << st.revenue << '\t'
                  << st.quantity << '\n';
    }

    return 0;
}
