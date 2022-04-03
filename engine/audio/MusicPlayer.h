#pragma once

#include <SFML/Audio.hpp>
#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace nebula {
namespace audio {

enum class PlaylistMode {
    Normal,
    Repeat,
    Shuffle
};

class MusicPlayer {
public:
    MusicPlayer();
    ~MusicPlayer() = default;

    bool openFromFile(const std::string& filename);
    void play();
    void pause();
    void stop();

    void setLoop(bool loop);
    void setPitch(float pitch);
    void setVolume(float volume);
    void setPlayingOffset(float offset);
    float getPlayingOffset() const;

    sf::SoundSource::Status getStatus() const;
    float getDuration() const;

    void setFadeIn(float duration);
    void setFadeOut(float duration);

    void addToPlaylist(const std::string& filepath);
    void clearPlaylist();
    void next();
    void previous();
    void setPlaylistShuffle(bool shuffle);
    void setPlaylistRepeat(bool repeat);
    std::string getCurrentTrack() const;

    void update(float dt);

    sf::Music& getSFMLMusic();
    const sf::Music& getSFMLMusic() const;

private:
    void playTrack(std::size_t index);
    void applyFade(float dt);

    sf::Music m_music;
    std::vector<std::string> m_playlist;
    std::size_t m_currentTrack = 0;
    bool m_shuffle = false;
    bool m_repeat = false;

    bool m_fadingIn = false;
    bool m_fadingOut = false;
    bool m_crossfading = false;
    float m_fadeDuration = 0.f;
    float m_fadeTimer = 0.f;
    float m_fadeStartVolume = 0.f;
    float m_targetVolume = 100.f;
    float m_crossfadeThreshold = 0.f;

    sf::Music m_nextMusic;
    std::string m_currentTrackPath;
};

} // namespace audio
} // namespace nebula
