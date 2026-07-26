#ifndef ZENITH_JOB_SYSTEM_H
#define ZENITH_JOB_SYSTEM_H

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <iostream>

namespace zenith {

class JobSystem {
private:
    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_tasks;
    std::mutex m_queueMutex;
    std::condition_variable m_condition;
    std::atomic<int> m_activeJobs{0};
    bool m_stop = false;

public:
    JobSystem() = default;
    ~JobSystem() { shutdown(); }

    void initialize(unsigned int numThreads = 0) {
        if (numThreads == 0) {
            numThreads = std::thread::hardware_concurrency();
            if (numThreads == 0) numThreads = 4;
        }

        m_stop = false;
        for (unsigned int i = 0; i < numThreads; ++i) {
            m_workers.emplace_back([this]() {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->m_queueMutex);
                        this->m_condition.wait(lock, [this]() {
                            return this->m_stop || !this->m_tasks.empty();
                        });
                        if (this->m_stop && this->m_tasks.empty()) return;
                        task = std::move(this->m_tasks.front());
                        this->m_tasks.pop();
                    }
                    task();
                    this->m_activeJobs--;
                }
            });
        }
    }

    void submit(std::function<void()> job) {
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_tasks.push(job);
            m_activeJobs++;
        }
        m_condition.notify_one();
    }

    // Parallel For loop across CPU worker threads
    void parallelFor(std::size_t count, std::size_t batchSize, std::function<void(std::size_t index)> jobFunc) {
        if (count == 0) return;
        if (m_workers.empty()) initialize();

        std::size_t batches = (count + batchSize - 1) / batchSize;
        for (std::size_t b = 0; b < batches; ++b) {
            std::size_t start = b * batchSize;
            std::size_t end = std::min(start + batchSize, count);

            submit([start, end, jobFunc]() {
                for (std::size_t i = start; i < end; ++i) {
                    jobFunc(i);
                }
            });
        }
        wait();
    }

    void wait() {
        while (m_activeJobs > 0) {
            std::this_thread::yield();
        }
    }

    void shutdown() {
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_stop = true;
        }
        m_condition.notify_all();
        for (std::thread& worker : m_workers) {
            if (worker.joinable()) worker.join();
        }
        m_workers.clear();
    }

    unsigned int getThreadCount() const { return static_cast<unsigned int>(m_workers.size()); }
};

} // namespace zenith

#endif // ZENITH_JOB_SYSTEM_H
