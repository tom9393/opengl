#pragma once

#include "pch.h"

namespace Engine {

    class AssetUploadQueue {
    public:
        // Worker thread submits tasks that need to be ran on main thread
        static void Submit(std::function<void()> uploadTask) {
            std::lock_guard<std::mutex> lock(s_Mutex);
            s_Queue.push(std::move(uploadTask));
        }

        // Main thread calls this to process previously submitted tasks
        static void Process() {
            std::lock_guard<std::mutex> lock(s_Mutex);
            while (!s_Queue.empty()) {
                auto task = s_Queue.front();
                task();
                s_Queue.pop();
            }
        }

    private:
        static inline std::mutex s_Mutex;
        static inline std::queue<std::function<void()>> s_Queue;
    };

}