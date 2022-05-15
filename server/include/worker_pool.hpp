#pragma once

#include "asio.hpp"

#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>
#include <queue>
#include <atomic>

#include "task/task.hpp"
#include "task/invoker.hpp"

/**
 * @class WorkerPool пул потоков, который разбирают и выполняют очередь задач
 */
class WorkerPool final : public IInvoker
{
public:
	/**
	 * Ctor
	 * @param max_jobs Максимально возможное кол-во одновременно обрабатываемых
	 * задач
	 */
	explicit WorkerPool(uint max_jobs);

	WorkerPool(const WorkerPool&) = delete;
	void operator=(const WorkerPool&) = delete;

	/**
	 * Запуск потоков
	 */
	void run();

	/**
	 * Добавить задачу в очередь на обработку
	 * @param task задача
	 * @return true если задача добавлена, false если превышен лимит запросов
	 */
	bool invoke(TaskPtr task) override;

	/**
	 * Остановить пул задач
	 */
	void shutdown();

	~WorkerPool();

private:
	static const uint m_default_threads = 4;
	const uint        m_max_jobs;
	uint              m_thread_count{0};

	std::vector< std::thread > m_threads;

	std::queue< TaskPtr >   m_tasks;
	std::mutex              m_mutex;
	std::condition_variable m_cond;

	bool                m_stop{false};
	std::atomic< uint > m_now_running{0};
};