#pragma once

#include "asio.hpp"

#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>
#include <queue>
#include <atomic>

class ProtoSession;

using Image = std::vector< uint8_t >;

struct Task
{
	Task() : session(nullptr) {}

	Task(std::shared_ptr< ProtoSession > session, std::string&& text,
		 Image&& image)
		: session(session), text(std::move(text)), image(std::move(image))
	{
	}

	Task(Task&& other)
		: session(other.session), text(std::move(other.text)),
		  image(std::move(other.image))
	{
		other.session = nullptr;
	}

	Task(const Task&) = delete;
	void operator=(const Task&) = delete;

	void operator=(Task&& task)
	{
		session = task.session;
		text    = std::move(task.text);
		image   = std::move(task.image);
	}

	std::shared_ptr< ProtoSession > session;
	std::string                     text;
	Image                           image;
};

class ImageProcessor final
{
public:
	ImageProcessor() = delete;
	ImageProcessor(asio::io_service& io, uint max_jobs);

	ImageProcessor(const ImageProcessor&) = delete;
	void operator=(const ImageProcessor&) = delete;

	~ImageProcessor() { Stop(); }

	void Run();
	bool Enqueue(Task&& task);
	void Stop();

private:
	static const uint m_default_threads = 4;
	uint              m_thread_count;
	const uint        m_max_jobs;

	asio::io_service& m_io;

	std::vector< std::thread > m_threads;
	std::queue< Task >         m_tasks;

	std::mutex              m_mutex;
	std::condition_variable m_cond;
	bool                    m_stop;
	std::atomic< uint >     m_now_running;

	bool draw_watermark(const Image& image, const std::string& text,
						Image& result);
};