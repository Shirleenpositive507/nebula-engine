#include "MusicPlayer.h"
#include <algorithm>
#include <random>
#include <ctime>

namespace nebula {
namespace audio {

MusicPlayer::MusicPlayer() {
    m_music.setVolume(100.f);
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

bool MusicPlayer::openFromFile(const std::string& filename) {
    return m_music.openFromFile(filename);
}

void MusicPlayer::play() {
    m_music.play();
}

void MusicPlayer::pause() {
    m_music.pause();
}

void MusicPlayer::stop() {
    m_music.stop();
    m_fadingIn = false;
    m_fadingOut = false;
    m_crossfading = false;
}

void MusicPlayer::setLoop(bool loop) {
    m_music.setLoop(loop);
}

void MusicPlayer::setPitch(float pitch) {
    m_music.setPitch(pitch);
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
