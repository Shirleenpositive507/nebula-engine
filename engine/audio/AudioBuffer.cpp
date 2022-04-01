#include "AudioBuffer.h"

namespace nebula {
namespace audio {

AudioBuffer::AudioBuffer() : m_buffer() {}

bool AudioBuffer::loadFromFile(const std::string& filename) {
    return m_buffer.loadFromFile(filename);
}

bool AudioBuffer::loadFromMemory(const void* data, std::size_t size) {
    return m_buffer.loadFromMemory(data, size);
}

bool AudioBuffer::saveToFile(const std::string& filename) const {
    return m_buffer.saveToFile(filename);
}

float AudioBuffer::getDuration() const {
    return m_buffer.getDuration().asSeconds();
}

unsigned int AudioBuffer::getSampleRate() const {
    return m_buffer.getSampleRate();
}

unsigned int AudioBuffer::getChannelCount() const {
    return m_buffer.getChannelCount();
}

std::size_t AudioBuffer::getSampleCount() const {
    return m_buffer.getSampleCount();
}

const std::int16_t* AudioBuffer::getSamples() const {
    return m_buffer.getSamples();
}

std::size_t AudioBuffer::getSize() const {
    return m_buffer.getSampleCount() * sizeof(std::int16_t) * m_buffer.getChannelCount();
}

sf::SoundBuffer& AudioBuffer::getSFMLBuffer() {
    return m_buffer;
}

const sf::SoundBuffer& AudioBuffer::getSFMLBuffer() const {
    return m_buffer;
}

std::shared_ptr<AudioBuffer> AudioBufferManager::loadBuffer(
    const std::string& name, const std::string& filepath) {
    auto buffer = std::make_shared<AudioBuffer>();
    if (buffer->loadFromFile(filepath)) {
        m_cache[name] = buffer;
        return buffer;
    }
    return nullptr;
}

std::shared_ptr<AudioBuffer> AudioBufferManager::getBuffer(const std::string& name) const {
    auto it = m_cache.find(name);
    if (it != m_cache.end()) {
        return it->second;
    }
    return nullptr;
}

bool AudioBufferManager::unloadBuffer(const std::string& name) {
    auto it = m_cache.find(name);
    if (it != m_cache.end()) {
        m_cache.erase(it);
        return true;
    }
    return false;
}

void AudioBufferManager::preloadBuffers(
    const std::vector<std::pair<std::string, std::string>>& buffers) {
    for (const auto& [name, filepath] : buffers) {
        loadBuffer(name, filepath);
    }
}

std::size_t AudioBufferManager::getMemoryUsage() const {
    std::size_t total = 0;
    for (const auto& [name, buffer] : m_cache) {
        total += buffer->getSize();
    }
    return total;
}

} // namespace audio
} // namespace nebula
