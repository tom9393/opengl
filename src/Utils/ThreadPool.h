#pragma once

#include "pch.h"

namespace Engine {

class ThreadPool {
public:
    ThreadPool(size_t threadCount);
    ~ThreadPool();

    template<class F, class... Args>
    auto Enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>
    {
        using ReturnType = std::invoke_result_t<F, Args...>;

        // packaged_task lets us extract a future before handing ownership to the queue
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<ReturnType> result = task->get_future();
        {
            std::unique_lock<std::mutex> lock(m_QueueMutex);
            if (m_Stop)
                throw std::runtime_error("Enqueue on stopped ThreadPool");
            m_Tasks.emplace([task]() { (*task)(); });
        }
        m_Condition.notify_one();
        return result;
    }

private:
    std::vector<std::thread>          m_Workers;
    std::queue<std::function<void()>> m_Tasks;

    std::mutex              m_QueueMutex;
    std::condition_variable m_Condition;
    bool                    m_Stop;
};

}
