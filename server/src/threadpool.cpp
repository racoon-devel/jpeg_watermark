#include "threadpool.hpp"

#include <thread>

ThreadPool::ThreadPool()
    : m_thread_count(std::thread::hardware_concurrency())
{
    if (!m_thread_count)
        m_thread_count = m_default_threads;
}