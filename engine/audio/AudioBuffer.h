#pragma once

#include <SFML/Audio.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace nebula {
namespace audio {

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

    sf::SoundBuffer& getSFMLBuffer();
    const sf::SoundBuffer& getSFMLBuffer() const;

private:
    sf::SoundBuffer m_buffer;
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

private:
    mutable std::unordered_map<std::string, std::shared_ptr<AudioBuffer>> m_cache;
};

} // namespace audio
} // namespace nebula
