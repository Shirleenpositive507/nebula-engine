#include "MusicPlayer.h"
#include <algorithm>
#include <random>
#include <ctime>
#include <cstring>

namespace nebula {
namespace audio {

MusicPlayer::MusicPlayer() {
    m_music.setVolume(100.f);
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

bool MusicPlayer::openFromFile(const std::string& filename) {
    m_streamingFromMemory = false;
    bool result = m_music.openFromFile(filename);
    if (result) {
        m_currentTrackPath = filename;
    }
    return result;
}

bool MusicPlayer::openFromMemory(const void* data, std::size_t size) {
    m_streamingFromMemory = true;
    m_streamedMusic.samples.clear();
    m_streamedMusic.samples.resize(size / sizeof(std::int16_t));
    std::memcpy(m_streamedMusic.samples.data(), data, size);
    m_streamedMusic.totalSamples = m_streamedMusic.samples.size();
    m_streamedMusic.sampleRate = 44100;
    m_streamedMusic.channelCount = 2;
    m_streamedMusic.loaded = true;

    sf::SoundBuffer buffer;
    if (buffer.loadFromMemory(data, size)) {
        m_streamedMusic.sampleRate = buffer.getSampleRate();
        m_streamedMusic.channelCount = buffer.getChannelCount();
    }

    return m_music.openFromMemory(data, size);
}

void MusicPlayer::play() {
    m_music.play();
    for (auto& [name, layer] : m_layers) {
        if (layer.playing && layer.music) {
            layer.music->play();
        }
    }
}

void MusicPlayer::pause() {
    m_music.pause();
    for (auto& [name, layer] : m_layers) {
        if (layer.music) {
            layer.music->pause();
        }
    }
}

void MusicPlayer::stop() {
    m_music.stop();
    for (auto& [name, layer] : m_layers) {
        if (layer.music) {
            layer.music->stop();
            layer.playing = false;
        }
    }
    m_fadingIn = false;
    m_fadingOut = false;
    m_crossfading = false;
}

void MusicPlayer::setLoop(bool loop) {
    m_music.setLoop(loop);
}

void MusicPlayer::setPitch(float pitch) {
    m_basePitch = std::max(0.1f, pitch);
    m_music.setPitch(m_basePitch * m_tempo);
}

void MusicPlayer::setTempo(float tempo) {
    m_tempo = std::max(0.1f, tempo);
    m_music.setPitch(m_basePitch * m_tempo);
}

float MusicPlayer::getTempo() const {
    return m_tempo;
}

void MusicPlayer::setVolume(float volume) {
    m_targetVolume = std::clamp(volume, 0.f, 100.f);
    m_music.setVolume(m_targetVolume);
}

void MusicPlayer::setPlayingOffset(float offset) {
    m_music.setPlayingOffset(sf::seconds(offset));
}

float MusicPlayer::getPlayingOffset() const {
    return m_music.getPlayingOffset().asSeconds();
}

sf::SoundSource::Status MusicPlayer::getStatus() const {
    return m_music.getStatus();
}

float MusicPlayer::getDuration() const {
    return m_music.getDuration().asSeconds();
}

void MusicPlayer::setFadeIn(float duration) {
    m_fadingIn = true;
    m_fadingOut = false;
    m_fadeDuration = duration;
    m_fadeTimer = 0.f;
    m_fadeStartVolume = 0.f;
    m_music.setVolume(0.f);
}

void MusicPlayer::setFadeOut(float duration) {
    m_fadingOut = true;
    m_fadingIn = false;
    m_fadeDuration = duration;
    m_fadeTimer = 0.f;
    m_fadeStartVolume = m_targetVolume;
}

void MusicPlayer::addCuePoint(const std::string& name, float position) {
    for (auto& cue : m_cuePoints) {
        if (cue.name == name) {
            cue.position = position;
            return;
        }
    }
    m_cuePoints.emplace_back(name, position);
}

void MusicPlayer::removeCuePoint(const std::string& name) {
    for (auto it = m_cuePoints.begin(); it != m_cuePoints.end(); ++it) {
        if (it->name == name) {
            m_cuePoints.erase(it);
            return;
        }
    }
}

void MusicPlayer::jumpToCue(const std::string& name) {
    for (const auto& cue : m_cuePoints) {
        if (cue.name == name && cue.enabled) {
            setPlayingOffset(cue.position);
            return;
        }
    }
}

std::vector<CuePoint> MusicPlayer::getCuePoints() const {
    return m_cuePoints;
}

bool MusicPlayer::hasCuePoint(const std::string& name) const {
    for (const auto& cue : m_cuePoints) {
        if (cue.name == name) return true;
    }
    return false;
}

void MusicPlayer::addLayer(const std::string& name, const std::string& filepath) {
    auto music = std::make_shared<sf::Music>();
    if (music->openFromFile(filepath)) {
        MusicLayer layer(name, music);
        layer.volume = 0.0f;
        layer.targetVolume = 0.0f;
        layer.music->setVolume(0.0f);
        layer.music->setLoop(true);
        m_layers[name] = layer;
    }
}

void MusicPlayer::removeLayer(const std::string& name) {
    auto it = m_layers.find(name);
    if (it != m_layers.end()) {
        if (it->second.music) {
            it->second.music->stop();
        }
        m_layers.erase(it);
    }
}

void MusicPlayer::setLayerVolume(const std::string& name, float volume) {
    auto it = m_layers.find(name);
    if (it != m_layers.end()) {
        it->second.volume = std::clamp(volume, 0.0f, 100.0f);
        it->second.music->setVolume(it->second.volume);
    }
}

void MusicPlayer::setLayerTargetVolume(const std::string& name, float volume) {
    auto it = m_layers.find(name);
    if (it != m_layers.end()) {
        it->second.targetVolume = std::clamp(volume, 0.0f, 100.0f);
    }
}

float MusicPlayer::getLayerVolume(const std::string& name) const {
    auto it = m_layers.find(name);
    if (it != m_layers.end()) {
        return it->second.volume;
    }
    return 0.0f;
}

void MusicPlayer::playLayer(const std::string& name) {
    auto it = m_layers.find(name);
    if (it != m_layers.end() && it->second.music) {
        it->second.playing = true;
        it->second.music->setVolume(it->second.volume);
        it->second.music->play();
    }
}

void MusicPlayer::stopLayer(const std::string& name) {
    auto it = m_layers.find(name);
    if (it != m_layers.end() && it->second.music) {
        it->second.playing = false;
        it->second.music->stop();
    }
}

void MusicPlayer::fadeLayerIn(const std::string& name, float duration) {
    auto it = m_layers.find(name);
    if (it != m_layers.end()) {
        it->second.targetVolume = it->second.volume;
        it->second.volume = 0.0f;
        it->second.playing = true;
        if (it->second.music) {
            it->second.music->setVolume(0.0f);
            it->second.music->play();
        }
    }
}

void MusicPlayer::fadeLayerOut(const std::string& name, float duration) {
    auto it = m_layers.find(name);
    if (it != m_layers.end()) {
        it->second.targetVolume = 0.0f;
    }
}

void MusicPlayer::addToPlaylist(const std::string& filepath) {
    m_playlist.push_back(filepath);
}

void MusicPlayer::clearPlaylist() {
    m_playlist.clear();
    m_currentTrack = 0;
}

void MusicPlayer::next() {
    if (m_playlist.empty()) return;

    if (m_shuffle) {
        m_currentTrack = std::rand() % m_playlist.size();
    } else {
        m_currentTrack = (m_currentTrack + 1) % m_playlist.size();
    }
    playTrack(m_currentTrack);
}

void MusicPlayer::previous() {
    if (m_playlist.empty()) return;

    if (m_currentTrack == 0) {
        m_currentTrack = m_playlist.size() - 1;
    } else {
        m_currentTrack--;
    }
    playTrack(m_currentTrack);
}

void MusicPlayer::setPlaylistShuffle(bool shuffle) {
    m_shuffle = shuffle;
}

void MusicPlayer::setPlaylistRepeat(bool repeat) {
    m_repeat = repeat;
}

std::string MusicPlayer::getCurrentTrack() const {
    return m_currentTrackPath;
}

void MusicPlayer::update(float dt) {
    applyFade(dt);
    updateLayers(dt);
    updateStreaming(dt);

    if (!m_playlist.empty() && m_music.getStatus() == sf::SoundSource::Stopped) {
        if (m_repeat || m_shuffle) {
            next();
        }
    }

    if (m_crossfadeThreshold > 0.f && !m_crossfading) {
        float remaining = getDuration() - getPlayingOffset();
        if (remaining <= m_crossfadeThreshold && !m_playlist.empty()) {
            m_crossfading = true;
            setFadeOut(m_crossfadeThreshold);
            std::size_t nextIndex = m_shuffle
                ? std::rand() % m_playlist.size()
                : (m_currentTrack + 1) % m_playlist.size();
            m_nextMusic.openFromFile(m_playlist[nextIndex]);
            m_nextMusic.setVolume(0.f);
            m_nextMusic.play();
        }
    }
}

void MusicPlayer::updateLayers(float dt) {
    for (auto& [name, layer] : m_layers) {
        if (!layer.music) continue;

        if (std::abs(layer.volume - layer.targetVolume) > 0.5f) {
            float speed = 50.0f * dt;
            if (layer.volume < layer.targetVolume) {
                layer.volume = std::min(layer.volume + speed, layer.targetVolume);
            } else {
                layer.volume = std::max(layer.volume - speed, layer.targetVolume);
            }
            layer.music->setVolume(layer.volume);
        } else {
            layer.volume = layer.targetVolume;
        }

        if (layer.targetVolume <= 0.0f && layer.volume <= 0.0f && layer.playing) {
            layer.playing = false;
            layer.music->stop();
        }
    }
}

void MusicPlayer::updateStreaming(float dt) {
    if (!m_streamingFromMemory || !m_streamedMusic.loaded) return;
    (void)dt;
}

void MusicPlayer::playTrack(std::size_t index) {
    if (index >= m_playlist.size()) return;

    m_music.stop();
    m_currentTrackPath = m_playlist[index];
    if (m_music.openFromFile(m_playlist[index])) {
        m_music.setVolume(m_targetVolume);
        m_music.play();
    }
}

void MusicPlayer::applyFade(float dt) {
    if (!m_fadingIn && !m_fadingOut && !m_crossfading) return;

    m_fadeTimer += dt;

    if (m_fadingIn) {
        float t = std::min(m_fadeTimer / m_fadeDuration, 1.f);
        float volume = m_fadeStartVolume + (m_targetVolume - m_fadeStartVolume) * t;
        m_music.setVolume(volume);
        if (t >= 1.f) {
            m_fadingIn = false;
            m_music.setVolume(m_targetVolume);
        }
    }

    if (m_fadingOut) {
        float t = std::min(m_fadeTimer / m_fadeDuration, 1.f);
        float volume = m_fadeStartVolume * (1.f - t);
        m_music.setVolume(volume);

        if (m_crossfading && m_nextMusic.getStatus() == sf::SoundSource::Playing) {
            float nextVolume = m_targetVolume * t;
            m_nextMusic.setVolume(nextVolume);
        }

        if (t >= 1.f) {
            if (m_crossfading) {
                m_music.stop();
                std::swap(m_music, m_nextMusic);
                m_music.setVolume(m_targetVolume);
                m_crossfading = false;
                m_fadingIn = true;
                m_fadeTimer = 0.f;
                m_fadeStartVolume = m_targetVolume;
            } else {
                m_music.stop();
            }
            m_fadingOut = false;
        }
    }
}

sf::Music& MusicPlayer::getSFMLMusic() {
    return m_music;
}

const sf::Music& MusicPlayer::getSFMLMusic() const {
    return m_music;
}

} // namespace audio
} // namespace nebula
