#include "AudioEngine.h"

namespace nebula {
namespace audio {

AudioEngine::AudioEngine()
    : m_soundManager(SoundManager::getInstance()) {
}

AudioEngine::~AudioEngine() {
    shutdown();
}

bool AudioEngine::initialize() {
    if (m_initialized) return true;

    m_soundManager.init(64);
    AudioListener::setPosition(0.f, 0.f, 0.f);
    AudioListener::setDirection(0.f, 0.f, -1.f);
    AudioListener::setUpVector(0.f, 1.f, 0.f);
    AudioListener::setVolume(100.f);

    setDistanceModel(m_distanceModel);

    m_initialized = true;
    return true;
}

void AudioEngine::shutdown() {
    if (!m_initialized) return;
    m_musicPlayer.stop();
    m_soundManager.shutdown();
    m_initialized = false;
}

void AudioEngine::update(float dt) {
    if (!m_initialized) return;
    m_soundManager.update(dt);
    m_musicPlayer.update(dt);
}

SoundManager& AudioEngine::getSoundManager() {
    return m_soundManager;
}

MusicPlayer& AudioEngine::getMusicPlayer() {
    return m_musicPlayer;
}

void AudioEngine::setMasterVolume(float volume) {
    m_masterVolume = volume;
    AudioListener::setVolume(m_muted ? 0.f : m_masterVolume * 100.f);
}

float AudioEngine::getMasterVolume() const {
    return m_masterVolume;
}

void AudioEngine::setMute(bool mute) {
    m_muted = mute;
    AudioListener::setVolume(mute ? 0.f : m_masterVolume * 100.f);
}

bool AudioEngine::isMuted() const {
    return m_muted;
}

std::vector<std::string> AudioEngine::getAvailableDevices() const {
    return sf::SoundRecorder::getAvailableDevices();
}

bool AudioEngine::setAudioDevice(const std::string& deviceName) {
    (void)deviceName;
    return true;
}

std::string AudioEngine::getCurrentDevice() const {
    return sf::SoundRecorder::getDefaultDevice();
}

void AudioEngine::setPauseOnFocusLost(bool enabled) {
    m_pauseOnFocusLost = enabled;
}

bool AudioEngine::getPauseOnFocusLost() const {
    return m_pauseOnFocusLost;
}

void AudioEngine::setDistanceModel(DistanceModel model) {
    m_distanceModel = model;
    sf::SoundSource::DistanceModel sfModel = sf::SoundSource::InverseClamped;
    switch (model) {
        case DistanceModel::None:             sfModel = sf::SoundSource::None; break;
        case DistanceModel::Inverse:          sfModel = sf::SoundSource::Inverse; break;
        case DistanceModel::InverseClamped:   sfModel = sf::SoundSource::InverseClamped; break;
        case DistanceModel::Linear:           sfModel = sf::SoundSource::Linear; break;
        case DistanceModel::LinearClamped:    sfModel = sf::SoundSource::LinearClamped; break;
        case DistanceModel::Exponent:         sfModel = sf::SoundSource::Exponent; break;
        case DistanceModel::ExponentClamped:  sfModel = sf::SoundSource::ExponentClamped; break;
    }
    sf::SoundSource::setDistanceModel(sfModel);
}

DistanceModel AudioEngine::getDistanceModel() const {
    return m_distanceModel;
}

void AudioEngine::onFocusGained() {
    if (m_pauseOnFocusLost) {
        m_soundManager.resumeAll();
        m_musicPlayer.play();
    }
}

void AudioEngine::onFocusLost() {
    if (m_pauseOnFocusLost) {
        m_soundManager.pauseAll();
        m_musicPlayer.pause();
    }
}

} // namespace audio
} // namespace nebula
