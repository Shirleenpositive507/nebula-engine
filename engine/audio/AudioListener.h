#pragma once

#include <SFML/Audio.hpp>

namespace nebula {
namespace audio {

class AudioListener {
public:
    AudioListener() = default;
    ~AudioListener() = default;

    static void setPosition(float x, float y, float z);
    static void setDirection(float x, float y, float z);
    static void setUpVector(float x, float y, float z);
    static void setVolume(float volume);

    static float getPositionX();
    static float getPositionY();
    static float getPositionZ();

    static float getDirectionX();
    static float getDirectionY();
    static float getDirectionZ();

    static float getUpVectorX();
    static float getUpVectorY();
    static float getUpVectorZ();

    static float getGlobalVolume();

private:
    static sf::Vector3f m_position;
    static sf::Vector3f m_direction;
    static sf::Vector3f m_upVector;
    static float m_volume;
};

} // namespace audio
} // namespace nebula
