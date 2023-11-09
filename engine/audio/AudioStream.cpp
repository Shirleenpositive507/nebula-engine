#include "AudioStream.h"
#include <algorithm>
#include <cstring>

namespace nebula {
namespace audio {

AudioStream::AudioStream()
    : m_bufferSize(44100)
    , m_sampleRate(44100)
    , m_channelCount(2)
    , m_streaming(false)
    , m_underflowed(false)
    , m_seekRequested(false)
{
}

AudioStream::~AudioStream() {
    stop();
}

bool AudioStream::openFromFile(const std::string& filename) {
    if (!m_buffer.loadFromFile(filename)) {
        return false;
    }
    return loadFromFile(filename);
}

bool AudioStream::openFromMemory(const std::vector<uint8_t>& data) {
    if (!m_buffer.loadFromMemory(data.data(), data.size())) {
        return false;
    }
    return loadFromMemory(data);
}

bool AudioStream::openFromNetwork(const std::string& url) {
    (void)url;
    return false;
}

bool AudioStream::loadFromFile(const std::string& filename) {
    (void)filename;
    const sf::SoundBuffer* buf = &m_buffer;
    m_sampleRate = buf->getSampleRate();
    m_channelCount = buf->getChannelCount();
    const sf::Int16* samples = buf->getSamples();
    std::size_t count = buf->getSampleCount();
    m_sampleData.assign(samples, samples + count);
    initialize(m_channelCount, m_sampleRate);
    return true;
}

bool AudioStream::loadFromMemory(const std::vector<uint8_t>& data) {
    (void)data;
    const sf::SoundBuffer* buf = &m_buffer;
    m_sampleRate = buf->getSampleRate();
    m_channelCount = buf->getChannelCount();
    const sf::Int16* samples = buf->getSamples();
    std::size_t count = buf->getSampleCount();
    m_sampleData.assign(samples, samples + count);
    initialize(m_channelCount, m_sampleRate);
    return true;
}

void AudioStream::play() {
    if (m_sampleData.empty()) return;
    m_streaming = true;
    m_underflowed = false;
    sf::SoundStream::play();
}

void AudioStream::pause() {
    sf::SoundStream::pause();
}

void AudioStream::stop() {
    m_streaming = false;
    sf::SoundStream::stop();
    if (m_decodeThread.joinable()) {
        m_decodeThread.join();
    }
}

bool AudioStream::seek(sf::Time position) {
    if (m_streaming) {
        m_seekRequested = true;
        m_seekTarget = position;
    }
    return sf::SoundStream::setPlayingOffset(position);
}

sf::Time AudioStream::getDuration() const {
    return sf::seconds(static_cast<float>(m_sampleData.size()) / static_cast<float>(m_sampleRate * m_channelCount));
}

void AudioStream::setStreamBufferSize(std::size_t sampleCount) {
    m_bufferSize = sampleCount;
}

std::size_t AudioStream::getStreamBufferSize() const {
    return m_bufferSize;
}

void AudioStream::setPreloadCallback(std::function<void()> callback) {
    m_preloadCallback = std::move(callback);
}

void AudioStream::setUnderflowCallback(std::function<void()> callback) {
    m_underflowCallback = std::move(callback);
}

bool AudioStream::onGetData(Chunk& data) {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::size_t totalSamples = m_sampleData.size();
    std::size_t offset = static_cast<std::size_t>(getPlayingOffset().asSeconds() * m_sampleRate * m_channelCount);

    if (offset >= totalSamples) {
        m_streaming = false;
        return false;
    }

    std::size_t remaining = totalSamples - offset;
    std::size_t chunkSize = std::min(m_bufferSize, remaining);

    m_tempBuffer.resize(chunkSize);
    std::memcpy(m_tempBuffer.data(), m_sampleData.data() + offset, chunkSize * sizeof(int16_t));

    data.samples = m_tempBuffer.data();
    data.sampleCount = chunkSize;

    if (offset + chunkSize >= totalSamples) {
        return false;
    }

    return true;
}

void AudioStream::onSeek(sf::Time timeOffset) {
    (void)timeOffset;
}

void AudioStream::decodeThreadFunc() {
    while (m_streaming) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

}
}

