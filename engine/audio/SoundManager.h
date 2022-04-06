#pragma once

#include "Sound.h"
#include "AudioBuffer.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace nebula {
namespace audio {

enum class SoundGroup {
    SFX,
    Music,
    Voice,
    UI
};

struct SoundInstance {
    std::unique_ptr<Sound> sound;
    std::string name;
    SoundGroup group;
    bool inUse = false;
};

class SoundManager {
public:
    static SoundManager& getInstance();

    void init(std::size_t maxConcurrentSounds = 64);
    void shutdown();

    std::shared_ptr<Sound> playSound(const std::string& name,
                                     float volume = 100.f,
                                     float pitch = 1.f,
                                     bool loop = false);

    void stopSound(const std::string& name);
    void stopAll();
    void pauseAll();
    void resumeAll();

    void setVolume(const std::string& name, float volume);
    void setGlobalVolume(float volume);
    void setGroupVolume(SoundGroup group, float volume);

    std::shared_ptr<Sound> getSound(const std::string& name) const;
    bool soundExists(const std::string& name) const;
    void releaseSound(const std::string& name);

    void update(float dt);

    AudioBufferManager& getBufferManager();

    std::size_t getActiveSoundCount() const;
    std::size_t getMaxConcurrentSounds() const;

private:
    SoundManager() = default;
    ~SoundManager() = default;
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    SoundInstance* acquireInstance();
    void releaseInstance(SoundInstance* instance);

    AudioBufferManager m_bufferManager;
    std::vector<SoundInstance> m_pool;
    std::unordered_map<std::string, SoundInstance*> m_activeSounds;
    std::unordered_map<SoundGroup, float> m_groupVolumes;
    float m_globalVolume = 1.f;
    std::size_t m_maxConcurrent = 64;
    bool m_initialized = false;
};

} // namespace audio
} // namespace nebula
