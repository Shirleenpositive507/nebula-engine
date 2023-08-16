#include "SoundManager.h"
#include <algorithm>
#include <cmath>

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
        inst.priority = 0;
        inst.audibleRange = 100.0f;
        inst.is3D = false;
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

void SoundManager::preAllocateInstances(std::size_t count) {
    if (count > m_pool.size()) {
        std::size_t oldSize = m_pool.size();
        m_pool.resize(count);
        for (std::size_t i = oldSize; i < count; ++i) {
            m_pool[i].sound = std::make_unique<Sound>();
            m_pool[i].inUse = false;
            m_pool[i].priority = 0;
        }
        m_maxConcurrent = count;
    }
}

std::shared_ptr<Sound> SoundManager::playSound(const std::string& name,
                                                 float volume,
                                                 float pitch,
                                                 bool loop) {
    return playSound3D(name, Vector3f(0, 0, 0), volume, pitch, loop, 0);
}

std::shared_ptr<Sound> SoundManager::playSound3D(const std::string& name,
                                                   const Vector3f& position,
                                                   float volume,
                                                   float pitch,
                                                   bool loop,
                                                   int priority) {
    if (!m_initialized) return nullptr;

    auto buffer = m_bufferManager.getBuffer(name);
    if (!buffer) return nullptr;

    if (m_activeSounds.size() >= m_maxConcurrent) {
        cullLowPrioritySounds();
    }

    if (m_activeSounds.size() >= m_maxConcurrent) {
        auto* lowestPriority = acquireInstance();
        if (lowestPriority) {
            releaseInstance(lowestPriority);
        }
    }

    SoundInstance* instance = acquireInstance();
    if (!instance) return nullptr;

    instance->name = name;
    instance->inUse = true;
    instance->priority = priority;
    instance->audibleRange = 100.0f;
    instance->is3D = true;
    instance->position3D = position;
    instance->occlusion = OcclusionParams();
    instance->currentOcclusionTimer = 0.0f;

    Vector3f listenerPos(
        AudioListener::getPosition().x,
        AudioListener::getPosition().y,
        AudioListener::getPosition().z
    );
    Vector3f diff = position - listenerPos;
    float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);

    instance->occlusion = calculateOcclusion(position);

    float distanceAttenuation = 1.0f;
    if (distance > 0.01f && instance->audibleRange > 0.01f) {
        distanceAttenuation = std::max(0.0f, 1.0f - distance / instance->audibleRange);
    }

    instance->sound->setBuffer(
        std::make_shared<sf::SoundBuffer>(buffer->getSFMLBuffer()));
    instance->sound->setVolume(volume * m_globalVolume *
                               m_groupVolumes[instance->group] * 100.f *
                               distanceAttenuation *
                               instance->occlusion.volumeAttenuation);
    instance->sound->setPitch(pitch * m_globalPitch);
    instance->sound->setLoop(loop);
    instance->sound->play();

    m_activeSounds[name] = instance;
    return instance->sound;
}

void SoundManager::cullLowPrioritySounds() {
    if (m_activeSounds.size() < m_maxConcurrent) return;

    std::vector<std::pair<std::string, SoundInstance*>> sorted;
    sorted.reserve(m_activeSounds.size());
    for (auto& [name, inst] : m_activeSounds) {
        sorted.emplace_back(name, inst);
    }

    std::sort(sorted.begin(), sorted.end(),
        [](const auto& a, const auto& b) {
            return a.second->priority > b.second->priority;
        });

    while (sorted.size() >= m_maxConcurrent) {
        auto& [name, inst] = sorted.back();
        if (inst) {
            inst->sound->stop();
            releaseInstance(inst);
            m_activeSounds.erase(name);
        }
        sorted.pop_back();
    }
}

OcclusionParams SoundManager::calculateOcclusion(const Vector3f& soundPos) const {
    OcclusionParams params;
    Vector3f listenerPos(
        AudioListener::getPosition().x,
        AudioListener::getPosition().y,
        AudioListener::getPosition().z
    );
    Vector3f diff = soundPos - listenerPos;
    float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);

    if (distance < 0.1f) return params;

    float occlusion = 0.0f;
    for (const auto& [name, inst] : m_activeSounds) {
        if (!inst->inUse || inst->name.empty()) continue;
        Vector3f otherDiff = inst->position3D - listenerPos;
        float otherDist = std::sqrt(
            otherDiff.x * otherDiff.x +
            otherDiff.y * otherDiff.y +
            otherDiff.z * otherDiff.z
        );
        if (otherDist < 0.1f) continue;

        Vector3f dirToSound = diff / distance;
        Vector3f dirToOther = otherDiff / otherDist;
        float dot = dirToSound.x * dirToOther.x +
                    dirToSound.y * dirToOther.y +
                    dirToSound.z * dirToOther.z;

        if (dot > 0.8f && otherDist < distance) {
            float closeness = 1.0f - (otherDist / distance);
            occlusion = std::max(occlusion, closeness * 0.5f);
        }
    }

    params.occlusionFactor = occlusion;
    params.obstructionFactor = occlusion * 0.7f;
    params.lowPassCutoff = 22000.0f * (1.0f - occlusion * 0.8f);
    params.volumeAttenuation = 1.0f - occlusion * 0.4f;

    return params;
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
        if (instance && instance->sound) {
            instance->sound->stop();
        }
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

void SoundManager::setGlobalPitch(float pitch) {
    m_globalPitch = std::max(0.1f, pitch);
    for (auto& [name, instance] : m_activeSounds) {
        if (instance->inUse) {
            instance->sound->setPitch(m_globalPitch);
        }
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

void SoundManager::setSoundPriority(const std::string& name, int priority) {
    auto it = m_activeSounds.find(name);
    if (it != m_activeSounds.end()) {
        it->second->priority = priority;
    }
}

void SoundManager::setSoundAudibleRange(const std::string& name, float range) {
    auto it = m_activeSounds.find(name);
    if (it != m_activeSounds.end()) {
        it->second->audibleRange = std::max(0.1f, range);
    }
}

void SoundManager::setSoundPosition(const std::string& name, const Vector3f& position) {
    auto it = m_activeSounds.find(name);
    if (it != m_activeSounds.end()) {
        it->second->position3D = position;
        Vector3f listenerPos(
            AudioListener::getPosition().x,
            AudioListener::getPosition().y,
            AudioListener::getPosition().z
        );
        Vector3f diff = position - listenerPos;
        float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
        float distanceAttenuation = 1.0f;
        if (distance > 0.01f && it->second->audibleRange > 0.01f) {
            distanceAttenuation = std::max(0.0f, 1.0f - distance / it->second->audibleRange);
        }
        it->second->sound->setVolume(
            it->second->sound->getSFMLSound().getVolume() * distanceAttenuation);
    }
}

Vector3f SoundManager::getSoundPosition(const std::string& name) const {
    auto it = m_activeSounds.find(name);
    if (it != m_activeSounds.end()) {
        return it->second->position3D;
    }
    return Vector3f(0, 0, 0);
}

void SoundManager::setSoundOcclusion(const std::string& name, float occlusionFactor, float obstructionFactor) {
    auto it = m_activeSounds.find(name);
    if (it != m_activeSounds.end()) {
        it->second->occlusion.occlusionFactor = std::clamp(occlusionFactor, 0.0f, 1.0f);
        it->second->occlusion.obstructionFactor = std::clamp(obstructionFactor, 0.0f, 1.0f);
        it->second->occlusion.lowPassCutoff = 22000.0f * (1.0f - occlusionFactor * 0.8f);
        it->second->occlusion.volumeAttenuation = 1.0f - occlusionFactor * 0.4f;
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
        if (!inst || !inst->inUse) {
            it = m_activeSounds.erase(it);
            continue;
        }
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
        instance->priority = 0;
        instance->is3D = false;
        instance->occlusion = OcclusionParams();
    }
}

} // namespace audio
} // namespace nebula
