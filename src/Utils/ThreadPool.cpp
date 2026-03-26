#include "ThreadPool.h"

namespace Engine {

    // The constructor launches the worker threads
    ThreadPool::ThreadPool(size_t threads) : m_Stop(false)
    {
        for(size_t i = 0; i < threads; ++i)
        {
            m_Workers.emplace_back(
                [this]
                {
                    while(true)
                    {
                        std::function<void()> task;

                        {
                            // Wait until there is a task or the pool is stopped
                            std::unique_lock<std::mutex> lock(this->m_QueueMutex);
                            this->m_Condition.wait(lock,
                                [this]{ return this->m_Stop || !this->m_Tasks.empty(); });
                            
                            // If stopping and no more tasks, exit the thread
                            if(this->m_Stop && this->m_Tasks.empty())
                                return;
                            
                            // Grab the next task
                            task = std::move(this->m_Tasks.front());
                            this->m_Tasks.pop();
                        }

                        // Execute the task outside the mutex lock!
                        task();
                    }
                }
            );
        }
    }

    // The destructor joins all threads
    ThreadPool::~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(m_QueueMutex);
            m_Stop = true;
        }
        
        // Wake up all threads so they can finish and exit
        m_Condition.notify_all();
        
        for(std::thread &worker: m_Workers)
        {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

}
