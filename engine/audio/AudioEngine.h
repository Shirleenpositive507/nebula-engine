#pragma once

#include "SoundManager.h"
#include "MusicPlayer.h"
#include "AudioListener.h"
#include "AudioStream.h"
#include "math/Vector3.h"

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace nebula {
namespace audio {

enum class DistanceModel {
    None,
    Inverse,
    InverseClamped,
    Linear,
    LinearClamped,
    Exponent,
    ExponentClamped
};

enum class ReverbZoneShape {
    Sphere,
    Box,
    Capsule
};

struct ReverbZone {
    ReverbZoneShape shape;
    Vector3f position;
    Vector3f size;
    float roomSize;
    float damping;
    float wetLevel;
    float dryLevel;
    float width;
    bool enabled;

    ReverbZone()
        : shape(ReverbZoneShape::Sphere)
        , position(0, 0, 0)
        , size(5, 5, 5)
        , roomSize(0.5f)
        , damping(0.5f)
        , wetLevel(0.8f)
        , dryLevel(0.2f)
        , width(1.0f)
        , enabled(true) {}
};

struct MixerGroup {
    std::string name;
    float volume;
    float pan;
    bool mute;
    std::vector<std::string> childGroups;

    MixerGroup()
        : name("")
        , volume(1.0f)
        , pan(0.0f)
        , mute(false) {}

    explicit MixerGroup(const std::string& n)
        : name(n)
        , volume(1.0f)
        , pan(0.0f)
        , mute(false) {}
};

struct DuckingPair {
    std::string triggerGroup;
    std::string targetGroup;
    float reduceLevel;
    float attackTime;
    float releaseTime;
    float currentReduce;
    bool active;

    DuckingPair()
        : reduceLevel(0.5f)
        , attackTime(0.1f)
        , releaseTime(0.3f)
        , currentReduce(0.0f)
        , active(false) {}
};

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    bool initialize();
    void shutdown();
    void update(float dt);

    SoundManager& getSoundManager();
    MusicPlayer& getMusicPlayer();

    void setMasterVolume(float volume);
    float getMasterVolume() const;

    void setMute(bool mute);
    bool isMuted() const;

    std::vector<std::string> getAvailableDevices() const;
    bool setAudioDevice(const std::string& deviceName);
    std::string getCurrentDevice() const;

    void setPauseOnFocusLost(bool enabled);
    bool getPauseOnFocusLost() const;

    void setDistanceModel(DistanceModel model);
    DistanceModel getDistanceModel() const;

    void setDopplerFactor(float factor);
    float getDopplerFactor() const;

    void setSpeedOfSound(float speed);
    float getSpeedOfSound() const;

    void addReverbZone(const ReverbZone& zone);
    void removeReverbZone(std::size_t index);
    void clearReverbZones();
    std::size_t getReverbZoneCount() const;
    ReverbZone& getReverbZone(std::size_t index);

    void addMixerGroup(const MixerGroup& group);
    void removeMixerGroup(const std::string& name);
    MixerGroup* getMixerGroup(const std::string& name);
    void setMixerGroupVolume(const std::string& name, float volume);
    float getMixerGroupVolume(const std::string& name) const;

    void setAudioDucking(const std::string& triggerGroup, const std::string& targetGroup, float reduceLevel);
    void removeAudioDucking(const std::string& triggerGroup, const std::string& targetGroup);
    void updateDucking(float dt);

    void addDuckingPair(const DuckingPair& pair);
    void removeDuckingPair(std::size_t index);

    AudioStream* createStream(const std::string& filepath);
    void destroyStream(AudioStream* stream);
    void stopAllStreams();

    struct AudioDeviceInfo {
        std::string name;
        std::string driver;
        unsigned int sampleRate;
        unsigned int channelCount;
        bool isDefault;
    };
    std::vector<AudioDeviceInfo> enumerateDevices() const;
    bool selectOutputDevice(const std::string& deviceName);
    std::string getSelectedDevice() const { return m_selectedDevice; }

    void onFocusGained();
    void onFocusLost();

private:
    bool isPointInReverbZone(const Vector3f& point, const ReverbZone& zone) const;
    void applyReverbZones(const Vector3f& listenerPos);

    SoundManager& m_soundManager;
    MusicPlayer m_musicPlayer;
    float m_masterVolume = 1.f;
    bool m_muted = false;
    bool m_pauseOnFocusLost = true;
    bool m_initialized = false;
    DistanceModel m_distanceModel = DistanceModel::InverseClamped;
    float m_dopplerFactor = 1.0f;
    float m_speedOfSound = 343.3f;

    std::vector<ReverbZone> m_reverbZones;
    std::unordered_map<std::string, MixerGroup> m_mixerGroups;
    std::vector<DuckingPair> m_duckingPairs;
    std::vector<std::unique_ptr<AudioStream>> m_streams;
    std::string m_selectedDevice;
};

} // namespace audio
} // namespace nebula
