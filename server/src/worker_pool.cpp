#include "worker_pool.hpp"

#include "easylogging++.h"

std::ostream& operator<<(std::ostream& os, const WorkerPool* other)
{
	(void) other;
	os << "[Thread:" << std::this_thread::get_id() << "] ";
	return os;
}

WorkerPool::WorkerPool(uint max_jobs)
	: m_max_jobs(max_jobs), m_thread_count(std::thread::hardware_concurrency())
{
	if (!m_thread_count)
		m_thread_count = m_default_threads;

	// нет смысла порождать много потоков, если макс. кол-во одновременно
	// работающих задач ограничено
	if (max_jobs)
	{
		m_thread_count = std::min(m_thread_count, max_jobs);
	}
}

void WorkerPool::run()
{
	for (uint idx = 0; idx < m_thread_count; idx++)
	{
		m_threads.emplace_back(
			[this, idx]
			{
				LOG(DEBUG) << this << "#" << idx << " Thread started";

				while (true)
				{
					TaskPtr task;
					{
						std::unique_lock< std::mutex > lock(m_mutex);
						m_cond.wait(lock,
									[this]
									{ return m_stop || !m_tasks.empty(); });

						if (m_stop)
						{
							LOG(DEBUG)
								<< this << "#" << idx << " Thread stopped";
							return;
						}

						LOG(INFO) << this << "#" << idx << " Executing job...";

						task = std::move(m_tasks.front());

						m_tasks.pop();
					}

					// взяли задачу в работу - увеличили счетчик
					m_now_running.fetch_add(1);

					// запускаем задачу
					task->execute();

					LOG(INFO) << this << "#" << idx << " Job completed";
					m_now_running.fetch_sub(1);
				}
			});
	}
}

bool WorkerPool::invoke(TaskPtr task)
{
	{
		std::unique_lock< std::mutex > lock(m_mutex);

		// Проверяем условие, достигнут ли максимум задач - сколько сейчас в
		// очереди и сколько в обработке
		if (m_max_jobs && m_now_running.load() + m_tasks.size() >= m_max_jobs)
			return false;

		m_tasks.push(std::move(task));
	}

	m_cond.notify_one();

	return true;
}

void WorkerPool::shutdown()
{
	{
		std::unique_lock< std::mutex > lock(m_mutex);
		m_stop = true;
	}

	m_cond.notify_all();

	for (std::thread& worker : m_threads)
	{
		worker.join();
	}

	m_threads.clear();
}

WorkerPool::~WorkerPool()
{
	shutdown();
}
