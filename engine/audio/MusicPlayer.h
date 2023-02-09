#pragma once

#include <SFML/Audio.hpp>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <cstdint>

namespace nebula {
namespace audio {

enum class PlaylistMode {
    Normal,
    Repeat,
    Shuffle
};

struct CuePoint {
    std::string name;
    float position;
    bool enabled;

    CuePoint() : position(0.0f), enabled(true) {}
    CuePoint(const std::string& n, float p) : name(n), position(p), enabled(true) {}
};

struct MusicLayer {
    std::string name;
    std::shared_ptr<sf::Music> music;
    float volume;
    float targetVolume;
    bool playing;
    bool loop;

    MusicLayer()
        : volume(0.0f)
        , targetVolume(0.0f)
        , playing(false)
        , loop(false) {}

    MusicLayer(const std::string& n, std::shared_ptr<sf::Music> m)
        : name(n)
        , music(m)
        , volume(0.0f)
        , targetVolume(0.0f)
        , playing(false)
        , loop(false) {}
};

struct StreamedMusic {
    std::vector<std::int16_t> samples;
    unsigned int sampleRate;
    unsigned int channelCount;
    std::size_t totalSamples;
    bool loaded;

    StreamedMusic()
        : sampleRate(44100)
        , channelCount(2)
        , totalSamples(0)
        , loaded(false) {}
};

class MusicPlayer {
public:
    MusicPlayer();
    ~MusicPlayer() = default;

    bool openFromFile(const std::string& filename);
    bool openFromMemory(const void* data, std::size_t size);
    void play();
    void pause();
    void stop();

    void setLoop(bool loop);
    void setPitch(float pitch);
    void setVolume(float volume);
    void setTempo(float tempo);
    float getTempo() const;
    void setPlayingOffset(float offset);
    float getPlayingOffset() const;

    sf::SoundSource::Status getStatus() const;
    float getDuration() const;

    void setFadeIn(float duration);
    void setFadeOut(float duration);

    void addCuePoint(const std::string& name, float position);
    void removeCuePoint(const std::string& name);
    void jumpToCue(const std::string& name);
    std::vector<CuePoint> getCuePoints() const;
    bool hasCuePoint(const std::string& name) const;

    void addLayer(const std::string& name, const std::string& filepath);
    void removeLayer(const std::string& name);
    void setLayerVolume(const std::string& name, float volume);
    void setLayerTargetVolume(const std::string& name, float volume);
    float getLayerVolume(const std::string& name) const;
    void playLayer(const std::string& name);
    void stopLayer(const std::string& name);
    void fadeLayerIn(const std::string& name, float duration);
    void fadeLayerOut(const std::string& name, float duration);

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
    void updateLayers(float dt);
    void updateStreaming(float dt);

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

    float m_tempo = 1.0f;
    float m_basePitch = 1.0f;

    sf::Music m_nextMusic;
    std::string m_currentTrackPath;

    std::vector<CuePoint> m_cuePoints;
    std::unordered_map<std::string, MusicLayer> m_layers;
    StreamedMusic m_streamedMusic;
    bool m_streamingFromMemory = false;
};

} // namespace audio
} // namespace nebula
