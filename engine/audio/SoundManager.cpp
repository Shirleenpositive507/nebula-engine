#include "SoundManager.h"
#include <algorithm>

namespace nebula {
namespace audio {

SoundManager& SoundManager::getInstance() {
    static SoundManager instance;
    return instance;
}

void SoundManager::init(std::size_t maxConcurrentSounds) {
    if (m_initialized) return;
    m_maxConcurrent = maxConcurrentSounds;
    m_pool.resize(m_maxConcurrent);
    for (auto& inst : m_pool) {
        inst.sound = std::make_unique<Sound>();
        inst.inUse = false;
    }
    m_groupVolumes[SoundGroup::SFX] = 1.f;
    m_groupVolumes[SoundGroup::Music] = 1.f;
    m_groupVolumes[SoundGroup::Voice] = 1.f;
    m_groupVolumes[SoundGroup::UI] = 1.f;
    m_initialized = true;
}

void SoundManager::shutdown() {
    stopAll();
    m_activeSounds.clear();
    m_pool.clear();
    m_bufferManager = AudioBufferManager();
    m_initialized = false;
}

std::shared_ptr<Sound> SoundManager::playSound(const std::string& name,
                                                float volume,
                                                float pitch,
                                                bool loop) {
    if (!m_initialized) return nullptr;

    auto buffer = m_bufferManager.getBuffer(name);
    if (!buffer) return nullptr;

    SoundInstance* instance = acquireInstance();
    if (!instance) return nullptr;

    instance->name = name;
    instance->inUse = true;
    instance->sound->setBuffer(
        std::make_shared<sf::SoundBuffer>(buffer->getSFMLBuffer()));
    instance->sound->setVolume(volume * m_globalVolume *
                               m_groupVolumes[instance->group] * 100.f);
    instance->sound->setPitch(pitch);
    instance->sound->setLoop(loop);
    instance->sound->play();

    m_activeSounds[name] = instance;
    return instance->sound;
}

void SoundManager::stopSound(const std::string& name) {
    auto it = m_activeSounds.find(name);
    if (it != m_activeSounds.end()) {
        it->second->sound->stop();
        releaseInstance(it->second);
        m_activeSounds.erase(it);
    }
}

void SoundManager::stopAll() {
    for (auto& [name, instance] : m_activeSounds) {
        instance->sound->stop();
        releaseInstance(instance);
    }
    m_activeSounds.clear();
}

void SoundManager::pauseAll() {
    for (auto& [name, instance] : m_activeSounds) {
        instance->sound->pause();
    }
}

void SoundManager::resumeAll() {
    for (auto& [name, instance] : m_activeSounds) {
        instance->sound->play();
    }
}

void SoundManager::setVolume(const std::string& name, float volume) {
    auto it = m_activeSounds.find(name);
    if (it != m_activeSounds.end()) {
        it->second->sound->setVolume(
            volume * m_globalVolume * m_groupVolumes[it->second->group] * 100.f);
    }
}

void SoundManager::setGlobalVolume(float volume) {
    m_globalVolume = std::clamp(volume, 0.f, 1.f);
    for (auto& [name, instance] : m_activeSounds) {
        float baseVol = instance->sound->getSFMLSound().getVolume();
        instance->sound->setVolume(
            baseVol * m_globalVolume * m_groupVolumes[instance->group]);
    }
}

void SoundManager::setGroupVolume(SoundGroup group, float volume) {
    m_groupVolumes[group] = std::clamp(volume, 0.f, 1.f);
    for (auto& [name, instance] : m_activeSounds) {
        if (instance->group == group) {
            float baseVol = instance->sound->getSFMLSound().getVolume();
            instance->sound->setVolume(
                baseVol * m_globalVolume * m_groupVolumes[group]);
        }
    }
}

std::shared_ptr<Sound> SoundManager::getSound(const std::string& name) const {
    auto it = m_activeSounds.find(name);
    if (it != m_activeSounds.end()) {
        return it->second->sound;
    }
    return nullptr;
}

bool SoundManager::soundExists(const std::string& name) const {
    return m_activeSounds.find(name) != m_activeSounds.end();
}

void SoundManager::releaseSound(const std::string& name) {
    stopSound(name);
}

void SoundManager::update(float dt) {
    for (auto it = m_activeSounds.begin(); it != m_activeSounds.end();) {
        auto* inst = it->second;
        inst->sound->updateFade(dt);
        if (inst->sound->getStatus() == Sound::Status::Stopped) {
            releaseInstance(inst);
            it = m_activeSounds.erase(it);
        } else {
            ++it;
        }
    }
}

AudioBufferManager& SoundManager::getBufferManager() {
    return m_bufferManager;
}

std::size_t SoundManager::getActiveSoundCount() const {
    return m_activeSounds.size();
}

std::size_t SoundManager::getMaxConcurrentSounds() const {
    return m_maxConcurrent;
}

SoundInstance* SoundManager::acquireInstance() {
    for (auto& inst : m_pool) {
        if (!inst.inUse) {
            return &inst;
        }
    }
    return nullptr;
}

void SoundManager::releaseInstance(SoundInstance* instance) {
    if (instance) {
        instance->inUse = false;
        instance->name.clear();
    }
}

} // namespace audio
} // namespace nebula
