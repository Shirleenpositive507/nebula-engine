#pragma once

#include <SFML/Audio.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace nebula {
namespace audio {

enum class AudioFormat {
    Unknown,
    WAV,
    OGG,
    MP3,
    FLAC
};

enum class StreamingDecision {
    PreDecoded,
    Streamed
};

struct AudioFileInfo {
    AudioFormat format;
    float duration;
    unsigned int sampleRate;
    unsigned int channelCount;
    std::size_t sampleCount;
    std::size_t fileSize;
    int bitrate;
    bool valid;

    AudioFileInfo()
        : format(AudioFormat::Unknown)
        , duration(0.0f)
        , sampleRate(0)
        , channelCount(0)
        , sampleCount(0)
        , fileSize(0)
        , bitrate(0)
        , valid(false) {}
};

class AudioBuffer {
public:
    AudioBuffer();
    ~AudioBuffer() = default;

    bool loadFromFile(const std::string& filename);
    bool loadFromMemory(const void* data, std::size_t size);
    bool saveToFile(const std::string& filename) const;

    float getDuration() const;
    unsigned int getSampleRate() const;
    unsigned int getChannelCount() const;
    std::size_t getSampleCount() const;
    const std::int16_t* getSamples() const;
    std::size_t getSize() const;

    AudioFormat detectFormat(const std::string& filename) const;
    AudioFormat detectFormatFromHeader(const void* data, std::size_t size) const;
    AudioFileInfo getFileInfo() const;
    StreamingDecision getStreamingDecision() const;
    std::size_t getMemoryUsage() const;

    sf::SoundBuffer& getSFMLBuffer();
    const sf::SoundBuffer& getSFMLBuffer() const;

    static bool isFormatSupported(AudioFormat format);
    static std::string formatToString(AudioFormat format);
    static std::size_t estimateMemoryUsage(const AudioFileInfo& info);

private:
    AudioFormat detectFormatByExtension(const std::string& filename) const;
    AudioFormat detectFormatByMagic(const uint8_t* data, std::size_t size) const;

    sf::SoundBuffer m_buffer;
    AudioFileInfo m_fileInfo;
    AudioFormat m_detectedFormat = AudioFormat::Unknown;
    StreamingDecision m_streamingDecision = StreamingDecision::PreDecoded;
    std::size_t m_rawMemoryUsage = 0;
};

class AudioBufferManager {
public:
    AudioBufferManager() = default;
    ~AudioBufferManager() = default;

    std::shared_ptr<AudioBuffer> loadBuffer(const std::string& name, const std::string& filepath);
    std::shared_ptr<AudioBuffer> getBuffer(const std::string& name) const;
    bool unloadBuffer(const std::string& name);
    void preloadBuffers(const std::vector<std::pair<std::string, std::string>>& buffers);
    std::size_t getMemoryUsage() const;
    std::size_t getBufferCount() const;
    std::vector<std::string> getLoadedBufferNames() const;
    bool isBufferLoaded(const std::string& name) const;

private:
    mutable std::unordered_map<std::string, std::shared_ptr<AudioBuffer>> m_cache;
    std::size_t m_totalMemoryUsage = 0;
};

} // namespace audio
} // namespace nebula
