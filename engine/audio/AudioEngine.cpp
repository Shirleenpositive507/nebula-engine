#include "AudioEngine.h"
#include <algorithm>
#include <cmath>

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

    MixerGroup master("Master");
    master.volume = 1.0f;
    m_mixerGroups["Master"] = master;

    MixerGroup sfx("SFX");
    sfx.volume = 1.0f;
    sfx.childGroups.push_back("Master");
    m_mixerGroups["SFX"] = sfx;

    MixerGroup music("Music");
    music.volume = 1.0f;
    music.childGroups.push_back("Master");
    m_mixerGroups["Music"] = music;

    MixerGroup voice("Voice");
    voice.volume = 1.0f;
    voice.childGroups.push_back("Master");
    m_mixerGroups["Voice"] = voice;

    MixerGroup ui("UI");
    ui.volume = 1.0f;
    ui.childGroups.push_back("Master");
    m_mixerGroups["UI"] = ui;

    m_initialized = true;
    return true;
}

void AudioEngine::shutdown() {
    if (!m_initialized) return;
    m_musicPlayer.stop();
    m_soundManager.shutdown();
    m_reverbZones.clear();
    m_mixerGroups.clear();
    m_duckingPairs.clear();
    m_initialized = false;
}

void AudioEngine::update(float dt) {
    if (!m_initialized) return;

    Vector3f listenerPos(
        AudioListener::getPosition().x,
        AudioListener::getPosition().y,
        AudioListener::getPosition().z
    );

    applyReverbZones(listenerPos);
    updateDucking(dt);

    m_soundManager.update(dt);
    m_musicPlayer.update(dt);

    float dopplerPitch = 1.0f;
    Vector3f listenerVel(
        AudioListener::getVelocity().x,
        AudioListener::getVelocity().y,
        AudioListener::getVelocity().z
    );
    float listenerSpeed = std::sqrt(
        listenerVel.x * listenerVel.x +
        listenerVel.y * listenerVel.y +
        listenerVel.z * listenerVel.z
    );
    if (listenerSpeed > 0.01f && m_dopplerFactor > 0.01f) {
        dopplerPitch = m_speedOfSound / (m_speedOfSound + listenerSpeed * m_dopplerFactor);
    }
    m_soundManager.setGlobalPitch(dopplerPitch);
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
    for (auto& [name, group] : m_mixerGroups) {
        if (name == "Master") {
            group.volume = volume;
        }
    }
}

float AudioEngine::getMasterVolume() const {
    return m_masterVolume;
}

void AudioEngine::setMute(bool mute) {
    m_muted = mute;
    AudioListener::setVolume(mute ? 0.f : m_masterVolume * 100.f);
    for (auto& [name, group] : m_mixerGroups) {
        group.mute = mute;
    }
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

void AudioEngine::setDopplerFactor(float factor) {
    m_dopplerFactor = std::max(0.0f, factor);
}

float AudioEngine::getDopplerFactor() const {
    return m_dopplerFactor;
}

void AudioEngine::setSpeedOfSound(float speed) {
    m_speedOfSound = std::max(0.1f, speed);
}

float AudioEngine::getSpeedOfSound() const {
    return m_speedOfSound;
}

void AudioEngine::addReverbZone(const ReverbZone& zone) {
    m_reverbZones.push_back(zone);
}

void AudioEngine::removeReverbZone(std::size_t index) {
    if (index < m_reverbZones.size()) {
        m_reverbZones.erase(m_reverbZones.begin() + static_cast<std::ptrdiff_t>(index));
    }
}

void AudioEngine::clearReverbZones() {
    m_reverbZones.clear();
}

std::size_t AudioEngine::getReverbZoneCount() const {
    return m_reverbZones.size();
}

ReverbZone& AudioEngine::getReverbZone(std::size_t index) {
    return m_reverbZones[index];
}

void AudioEngine::addMixerGroup(const MixerGroup& group) {
    m_mixerGroups[group.name] = group;
}

void AudioEngine::removeMixerGroup(const std::string& name) {
    m_mixerGroups.erase(name);
}

MixerGroup* AudioEngine::getMixerGroup(const std::string& name) {
    auto it = m_mixerGroups.find(name);
    if (it != m_mixerGroups.end()) {
        return &it->second;
    }
    return nullptr;
}

void AudioEngine::setMixerGroupVolume(const std::string& name, float volume) {
    auto it = m_mixerGroups.find(name);
    if (it != m_mixerGroups.end()) {
        it->second.volume = std::clamp(volume, 0.0f, 1.0f);
    }
}

float AudioEngine::getMixerGroupVolume(const std::string& name) const {
    auto it = m_mixerGroups.find(name);
    if (it != m_mixerGroups.end()) {
        return it->second.volume;
    }
    return 0.0f;
}

void AudioEngine::setAudioDucking(const std::string& triggerGroup, const std::string& targetGroup, float reduceLevel) {
    DuckingPair pair;
    pair.triggerGroup = triggerGroup;
    pair.targetGroup = targetGroup;
    pair.reduceLevel = std::clamp(reduceLevel, 0.0f, 1.0f);
    m_duckingPairs.push_back(pair);
}

void AudioEngine::removeAudioDucking(const std::string& triggerGroup, const std::string& targetGroup) {
    for (auto it = m_duckingPairs.begin(); it != m_duckingPairs.end();) {
        if (it->triggerGroup == triggerGroup && it->targetGroup == targetGroup) {
            it = m_duckingPairs.erase(it);
        } else {
            ++it;
        }
    }
}

void AudioEngine::updateDucking(float dt) {
    for (auto& pair : m_duckingPairs) {
        auto triggerIt = m_mixerGroups.find(pair.triggerGroup);
        if (triggerIt == m_mixerGroups.end()) continue;

        MixerGroup& triggerGroup = triggerIt->second;
        bool hasActiveSounds = triggerGroup.volume > 0.01f;

        if (hasActiveSounds && !pair.active) {
            pair.currentReduce = std::min(pair.currentReduce + dt / pair.attackTime, pair.reduceLevel);
            if (pair.currentReduce >= pair.reduceLevel) {
                pair.active = true;
            }
        } else if (!hasActiveSounds && pair.active) {
            pair.currentReduce = std::max(pair.currentReduce - dt / pair.releaseTime, 0.0f);
            if (pair.currentReduce <= 0.0f) {
                pair.active = false;
            }
        }

        auto targetIt = m_mixerGroups.find(pair.targetGroup);
        if (targetIt != m_mixerGroups.end()) {
            MixerGroup& targetGroup = targetIt->second;
            targetGroup.volume = 1.0f - pair.currentReduce;
        }
    }
}

void AudioEngine::addDuckingPair(const DuckingPair& pair) {
    m_duckingPairs.push_back(pair);
}

void AudioEngine::removeDuckingPair(std::size_t index) {
    if (index < m_duckingPairs.size()) {
        m_duckingPairs.erase(m_duckingPairs.begin() + static_cast<std::ptrdiff_t>(index));
    }
}

bool AudioEngine::isPointInReverbZone(const Vector3f& point, const ReverbZone& zone) const {
    switch (zone.shape) {
        case ReverbZoneShape::Sphere: {
            Vector3f diff = point - zone.position;
            float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
            float radius = zone.size.x;
            return distSq <= radius * radius;
        }
        case ReverbZoneShape::Box: {
            return std::abs(point.x - zone.position.x) <= zone.size.x * 0.5f &&
                   std::abs(point.y - zone.position.y) <= zone.size.y * 0.5f &&
                   std::abs(point.z - zone.position.z) <= zone.size.z * 0.5f;
        }
        case ReverbZoneShape::Capsule: {
            Vector3f start = zone.position - Vector3f(0, zone.size.y * 0.5f, 0);
            Vector3f end = zone.position + Vector3f(0, zone.size.y * 0.5f, 0);
            Vector3f seg = end - start;
            float segLenSq = seg.x * seg.x + seg.y * seg.y + seg.z * seg.z;
            if (segLenSq < 1e-8f) {
                Vector3f diff = point - zone.position;
                float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
                return distSq <= zone.size.x * zone.size.x;
            }
            float t = ((point - start).dot(seg)) / segLenSq;
            t = std::clamp(t, 0.0f, 1.0f);
            Vector3f closest = start + seg * t;
            Vector3f diff = point - closest;
            float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
            return distSq <= zone.size.x * zone.size.x;
        }
    }
    return false;
}

void AudioEngine::applyReverbZones(const Vector3f& listenerPos) {
    float totalWet = 0.0f;
    float totalDry = 0.0f;
    float totalRoom = 0.0f;
    float totalDamping = 0.0f;
    int activeZones = 0;

    for (const auto& zone : m_reverbZones) {
        if (!zone.enabled) continue;
        if (isPointInReverbZone(listenerPos, zone)) {
            totalWet += zone.wetLevel;
            totalDry += zone.dryLevel;
            totalRoom += zone.roomSize;
            totalDamping += zone.damping;
            activeZones++;
        }
    }

    if (activeZones > 0) {
        float invCount = 1.0f / static_cast<float>(activeZones);
        totalWet *= invCount;
        totalDry *= invCount;
        totalRoom *= invCount;
        totalDamping *= invCount;
    }
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
