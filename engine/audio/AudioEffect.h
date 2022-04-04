#pragma once

#include <SFML/Audio.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace nebula {
namespace audio {

class AudioEffect {
public:
    enum class EffectType {
        Reverb,
        Echo,
        Flanger,
        Chorus,
        Distortion,
        LowPass,
        HighPass,
        PitchShift
    };

    struct Parameters {
        union {
            struct { float roomSize; float damping; float wetLevel; float dryLevel; float width; } reverb;
            struct { float delay; float decay; float wetDryMix; } echo;
            struct { float delay; float depth; float rate; float feedback; } flanger;
            struct { float delay; float depth; float rate; float feedback; } chorus;
            struct { float drive; float tone; float wetDryMix; } distortion;
            struct { float cutoffFrequency; float resonance; } lowPass;
            struct { float cutoffFrequency; float resonance; } highPass;
            struct { float shiftValue; } pitchShift;
        };
    };

    AudioEffect(EffectType type);
    ~AudioEffect() = default;

    EffectType getType() const;

    void apply(sf::SoundBuffer& buffer);
    void remove(sf::SoundBuffer& buffer);

    void setParameter(const std::string& name, float value);
    float getParameter(const std::string& name) const;

    void setWetDryMix(float mix);
    float getWetDryMix() const;

    void enable();
    void disable();
    bool isEnabled() const;

    std::string getTypeName() const;

private:
    EffectType m_type;
    Parameters m_params;
    float m_wetDryMix = 1.0f;
    bool m_enabled = true;

    void processReverb(std::vector<std::int16_t>& samples, unsigned int sampleRate, unsigned int channels);
    void processEcho(std::vector<std::int16_t>& samples, unsigned int sampleRate, unsigned int channels);
    void processFlanger(std::vector<std::int16_t>& samples, unsigned int sampleRate, unsigned int channels);
    void processChorus(std::vector<std::int16_t>& samples, unsigned int sampleRate, unsigned int channels);
    void processDistortion(std::vector<std::int16_t>& samples, unsigned int channels);
    void processLowPass(std::vector<std::int16_t>& samples, unsigned int sampleRate, unsigned int channels);
    void processHighPass(std::vector<std::int16_t>& samples, unsigned int sampleRate, unsigned int channels);
    void processPitchShift(std::vector<std::int16_t>& samples, unsigned int sampleRate, unsigned int channels);
};

class DSPChain {
public:
    DSPChain() = default;
    ~DSPChain() = default;

    void addEffect(std::shared_ptr<AudioEffect> effect);
    void removeEffect(std::size_t index);
    void clearEffects();
    std::shared_ptr<AudioEffect> getEffect(std::size_t index) const;
    std::size_t getEffectCount() const;

    void process(sf::SoundBuffer& buffer);

private:
    std::vector<std::shared_ptr<AudioEffect>> m_effects;
};

} // namespace audio
} // namespace nebula
