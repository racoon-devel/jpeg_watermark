#pragma once

#include "asio.hpp"

class ThreadPool final
{
public:
    ThreadPool();

private:
    static const uint m_default_threads = 4; 
    uint m_thread_count;
    
};