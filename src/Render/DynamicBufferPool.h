#pragma once

#include "../pch.h"
#include <queue>

namespace Engine {

    struct Allocation {
        size_t Offset;
        size_t Size;
    };

    class DynamicBufferPool {
    public:
        DynamicBufferPool(size_t capacity) : m_Capacity(capacity) {
            glCreateBuffers(1, &m_RendererID);
            
            // The magic flags: Persistent and Coherent
            GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
            
            // Allocate immutable storage
            glNamedBufferStorage(m_RendererID, m_Capacity, nullptr, flags);
            
            // Map the pointer permanently
            m_MappedPtr = (uint8_t*)glMapNamedBufferRange(m_RendererID, 0, m_Capacity, flags);
        }

        ~DynamicBufferPool() {
            glUnmapNamedBuffer(m_RendererID);
            glDeleteBuffers(1, &m_RendererID);
        }

        Allocation Allocate(const void* data, size_t size)
        {
            // Alignment is crucial for SSBOs and UBOs! 
            // SSBOs usually require 16 or 256 byte alignment depending on the hardware.
            size_t alignment = 256;  // Make sure this is in alignment with UBO
            m_Head = (m_Head + alignment - 1) & ~(alignment - 1);

            // If we run out of space, wrap around to the beginning (Ring Buffer)
            if (m_Head + size > m_Capacity) {
                m_Head = 0;
                // Optional Sanity Check: If a single frame requires more memory than 
                // m_Capacity / m_MaxFramesInFlight, you will overwrite data!
                // In a production engine, you'd want to assert here if size > (m_Capacity / 3)
            }

            size_t currentOffset = m_Head;
            
            // Copy data directly to mapped GPU memory (Zero driver overhead!)
            memcpy(m_MappedPtr + currentOffset, data, size);
            
            m_Head += size;
            
            return { currentOffset, size };
        }

        void BindAsSSBO(uint32_t bindingPoint, size_t offset, size_t size) {
            glBindBufferRange(GL_SHADER_STORAGE_BUFFER, bindingPoint, m_RendererID, offset, size);
        }

        void BindAsUBO(uint32_t bindingPoint, size_t offset, size_t size) {
            glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, m_RendererID, offset, size);
        }

        void BeginFrame()
        {
            // If we have reached our maximum allowed frames in flight, 
            // we must wait for the oldest frame to finish on the GPU.
            while (m_FrameFences.size() >= m_MaxFramesInFlight) {
                GLsync oldestSync = m_FrameFences.front();
                
                // Ask OpenGL to wait. We use a short timeout (e.g., 1ms = 1,000,000 nanoseconds)
                // inside a loop to prevent infinite deadlocks in case of driver crashes.
                // GL_SYNC_FLUSH_COMMANDS_BIT ensures the GPU actually starts processing.
                GLenum waitReturn = glClientWaitSync(oldestSync, GL_SYNC_FLUSH_COMMANDS_BIT, 1000000);
                
                if (waitReturn == GL_ALREADY_SIGNALED || waitReturn == GL_CONDITION_SATISFIED) {
                    // The GPU has passed the fence! It is done reading that frame's memory.
                    // We can safely clean up the sync object and proceed.
                    glDeleteSync(oldestSync);
                    m_FrameFences.pop();
                } 
                else if (waitReturn == GL_WAIT_FAILED) {
                    // Something went terribly wrong in the GL driver
                    break; 
                }
                // If it returns GL_TIMEOUT_EXPIRED, the loop continues and we wait again.
            }
        }

        void InsertFrameFence()
        {
            GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
            m_FrameFences.push(sync);
        }

    private:
        uint32_t m_RendererID;
        size_t m_Capacity;
        uint8_t* m_MappedPtr;
        size_t m_Head = 0;
        std::queue<GLsync> m_FrameFences;
        const size_t m_MaxFramesInFlight = 3;
    };
}