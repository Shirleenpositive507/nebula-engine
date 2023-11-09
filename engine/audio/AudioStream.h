#pragma once

#include <SFML/Audio/SoundStream.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
#include <functional>

namespace nebula {
namespace audio {

struct StreamBuffer {
    std::vector<int16_t> samples[2];
    std::size_t currentReadBuffer = 0;
    bool bufferReady[2] = { false, false };
};

class AudioStream : public sf::SoundStream {
public:
    AudioStream();
    ~AudioStream();

    bool openFromFile(const std::string& filename);
    bool openFromMemory(const std::vector<uint8_t>& data);
    bool openFromNetwork(const std::string& url);

    void play();
    void pause();
    void stop();

    bool seek(sf::Time position);
    sf::Time getDuration() const;

    void setStreamBufferSize(std::size_t sampleCount);
    std::size_t getStreamBufferSize() const;

    void setPreloadCallback(std::function<void()> callback);
    void setUnderflowCallback(std::function<void()> callback);

    bool isStreaming() const { return m_streaming; }
    bool hasUnderflowed() const { return m_underflowed; }

private:
    bool onGetData(Chunk& data) override;
    void onSeek(sf::Time timeOffset) override;

    void decodeThreadFunc();
    bool loadFromFile(const std::string& filename);
    bool loadFromMemory(const std::vector<uint8_t>& data);

    sf::SoundBuffer m_buffer;
    std::vector<int16_t> m_sampleData;

    StreamBuffer m_streamBuffer;
    std::size_t m_bufferSize;
    std::size_t m_sampleRate;
    unsigned int m_channelCount;

    std::thread m_decodeThread;
    std::mutex m_mutex;
    std::atomic<bool> m_streaming;
    std::atomic<bool> m_underflowed;
    std::atomic<bool> m_seekRequested;
    sf::Time m_seekTarget;

    std::vector<int16_t> m_tempBuffer;
    std::function<void()> m_preloadCallback;
    std::function<void()> m_underflowCallback;
};

}
}

