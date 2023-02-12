#include "AudioBuffer.h"
#include <algorithm>
#include <cstring>
#include <fstream>

namespace nebula {
namespace audio {

static const uint8_t WAV_HEADER[] = { 0x52, 0x49, 0x46, 0x46 };
static const uint8_t OGG_HEADER[] = { 0x4F, 0x67, 0x67, 0x53 };
static const uint8_t MP3_ID3[]    = { 0x49, 0x44, 0x33 };
static const uint8_t MP3_SYNC[]   = { 0xFF, 0xFB };
static const uint8_t FLAC_HEADER[] = { 0x66, 0x4C, 0x61, 0x43 };

AudioBuffer::AudioBuffer() : m_buffer() {}

bool AudioBuffer::loadFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) return false;

    std::size_t fileSize = static_cast<std::size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> headerData(16);
    if (file.read(reinterpret_cast<char*>(headerData.data()), 16)) {
        m_detectedFormat = detectFormatFromHeader(headerData.data(), 16);
    }

    m_detectedFormat = detectFormatByExtension(filename);

    bool loaded = m_buffer.loadFromFile(filename);
    if (loaded) {
        m_fileInfo.format = m_detectedFormat;
        m_fileInfo.duration = m_buffer.getDuration().asSeconds();
        m_fileInfo.sampleRate = m_buffer.getSampleRate();
        m_fileInfo.channelCount = m_buffer.getChannelCount();
        m_fileInfo.sampleCount = m_buffer.getSampleCount();
        m_fileInfo.fileSize = fileSize;
        m_fileInfo.valid = true;

        m_rawMemoryUsage = getSize();

        if (fileSize > 5 * 1024 * 1024) {
            m_streamingDecision = StreamingDecision::Streamed;
        } else {
            m_streamingDecision = StreamingDecision::PreDecoded;
        }
    }
    return loaded;
}

bool AudioBuffer::loadFromMemory(const void* data, std::size_t size) {
    m_detectedFormat = detectFormatFromHeader(data, size);
    bool loaded = m_buffer.loadFromMemory(data, size);
    if (loaded) {
        m_fileInfo.format = m_detectedFormat;
        m_fileInfo.duration = m_buffer.getDuration().asSeconds();
        m_fileInfo.sampleRate = m_buffer.getSampleRate();
        m_fileInfo.channelCount = m_buffer.getChannelCount();
        m_fileInfo.sampleCount = m_buffer.getSampleCount();
        m_fileInfo.fileSize = size;
        m_fileInfo.valid = true;
        m_rawMemoryUsage = size;
        m_streamingDecision = StreamingDecision::PreDecoded;
    }
    return loaded;
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

AudioFormat AudioBuffer::detectFormat(const std::string& filename) const {
    return detectFormatByExtension(filename);
}

AudioFormat AudioBuffer::detectFormatFromHeader(const void* data, std::size_t size) const {
    return detectFormatByMagic(static_cast<const uint8_t*>(data), size);
}

AudioFileInfo AudioBuffer::getFileInfo() const {
    return m_fileInfo;
}

StreamingDecision AudioBuffer::getStreamingDecision() const {
    return m_streamingDecision;
}

std::size_t AudioBuffer::getMemoryUsage() const {
    return m_rawMemoryUsage;
}

sf::SoundBuffer& AudioBuffer::getSFMLBuffer() {
    return m_buffer;
}

const sf::SoundBuffer& AudioBuffer::getSFMLBuffer() const {
    return m_buffer;
}

bool AudioBuffer::isFormatSupported(AudioFormat format) {
    return format == AudioFormat::WAV ||
           format == AudioFormat::OGG ||
           format == AudioFormat::MP3 ||
           format == AudioFormat::FLAC;
}

std::string AudioBuffer::formatToString(AudioFormat format) {
    switch (format) {
        case AudioFormat::WAV:   return "WAV";
        case AudioFormat::OGG:   return "OGG";
        case AudioFormat::MP3:   return "MP3";
        case AudioFormat::FLAC:  return "FLAC";
        case AudioFormat::Unknown:
        default:                 return "Unknown";
    }
}

std::size_t AudioBuffer::estimateMemoryUsage(const AudioFileInfo& info) {
    return info.sampleCount * sizeof(std::int16_t) * info.channelCount;
}

AudioFormat AudioBuffer::detectFormatByExtension(const std::string& filename) const {
    std::string ext;
    std::size_t dot = filename.rfind('.');
    if (dot != std::string::npos) {
        ext = filename.substr(dot);
        for (auto& c : ext) c = static_cast<char>(std::tolower(c));
    }

    if (ext == ".wav")  return AudioFormat::WAV;
    if (ext == ".ogg")  return AudioFormat::OGG;
    if (ext == ".mp3")  return AudioFormat::MP3;
    if (ext == ".flac") return AudioFormat::FLAC;
    return AudioFormat::Unknown;
}

AudioFormat AudioBuffer::detectFormatByMagic(const uint8_t* data, std::size_t size) const {
    if (size < 4) return AudioFormat::Unknown;

    if (std::memcmp(data, WAV_HEADER, 4) == 0) return AudioFormat::WAV;
    if (std::memcmp(data, OGG_HEADER, 4) == 0) return AudioFormat::OGG;
    if (size >= 3 && std::memcmp(data, MP3_ID3, 3) == 0) return AudioFormat::MP3;
    if (size >= 2 && std::memcmp(data, MP3_SYNC, 2) == 0) return AudioFormat::MP3;
    if (std::memcmp(data, FLAC_HEADER, 4) == 0) return AudioFormat::FLAC;

    return AudioFormat::Unknown;
}

std::shared_ptr<AudioBuffer> AudioBufferManager::loadBuffer(
    const std::string& name, const std::string& filepath) {
    auto buffer = std::make_shared<AudioBuffer>();
    if (buffer->loadFromFile(filepath)) {
        m_cache[name] = buffer;
        m_totalMemoryUsage += buffer->getMemoryUsage();
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
        m_totalMemoryUsage -= it->second->getMemoryUsage();
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
    return m_totalMemoryUsage;
}

std::size_t AudioBufferManager::getBufferCount() const {
    return m_cache.size();
}

std::vector<std::string> AudioBufferManager::getLoadedBufferNames() const {
    std::vector<std::string> names;
    names.reserve(m_cache.size());
    for (const auto& [name, buffer] : m_cache) {
        names.push_back(name);
    }
    return names;
}

bool AudioBufferManager::isBufferLoaded(const std::string& name) const {
    return m_cache.find(name) != m_cache.end();
}

} // namespace audio
} // namespace nebula
