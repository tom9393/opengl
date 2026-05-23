#pragma once

#include "pch.h"
#include "Asset/Asset.h"

namespace Engine
{
    // CPU-side audio asset: the whole file decoded to interleaved f32 PCM in RAM.
    // One AudioClip is shared by every AudioInstance that plays it (each instance
    // reads through its own cursor), mirroring how Texture2D backs the GPU.
    // Format is stored as the underlying ma_format value so this header stays
    // free of the large miniaudio.h include.
    class AudioClip : public Asset
    {
        public:
        AudioClip() { m_Type = AssetType::Audio; }

        const std::vector<uint8_t>& Data() const { return m_Data; }
        uint32_t Format() const { return m_Format; }
        uint32_t Channels() const { return m_Channels; }
        uint32_t SampleRate() const { return m_SampleRate; }
        uint64_t FrameCount() const { return m_FrameCount; }

        bool IsValid() const { return !m_Data.empty(); }

        private:
        friend class AudioImporter;

        std::vector<uint8_t> m_Data;
        uint32_t m_Format = 0;
        uint32_t m_Channels = 0;
        uint32_t m_SampleRate = 0;
        uint64_t m_FrameCount = 0;
    };
}
