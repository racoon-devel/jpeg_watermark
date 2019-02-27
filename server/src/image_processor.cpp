#include "image_processor.hpp"

#include <thread>
#include <iostream>
#include <memory>
#include <random>

#include "session.hpp"
#include "easylogging++.h"

std::ostream& operator<<(std::ostream& os, const ImageProcessor * other)
{   
    (void)other;
    os << "[Thread:" << std::this_thread::get_id() << "] ";
    return os;
}

ImageProcessor::ImageProcessor(asio::io_service& io, uint max_jobs)
    : m_thread_count(std::thread::hardware_concurrency()),
    m_max_jobs(max_jobs),
    m_io(io),
    m_stop(false),
    m_now_running(0)
{
    if (!m_thread_count)
        m_thread_count = m_default_threads;
}

void ImageProcessor::Run()
{
    for (uint idx = 0; idx < m_thread_count; idx++)
    {
        m_threads.emplace_back(
            [this, idx] 
            {
                // XXX только для тестов серверной части
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(3000, 4000);

                LOG(DEBUG) << this << "#" << idx << " Thread started";

                while (true)
                {
                    Task task;
                    {
                        std::unique_lock<std::mutex> lock(this->m_mutex);
                        this->m_cond.wait(lock, [this] { return this->m_stop || !this->m_tasks.empty();});

                        if(this->m_stop)
                        {
                            LOG(DEBUG) << this << "#" << idx << " Thread stopped";
                            return;
                        }

                        LOG(INFO) << this << "#" << idx << " Starting new job...";

                        task = std::move(m_tasks.front());
 
                        this->m_tasks.pop();
                    }

                    // взяли задачу в работу - увеличили счетчик
                    this->m_now_running.fetch_add(1);

                    // запускаем задачу - пока просто ждем случайное время
                    std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));

                    LOG(INFO) << this << "#" << idx << " Job completed";

                    std::vector<uint8_t> fake_data(8192 * 32, 0);
                    auto session = task.session;
                    // TODO: избежать копирования
                    // Отправляем оповещение о результате на главный поток
                    m_io.post([session,  fake_data] { session->complete(fake_data); });

                    this->m_now_running.fetch_sub(1);
                }
            }
        );
    }
}

bool ImageProcessor::Enqueue(Task&& task)
{
    // Проверяем условие, достигнут ли максимум задач
    if (m_now_running.load() >= m_max_jobs)
        return false;

    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_tasks.push(std::move(task));
    }

    m_cond.notify_one();

    return true;
}

void ImageProcessor::Stop()
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_stop = true;
    }

    m_cond.notify_all();

    for(std::thread &worker: m_threads)
        worker.join();

    m_threads.clear();
}