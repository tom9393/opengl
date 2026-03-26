#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

namespace Engine {

    class ThreadPool {
    public:
        // Initialize the pool with a specific number of threads
        ThreadPool(size_t threads);
        ~ThreadPool();

        // Add a new job to the pool. Returns a std::future so you can wait on the result.
        template<class F, class... Args>
        auto Enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>
        {
            using return_type = typename std::invoke_result<F, Args...>::type;

            // Wrap the function in a packaged_task so we can safely return a future
            auto task = std::make_shared< std::packaged_task<return_type()> >(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );
                
            std::future<return_type> res = task->get_future();
            {
                std::unique_lock<std::mutex> lock(m_QueueMutex);

                // Don't allow enqueueing after stopping the pool
                if(m_Stop)
                    throw std::runtime_error("Enqueue on stopped ThreadPool");

                // Push a lambda that executes our packaged task
                m_Tasks.emplace([task](){ (*task)(); });
            }
            
            // Wake up one sleeping worker thread
            m_Condition.notify_one();
            return res;
        }

    private:
        // Need to keep track of threads so we can join them
        std::vector<std::thread> m_Workers;
        
        // The task queue
        std::queue<std::function<void()>> m_Tasks;
        
        // Synchronization tools
        std::mutex m_QueueMutex;
        std::condition_variable m_Condition;
        bool m_Stop;
    };


}
