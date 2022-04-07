#pragma once

#include "SoundManager.h"
#include "MusicPlayer.h"
#include "AudioListener.h"

#include <memory>
#include <string>
#include <vector>

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

    void onFocusGained();
    void onFocusLost();

private:
    SoundManager& m_soundManager;
    MusicPlayer m_musicPlayer;
    float m_masterVolume = 1.f;
    bool m_muted = false;
    bool m_pauseOnFocusLost = true;
    bool m_initialized = false;
    DistanceModel m_distanceModel = DistanceModel::InverseClamped;
};

} // namespace audio
} // namespace nebula
