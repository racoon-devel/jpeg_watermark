#include "image_processor.hpp"

#include <thread>
#include <iostream>
#include <memory>
#include <random>

#include "session.hpp"
#include "easylogging++.h"

#include <jpeglib.h>
#include <jerror.h>
#define cimg_display 0
#define cimg_plugin "plugins/jpeg_buffer.h"
#include "CImg.h"

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

    // Нет смысла порождать много потоков, если макс. кол-во одновременно работабщих задач ограничено
    m_thread_count = std::min(m_thread_count, max_jobs);
}

void ImageProcessor::Run()
{
    for (uint idx = 0; idx < m_thread_count; idx++)
    {
        m_threads.emplace_back(
            [this, idx] 
            {
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

                    // запускаем задачу - накладываем watermark
                    Image result;
                    bool retval = draw_watermark(task.image, task.text, result);

                    LOG(INFO) << this << "#" << idx << " Job completed";

                    auto session = task.session;
                    
                    // TODO: избежать копирования
                    // Отправляем оповещение о результате на главный поток
                    m_io.post([session, retval, result] { session->complete(retval, result); });

                    this->m_now_running.fetch_sub(1);
                }
            }
        );
    }
}

bool ImageProcessor::Enqueue(Task&& task)
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        // Проверяем условие, достигнут ли максимум задач - сколько сейчас в очереди и сколько в обработке
        if (m_now_running.load() + m_tasks.size() >= m_max_jobs)
            return false;
        
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

bool ImageProcessor::draw_watermark(const Image& image, const std::string& text, Image& result)
{
    try
    {
        cimg_library::CImg<uint8_t> img;

        LOG(DEBUG) << this << "source image size = " << image.size();

        img.load_jpeg_buffer(&image[0], image.size());

        const unsigned char green[] = { 0, 255, 0 };
        img.draw_text(0,0,text.c_str(),green,0,1,57);

        result.resize(image.size() * 2, 0);
        uint size = result.size();

        img.save_jpeg_buffer(&result[0], size, 90);

        LOG(DEBUG) << this << "output image size = " << size;

        result.resize(size, 0);
    }
    catch (const cimg_library::CImgException& e)
    {
        LOG(ERROR) << "Image processing failed. " << e.what();
        return false;
    }

    return true;
}