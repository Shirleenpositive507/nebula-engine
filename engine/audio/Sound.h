#pragma once

#include <SFML/Audio.hpp>
#include <memory>
#include <string>

namespace nebula {
namespace audio {

class Sound {
public:
    enum class Status {
        Stopped,
        Playing,
        Paused
    };

    Sound();
    ~Sound() = default;

    void setBuffer(std::shared_ptr<sf::SoundBuffer> buffer);
    void play();
    void pause();
    void stop();

    void setLoop(bool loop);
    void setPitch(float pitch);
    void setVolume(float volume);
    void setPosition(float x, float y, float z);
    void setRelativeToListener(bool relative);
    void setMinDistance(float distance);
    void setAttenuation(float attenuation);

    Status getStatus() const;
    float getPlayingOffset() const;
    void setPlayingOffset(float offset);
    bool isPlaying() const;
    float getDuration() const;

    void fadeIn(float duration);
    void fadeOut(float duration);
    void updateFade(float dt);

    sf::Sound& getSFMLSound();
    const sf::Sound& getSFMLSound() const;

private:
    sf::Sound m_sound;
    std::shared_ptr<sf::SoundBuffer> m_buffer;

    bool m_fadingIn = false;
    bool m_fadingOut = false;
    float m_fadeDuration = 0.f;
    float m_fadeTimer = 0.f;
    float m_fadeStartVolume = 0.f;
    float m_targetVolume = 100.f;
};

} // namespace audio
} // namespace nebula
