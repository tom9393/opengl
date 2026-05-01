#pragma once

#include "../pch.h"

namespace Engine {

    class AssetUploadQueue {
    public:
        // Worker threads call this to push GPU tasks back to the main thread
        static void Submit(std::function<void()> uploadTask) {
            std::lock_guard<std::mutex> lock(s_Mutex);
            s_Queue.push(std::move(uploadTask));
        }

        // The Main thread calls this every frame to process the queue
        static void Process() {
            std::lock_guard<std::mutex> lock(s_Mutex);
            while (!s_Queue.empty()) {
                auto task = s_Queue.front();
                task(); // Execute the OpenGL creation logic
                s_Queue.pop();
            }
        }

    private:
        static inline std::mutex s_Mutex;
        static inline std::queue<std::function<void()>> s_Queue;
    };

}