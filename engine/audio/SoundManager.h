#pragma once

#include "Sound.h"
#include "AudioBuffer.h"
#include "math/Vector3.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>

namespace nebula {
namespace audio {

enum class SoundGroup {
    SFX,
    Music,
    Voice,
    UI
};

struct OcclusionParams {
    float occlusionFactor;
    float obstructionFactor;
    float lowPassCutoff;
    float volumeAttenuation;

    OcclusionParams()
        : occlusionFactor(0.0f)
        , obstructionFactor(0.0f)
        , lowPassCutoff(22000.0f)
        , volumeAttenuation(1.0f) {}
};

struct SoundInstance {
    std::unique_ptr<Sound> sound;
    std::string name;
    SoundGroup group;
    bool inUse = false;
    int priority = 0;
    float audibleRange = 100.0f;
    Vector3f position3D;
    bool is3D = false;
    OcclusionParams occlusion;
    float currentOcclusionTimer = 0.0f;

    SoundInstance() : priority(0) {}
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

    std::shared_ptr<Sound> playSound3D(const std::string& name,
                                        const Vector3f& position,
                                        float volume = 100.f,
                                        float pitch = 1.f,
                                        bool loop = false,
                                        int priority = 0);

    void stopSound(const std::string& name);
    void stopAll();
    void pauseAll();
    void resumeAll();

    void setVolume(const std::string& name, float volume);
    void setGlobalVolume(float volume);
    void setGroupVolume(SoundGroup group, float volume);
    void setGlobalPitch(float pitch);

    void setSoundPriority(const std::string& name, int priority);
    void setSoundAudibleRange(const std::string& name, float range);
    void setSoundPosition(const std::string& name, const Vector3f& position);
    Vector3f getSoundPosition(const std::string& name) const;
    void setSoundOcclusion(const std::string& name, float occlusionFactor, float obstructionFactor);

    void preAllocateInstances(std::size_t count);

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
    void cullLowPrioritySounds();
    OcclusionParams calculateOcclusion(const Vector3f& soundPos) const;

    AudioBufferManager m_bufferManager;
    std::vector<SoundInstance> m_pool;
    std::unordered_map<std::string, SoundInstance*> m_activeSounds;
    std::unordered_map<SoundGroup, float> m_groupVolumes;
    float m_globalVolume = 1.f;
    float m_globalPitch = 1.f;
    std::size_t m_maxConcurrent = 64;
    bool m_initialized = false;
};

} // namespace audio
} // namespace nebula
