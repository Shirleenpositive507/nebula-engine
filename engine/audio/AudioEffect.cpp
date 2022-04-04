#include "AudioEffect.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace nebula {
namespace audio {

AudioEffect::AudioEffect(EffectType type) : m_type(type) {
    std::memset(&m_params, 0, sizeof(m_params));
    switch (type) {
        case EffectType::Reverb:
            m_params.reverb = {0.5f, 0.5f, 0.8f, 0.2f, 1.0f};
            break;
        case EffectType::Echo:
            m_params.echo = {0.3f, 0.5f, 0.5f};
            break;
        case EffectType::Flanger:
            m_params.flanger = {0.005f, 0.002f, 0.5f, 0.3f};
            break;
        case EffectType::Chorus:
            m_params.chorus = {0.03f, 0.001f, 0.8f, 0.4f};
            break;
        case EffectType::Distortion:
            m_params.distortion = {1.0f, 0.5f, 0.7f};
            break;
        case EffectType::LowPass:
            m_params.lowPass = {8000.0f, 1.0f};
            break;
        case EffectType::HighPass:
            m_params.highPass = {100.0f, 1.0f};
            break;
        case EffectType::PitchShift:
            m_params.pitchShift = {1.0f};
            break;
    }
}

AudioEffect::EffectType AudioEffect::getType() const {
    return m_type;
}

void AudioEffect::apply(sf::SoundBuffer& buffer) {
    if (!m_enabled) return;

    std::vector<std::int16_t> samples(
        buffer.getSamples(),
        buffer.getSamples() + buffer.getSampleCount()
    );
    unsigned int sampleRate = buffer.getSampleRate();
    unsigned int channels = buffer.getChannelCount();

    switch (m_type) {
        case EffectType::Reverb:      processReverb(samples, sampleRate, channels); break;
        case EffectType::Echo:        processEcho(samples, sampleRate, channels); break;
        case EffectType::Flanger:     processFlanger(samples, sampleRate, channels); break;
        case EffectType::Chorus:      processChorus(samples, sampleRate, channels); break;
        case EffectType::Distortion:  processDistortion(samples, channels); break;
        case EffectType::LowPass:     processLowPass(samples, sampleRate, channels); break;
        case EffectType::HighPass:    processHighPass(samples, sampleRate, channels); break;
        case EffectType::PitchShift:  processPitchShift(samples, sampleRate, channels); break;
    }

    buffer.loadFromSamples(samples.data(), samples.size(), channels, sampleRate);
}

void AudioEffect::remove(sf::SoundBuffer& buffer) {
    (void)buffer;
}

void AudioEffect::setParameter(const std::string& name, float value) {
    if (name == "roomSize" && m_type == EffectType::Reverb)
        m_params.reverb.roomSize = std::clamp(value, 0.f, 1.f);
    else if (name == "damping" && m_type == EffectType::Reverb)
        m_params.reverb.damping = std::clamp(value, 0.f, 1.f);
    else if (name == "wetLevel" && m_type == EffectType::Reverb)
        m_params.reverb.wetLevel = std::clamp(value, 0.f, 1.f);
    else if (name == "dryLevel" && m_type == EffectType::Reverb)
        m_params.reverb.dryLevel = std::clamp(value, 0.f, 1.f);
    else if (name == "delay" && m_type == EffectType::Echo)
        m_params.echo.delay = std::max(0.001f, value);
    else if (name == "decay" && m_type == EffectType::Echo)
        m_params.echo.decay = std::clamp(value, 0.f, 1.f);
    else if (name == "depth" && (m_type == EffectType::Flanger || m_type == EffectType::Chorus))
        m_params.flanger.depth = std::clamp(value, 0.f, 0.01f);
    else if (name == "rate" && (m_type == EffectType::Flanger || m_type == EffectType::Chorus))
        m_params.flanger.rate = std::max(0.f, value);
    else if (name == "drive" && m_type == EffectType::Distortion)
        m_params.distortion.drive = std::max(0.f, value);
    else if (name == "cutoffFrequency" && (m_type == EffectType::LowPass || m_type == EffectType::HighPass))
        m_params.lowPass.cutoffFrequency = std::max(20.f, value);
    else if (name == "resonance" && (m_type == EffectType::LowPass || m_type == EffectType::HighPass))
        m_params.lowPass.resonance = std::max(0.1f, value);
    else if (name == "shiftValue" && m_type == EffectType::PitchShift)
        m_params.pitchShift.shiftValue = std::max(0.1f, value);
    else if (name == "feedback" && (m_type == EffectType::Flanger || m_type == EffectType::Chorus))
        m_params.flanger.feedback = std::clamp(value, 0.f, 0.99f);
}

float AudioEffect::getParameter(const std::string& name) const {
    if (name == "roomSize" && m_type == EffectType::Reverb) return m_params.reverb.roomSize;
    if (name == "damping" && m_type == EffectType::Reverb) return m_params.reverb.damping;
    if (name == "wetLevel" && m_type == EffectType::Reverb) return m_params.reverb.wetLevel;
    if (name == "dryLevel" && m_type == EffectType::Reverb) return m_params.reverb.dryLevel;
    if (name == "delay" && m_type == EffectType::Echo) return m_params.echo.delay;
    if (name == "decay" && m_type == EffectType::Echo) return m_params.echo.decay;
    if (name == "depth" && (m_type == EffectType::Flanger || m_type == EffectType::Chorus)) return m_params.flanger.depth;
    if (name == "rate" && (m_type == EffectType::Flanger || m_type == EffectType::Chorus)) return m_params.flanger.rate;
    if (name == "drive" && m_type == EffectType::Distortion) return m_params.distortion.drive;
    if (name == "cutoffFrequency" && (m_type == EffectType::LowPass || m_type == EffectType::HighPass)) return m_params.lowPass.cutoffFrequency;
    if (name == "resonance" && (m_type == EffectType::LowPass || m_type == EffectType::HighPass)) return m_params.lowPass.resonance;
    if (name == "shiftValue" && m_type == EffectType::PitchShift) return m_params.pitchShift.shiftValue;
    if (name == "feedback" && (m_type == EffectType::Flanger || m_type == EffectType::Chorus)) return m_params.flanger.feedback;
    return 0.f;
}

void AudioEffect::setWetDryMix(float mix) {
    m_wetDryMix = std::clamp(mix, 0.f, 1.f);
}

float AudioEffect::getWetDryMix() const {
    return m_wetDryMix;
}

void AudioEffect::enable() {
    m_enabled = true;
}

void AudioEffect::disable() {
    m_enabled = false;
}

bool AudioEffect::isEnabled() const {
    return m_enabled;
}

std::string AudioEffect::getTypeName() const {
    switch (m_type) {
        case EffectType::Reverb:     return "Reverb";
        case EffectType::Echo:       return "Echo";
        case EffectType::Flanger:    return "Flanger";
        case EffectType::Chorus:     return "Chorus";
        case EffectType::Distortion: return "Distortion";
        case EffectType::LowPass:    return "LowPass";
        case EffectType::HighPass:   return "HighPass";
        case EffectType::PitchShift: return "PitchShift";
    }
    return "Unknown";
}

void AudioEffect::processReverb(std::vector<std::int16_t>& samples,
                                unsigned int sampleRate, unsigned int channels) {
    std::vector<std::int16_t> output(samples.size());
    int delaySamples = static_cast<int>(sampleRate * 0.03f);
    for (std::size_t i = 0; i < samples.size(); ++i) {
        float wet = 0.f;
        if (i >= static_cast<std::size_t>(delaySamples)) {
            wet = samples[i - delaySamples] * m_params.reverb.wetLevel;
        }
        float dry = samples[i] * m_params.reverb.dryLevel;
        output[i] = static_cast<std::int16_t>(std::clamp(dry + wet, -32768.f, 32767.f));
    }
    samples = output;
}

void AudioEffect::processEcho(std::vector<std::int16_t>& samples,
                              unsigned int sampleRate, unsigned int channels) {
    std::vector<std::int16_t> output = samples;
    int delaySamples = static_cast<int>(sampleRate * m_params.echo.delay);
    for (std::size_t i = 0; i < samples.size(); ++i) {
        if (i >= static_cast<std::size_t>(delaySamples)) {
            float echo = output[i - delaySamples] * m_params.echo.decay;
            float mixed = samples[i] * (1.f - m_wetDryMix) + echo * m_wetDryMix;
            output[i] = static_cast<std::int16_t>(std::clamp(mixed, -32768.f, 32767.f));
        }
    }
    samples = output;
}

void AudioEffect::processFlanger(std::vector<std::int16_t>& samples,
                                 unsigned int sampleRate, unsigned int channels) {
    std::vector<std::int16_t> output(samples.size());
    for (std::size_t i = 0; i < samples.size(); ++i) {
        float phase = 2.f * 3.14159f * m_params.flanger.rate *
                      (static_cast<float>(i) / static_cast<float>(sampleRate));
        int offset = static_cast<int>(m_params.flanger.depth * sampleRate *
                      (0.5f + 0.5f * std::sin(phase)));
        float processed = samples[i];
        if (i >= static_cast<std::size_t>(offset) && offset > 0) {
            processed = samples[i] + samples[i - offset] * m_params.flanger.feedback;
        }
        processed = processed / (1.f + m_params.flanger.feedback);
        output[i] = static_cast<std::int16_t>(std::clamp(processed, -32768.f, 32767.f));
    }
    samples = output;
}

void AudioEffect::processChorus(std::vector<std::int16_t>& samples,
                                unsigned int sampleRate, unsigned int channels) {
    std::vector<std::int16_t> output(samples.size());
    for (std::size_t i = 0; i < samples.size(); ++i) {
        float phase = 2.f * 3.14159f * m_params.chorus.rate *
                      (static_cast<float>(i) / static_cast<float>(sampleRate));
        int offset = static_cast<int>(m_params.chorus.depth * sampleRate *
                      (0.5f + 0.5f * std::sin(phase)));
        float processed = samples[i];
        if (i >= static_cast<std::size_t>(offset) && offset > 0) {
            processed = samples[i] + samples[i - offset] * m_params.chorus.feedback;
        }
        processed = processed / (1.f + m_params.chorus.feedback);
        output[i] = static_cast<std::int16_t>(std::clamp(processed, -32768.f, 32767.f));
    }
    samples = output;
}

void AudioEffect::processDistortion(std::vector<std::int16_t>& samples,
                                    unsigned int channels) {
    for (auto& sample : samples) {
        float s = static_cast<float>(sample) / 32768.f;
        s *= m_params.distortion.drive;
        s = std::clamp(s, -1.f, 1.f);
        s = std::tanh(s * 2.f) / std::tanh(2.f);
        sample = static_cast<std::int16_t>(s * 32767.f);
    }
}

void AudioEffect::processLowPass(std::vector<std::int16_t>& samples,
                                 unsigned int sampleRate, unsigned int channels) {
    float rc = 1.f / (m_params.lowPass.cutoffFrequency * 2.f * 3.14159f);
    float dt = 1.f / static_cast<float>(sampleRate);
    float alpha = dt / (rc + dt);

    std::vector<std::int16_t> output(samples.size());
    for (std::size_t i = 0; i < samples.size(); ++i) {
        if (i == 0) {
            output[i] = samples[i];
        } else {
            float filtered = alpha * samples[i] + (1.f - alpha) * output[i - 1];
            output[i] = static_cast<std::int16_t>(std::clamp(filtered, -32768.f, 32767.f));
        }
    }
    samples = output;
}

void AudioEffect::processHighPass(std::vector<std::int16_t>& samples,
                                  unsigned int sampleRate, unsigned int channels) {
    float rc = 1.f / (m_params.highPass.cutoffFrequency * 2.f * 3.14159f);
    float dt = 1.f / static_cast<float>(sampleRate);
    float alpha = rc / (rc + dt);

    std::vector<std::int16_t> output(samples.size());
    std::int16_t prevIn = 0;
    std::int16_t prevOut = 0;
    for (std::size_t i = 0; i < samples.size(); ++i) {
        float filtered = alpha * (prevOut + samples[i] - prevIn);
        output[i] = static_cast<std::int16_t>(std::clamp(filtered, -32768.f, 32767.f));
        prevIn = samples[i];
        prevOut = output[i];
    }
    samples = output;
}

void AudioEffect::processPitchShift(std::vector<std::int16_t>& samples,
                                    unsigned int sampleRate, unsigned int channels) {
    (void)channels;
    float shift = m_params.pitchShift.shiftValue;
    if (std::abs(shift - 1.f) < 0.001f) return;

    std::size_t newSize = static_cast<std::size_t>(samples.size() / shift);
    std::vector<std::int16_t> output(newSize);
    for (std::size_t i = 0; i < newSize; ++i) {
        float srcIdx = static_cast<float>(i) * shift;
        std::size_t idx0 = static_cast<std::size_t>(srcIdx);
        std::size_t idx1 = std::min(idx0 + 1, samples.size() - 1);
        float frac = srcIdx - static_cast<float>(idx0);
        float s = samples[idx0] * (1.f - frac) + samples[idx1] * frac;
        output[i] = static_cast<std::int16_t>(std::clamp(s, -32768.f, 32767.f));
    }
    samples = output;
}

void DSPChain::addEffect(std::shared_ptr<AudioEffect> effect) {
    m_effects.push_back(effect);
}

void DSPChain::removeEffect(std::size_t index) {
    if (index < m_effects.size()) {
        m_effects.erase(m_effects.begin() + static_cast<std::ptrdiff_t>(index));
    }
}

void DSPChain::clearEffects() {
    m_effects.clear();
}

std::shared_ptr<AudioEffect> DSPChain::getEffect(std::size_t index) const {
    if (index < m_effects.size()) {
        return m_effects[index];
    }
    return nullptr;
}

std::size_t DSPChain::getEffectCount() const {
    return m_effects.size();
}

void DSPChain::process(sf::SoundBuffer& buffer) {
    for (auto& effect : m_effects) {
        effect->apply(buffer);
    }
}

} // namespace audio
} // namespace nebula
