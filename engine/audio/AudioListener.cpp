#include "AudioListener.h"

namespace nebula {
namespace audio {

sf::Vector3f AudioListener::m_position(0.f, 0.f, 0.f);
sf::Vector3f AudioListener::m_direction(0.f, 0.f, -1.f);
sf::Vector3f AudioListener::m_upVector(0.f, 1.f, 0.f);
float AudioListener::m_volume = 100.f;

void AudioListener::setPosition(float x, float y, float z) {
    m_position = sf::Vector3f(x, y, z);
    sf::Listener::setPosition(x, y, z);
}

void AudioListener::setDirection(float x, float y, float z) {
    m_direction = sf::Vector3f(x, y, z);
    sf::Listener::setDirection(x, y, z);
}

void AudioListener::setUpVector(float x, float y, float z) {
    m_upVector = sf::Vector3f(x, y, z);
    sf::Listener::setUpVector(x, y, z);
}

void AudioListener::setVolume(float volume) {
    m_volume = volume;
    sf::Listener::setGlobalVolume(volume);
}

float AudioListener::getPositionX() { return m_position.x; }
float AudioListener::getPositionY() { return m_position.y; }
float AudioListener::getPositionZ() { return m_position.z; }

float AudioListener::getDirectionX() { return m_direction.x; }
float AudioListener::getDirectionY() { return m_direction.y; }
float AudioListener::getDirectionZ() { return m_direction.z; }

float AudioListener::getUpVectorX() { return m_upVector.x; }
float AudioListener::getUpVectorY() { return m_upVector.y; }
float AudioListener::getUpVectorZ() { return m_upVector.z; }

float AudioListener::getGlobalVolume() { return m_volume; }

} // namespace audio
} // namespace nebula
