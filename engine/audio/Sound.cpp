#include "Sound.h"
#include <algorithm>
#include <cmath>

namespace nebula {
namespace audio {

Sound::Sound() {
    m_sound.setVolume(100.f);
}

void Sound::setBuffer(std::shared_ptr<sf::SoundBuffer> buffer) {
    m_buffer = buffer;
    m_sound.setBuffer(*buffer);
}

void Sound::play() {
    m_sound.play();
}

void Sound::pause() {
    m_sound.pause();
}

void Sound::stop() {
    m_sound.stop();
    m_fadingIn = false;
    m_fadingOut = false;
}

void Sound::setLoop(bool loop) {
    m_sound.setLoop(loop);
}

void Sound::setPitch(float pitch) {
    m_sound.setPitch(pitch);
}

void Sound::setVolume(float volume) {
    m_targetVolume = std::clamp(volume, 0.f, 100.f);
    m_sound.setVolume(m_targetVolume);
}

void Sound::setPosition(float x, float y, float z) {
    m_sound.setPosition(x, y, z);
}

void Sound::setRelativeToListener(bool relative) {
    m_sound.setRelativeToListener(relative);
}

void Sound::setMinDistance(float distance) {
    m_sound.setMinDistance(distance);
}

void Sound::setAttenuation(float attenuation) {
    m_sound.setAttenuation(attenuation);
}

Sound::Status Sound::getStatus() const {
    sf::SoundSource::Status status = m_sound.getStatus();
    switch (status) {
        case sf::SoundSource::Playing: return Status::Playing;
        case sf::SoundSource::Paused:  return Status::Paused;
        default:                       return Status::Stopped;
    }
}

float Sound::getPlayingOffset() const {
    return m_sound.getPlayingOffset().asSeconds();
}

void Sound::setPlayingOffset(float offset) {
    m_sound.setPlayingOffset(sf::seconds(offset));
}

bool Sound::isPlaying() const {
    return m_sound.getStatus() == sf::SoundSource::Playing;
}

float Sound::getDuration() const {
    if (m_buffer) {
        return m_buffer->getDuration().asSeconds();
    }
    return 0.f;
}

void Sound::fadeIn(float duration) {
    if (duration <= 0.f) return;
    m_fadingIn = true;
    m_fadingOut = false;
    m_fadeDuration = duration;
    m_fadeTimer = 0.f;
    m_fadeStartVolume = 0.f;
    m_sound.setVolume(0.f);
    play();
}

void Sound::fadeOut(float duration) {
    if (duration <= 0.f) return;
    m_fadingOut = true;
    m_fadingIn = false;
    m_fadeDuration = duration;
    m_fadeTimer = 0.f;
    m_fadeStartVolume = m_targetVolume;
}

void Sound::updateFade(float dt) {
    if (!m_fadingIn && !m_fadingOut) return;

    m_fadeTimer += dt;

    if (m_fadingIn) {
        float t = std::min(m_fadeTimer / m_fadeDuration, 1.f);
        float volume = m_fadeStartVolume + (m_targetVolume - m_fadeStartVolume) * t;
        m_sound.setVolume(volume);
        if (t >= 1.f) {
            m_fadingIn = false;
            m_sound.setVolume(m_targetVolume);
        }
    }

    if (m_fadingOut) {
        float t = std::min(m_fadeTimer / m_fadeDuration, 1.f);
        float volume = m_fadeStartVolume * (1.f - t);
        m_sound.setVolume(volume);
        if (t >= 1.f) {
            m_fadingOut = false;
            stop();
        }
    }
}

sf::Sound& Sound::getSFMLSound() {
    return m_sound;
}

const sf::Sound& Sound::getSFMLSound() const {
    return m_sound;
}

} // namespace audio
} // namespace nebula
