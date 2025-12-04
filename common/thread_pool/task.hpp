#pragma once

// std
#include <iostream>
#include <type_traits>

namespace tp::exe
{

struct ITask
{
public:
    ITask()
    {
        // std::cout << "ITask()" << std::endl;
    }
    virtual ~ITask()
    {
        // std::cout << "~ITask()" << std::endl;
    } 
    virtual void operator()()
    {
        try 
        {
            Run();
        }
        catch(const std::exception& e)
        {
            std::cerr << "ITask:::exception: " << e.what() << std::endl;
        }
    }

protected:
    virtual void Run() = 0;

};

} // namespace tp::exe
