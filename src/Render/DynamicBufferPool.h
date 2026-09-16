#pragma once

#include "pch.h"
#include "Core/Application.h"

namespace Engine {

    struct Allocation {
        size_t Offset;
        size_t Size;
    };

    class DynamicBufferPool {
    public:
        DynamicBufferPool(size_t capacity) : m_Capacity(capacity) {

            m_FrameCapacity = m_Capacity / m_MaxFramesInFlight;
            glCreateBuffers(1, &m_RendererID);

            // Offsets are handed to both glBindBufferRange targets, so they must satisfy the stricter of the two hardware alignments
            GLint uboAlignment = 0, ssboAlignment = 0;
            glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uboAlignment);
            glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &ssboAlignment);
            m_Alignment = (size_t)(uboAlignment > ssboAlignment ? uboAlignment : ssboAlignment);
            if (m_Alignment == 0) m_Alignment = 256;

            // Coherent persistent mapping: writes become visible to GL commands
            // without per-allocation flushes (write-combined memory either way;
            // the frame fence provides the cross-frame ordering)
            GLbitfield storageFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
            GLbitfield mapFlags = storageFlags;

            glNamedBufferStorage(m_RendererID, m_Capacity, nullptr, storageFlags);

            m_MappedPtr = (uint8_t*)glMapNamedBufferRange(m_RendererID, 0, m_Capacity, mapFlags);
            if (m_MappedPtr == nullptr) {
                std::cerr << "FATAL ERROR: Failed to map persistent buffer! OpenGL Error: " << glGetError() << std::endl;
            }
        }

        ~DynamicBufferPool() {
            glUnmapNamedBuffer(m_RendererID);
            glDeleteBuffers(1, &m_RendererID);
        }

        // Returns { 0, 0 } when the current frame's block is exhausted; callers skip the draw that needed the allocation
        Allocation Allocate(const void* data, size_t size)
        {
            PROFILE_FUNCTION();
            m_Head = (m_Head + m_Alignment - 1) & ~(m_Alignment - 1);

            if (m_Head + size > (m_CurrentFrame + 1) * m_FrameCapacity) {
                std::cerr << "DynamicBufferPool: out of memory for current frame, skipping allocation of " << size << " bytes\n";
                return { 0, 0 };
            }

            size_t currentOffset = m_Head;

            // Copy data directly to mapped GPU memory
            memcpy(m_MappedPtr + currentOffset, data, size);

            m_Head += size;
            
            return { currentOffset, size };
        }

        void BindAsSSBO(uint32_t bindingPoint, size_t offset, size_t size) {
            PROFILE_FUNCTION();
            glBindBufferRange(GL_SHADER_STORAGE_BUFFER, bindingPoint, m_RendererID, offset, size);
        }

        void BindAsUBO(uint32_t bindingPoint, size_t offset, size_t size) {
            PROFILE_FUNCTION();
            glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, m_RendererID, offset, size);
        }

        void BeginFrame()
        {
            PROFILE_FUNCTION();
            while (m_FrameFences.size() >= m_MaxFramesInFlight) {
                GLsync oldestSync = m_FrameFences.front();
                
                GLuint64 timeout = 1000000000; 
                GLenum waitReturn = glClientWaitSync(oldestSync, GL_SYNC_FLUSH_COMMANDS_BIT, timeout);
                
                if (waitReturn == GL_ALREADY_SIGNALED || waitReturn == GL_CONDITION_SATISFIED) {
                    glDeleteSync(oldestSync);
                    m_FrameFences.pop();
                } 
                else if (waitReturn == GL_WAIT_FAILED) {
                    std::cerr << "GL_WAIT_FAILED in BeginFrame\n";
                    break; 
                }
            }
            m_CurrentFrame = (m_CurrentFrame + 1) % m_MaxFramesInFlight;
    
            // Reset head to the start of the current frame's dedicated memory block
            m_Head = m_CurrentFrame * m_FrameCapacity;
        }

        void InsertFrameFence()
        {
            GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
            m_FrameFences.push(sync);
        }

    private:
        uint32_t m_RendererID;
        size_t m_Capacity;
        size_t m_Alignment = 256;
        uint8_t* m_MappedPtr;
        size_t m_Head = 0;
        std::queue<GLsync> m_FrameFences;
        const size_t m_MaxFramesInFlight = 3;
        size_t m_CurrentFrame = 0;
        size_t m_FrameCapacity;
    };
}