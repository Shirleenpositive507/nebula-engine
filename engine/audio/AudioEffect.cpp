#include "AudioEffect.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <complex>

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
        case EffectType::ConvolutionReverb:
            m_params.convolution = {0.5f, 1.0f};
            break;
        case EffectType::ParametricEQ:
            {
                EQBand defaultBand;
                defaultBand.type = EQBand::BandType::Peaking;
                defaultBand.frequency = 1000.0f;
                defaultBand.gain = 0.0f;
                defaultBand.Q = 0.707f;
                m_params.eq.bands.push_back(defaultBand);
            }
            break;
        case EffectType::Compressor:
            m_params.compressor = {-20.0f, 4.0f, 0.005f, 0.1f, 0.0f};
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
        case EffectType::Reverb:            processReverb(samples, sampleRate, channels); break;
        case EffectType::Echo:              processEcho(samples, sampleRate, channels); break;
        case EffectType::Flanger:           processFlanger(samples, sampleRate, channels); break;
        case EffectType::Chorus:            processChorus(samples, sampleRate, channels); break;
        case EffectType::Distortion:        processDistortion(samples, channels); break;
        case EffectType::LowPass:           processLowPass(samples, sampleRate, channels); break;
        case EffectType::HighPass:          processHighPass(samples, sampleRate, channels); break;
        case EffectType::PitchShift:        processPitchShift(samples, sampleRate, channels); break;
        case EffectType::ConvolutionReverb: processConvolutionReverb(samples, sampleRate, channels); break;
        case EffectType::ParametricEQ:      processParametricEQ(samples, sampleRate, channels); break;
        case EffectType::Compressor:        processCompressor(samples, sampleRate, channels); break;
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
    else if (name == "threshold" && m_type == EffectType::Compressor)
        m_params.compressor.threshold = value;
    else if (name == "ratio" && m_type == EffectType::Compressor)
        m_params.compressor.ratio = std::max(1.0f, value);
    else if (name == "attack" && m_type == EffectType::Compressor)
        m_params.compressor.attack = std::max(0.001f, value);
    else if (name == "release" && m_type == EffectType::Compressor)
        m_params.compressor.release = std::max(0.001f, value);
    else if (name == "makeupGain" && m_type == EffectType::Compressor)
        m_params.compressor.makeupGain = value;
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
    if (name == "threshold" && m_type == EffectType::Compressor) return m_params.compressor.threshold;
    if (name == "ratio" && m_type == EffectType::Compressor) return m_params.compressor.ratio;
    if (name == "attack" && m_type == EffectType::Compressor) return m_params.compressor.attack;
    if (name == "release" && m_type == EffectType::Compressor) return m_params.compressor.release;
    if (name == "makeupGain" && m_type == EffectType::Compressor) return m_params.compressor.makeupGain;
    return 0.f;
}

void AudioEffect::setEQBand(std::size_t index, const EQBand& band) {
    if (m_type == EffectType::ParametricEQ && index < m_params.eq.bands.size()) {
        m_params.eq.bands[index] = band;
    }
}

void AudioEffect::addEQBand(const EQBand& band) {
    if (m_type == EffectType::ParametricEQ) {
        m_params.eq.bands.push_back(band);
    }
}

void AudioEffect::removeEQBand(std::size_t index) {
    if (m_type == EffectType::ParametricEQ && index < m_params.eq.bands.size()) {
        m_params.eq.bands.erase(m_params.eq.bands.begin() + static_cast<std::ptrdiff_t>(index));
    }
}

std::size_t AudioEffect::getEQBandCount() const {
    if (m_type == EffectType::ParametricEQ) {
        return m_params.eq.bands.size();
    }
    return 0;
}

AudioEffect::EQBand AudioEffect::getEQBand(std::size_t index) const {
    if (m_type == EffectType::ParametricEQ && index < m_params.eq.bands.size()) {
        return m_params.eq.bands[index];
    }
    return EQBand();
}

bool AudioEffect::loadImpulseResponse(const std::string& filepath) {
    return m_impulseResponse.loadFromFile(filepath);
}

bool AudioEffect::loadImpulseResponseFromMemory(const void* data, std::size_t size) {
    return m_impulseResponse.loadFromMemory(data, size);
}

void AudioEffect::setThreshold(float threshold) {
    if (m_type == EffectType::Compressor) m_params.compressor.threshold = threshold;
}

void AudioEffect::setRatio(float ratio) {
    if (m_type == EffectType::Compressor) m_params.compressor.ratio = std::max(1.0f, ratio);
}

void AudioEffect::setAttack(float attack) {
    if (m_type == EffectType::Compressor) m_params.compressor.attack = std::max(0.001f, attack);
}

void AudioEffect::setRelease(float release) {
    if (m_type == EffectType::Compressor) m_params.compressor.release = std::max(0.001f, release);
}

void AudioEffect::setMakeupGain(float gain) {
    if (m_type == EffectType::Compressor) m_params.compressor.makeupGain = gain;
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
        case EffectType::Reverb:            return "Reverb";
        case EffectType::Echo:              return "Echo";
        case EffectType::Flanger:           return "Flanger";
        case EffectType::Chorus:            return "Chorus";
        case EffectType::Distortion:        return "Distortion";
        case EffectType::LowPass:           return "LowPass";
        case EffectType::HighPass:          return "HighPass";
        case EffectType::PitchShift:        return "PitchShift";
        case EffectType::ConvolutionReverb: return "ConvolutionReverb";
        case EffectType::ParametricEQ:      return "ParametricEQ";
        case EffectType::Compressor:        return "Compressor";
    }
    return "Unknown";
}

void AudioEffect::updateCompressorEnvelope(float& envelope, float input, float sampleRate, float attack, float release) {
    float absInput = std::abs(input);
    float attackCoeff = std::exp(-1.0f / (sampleRate * attack));
    float releaseCoeff = std::exp(-1.0f / (sampleRate * release));

    if (absInput > envelope) {
        envelope = attackCoeff * (envelope - absInput) + absInput;
    } else {
        envelope = releaseCoeff * (envelope - absInput) + absInput;
    }
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

void AudioEffect::processConvolutionReverb(std::vector<std::int16_t>& samples,
                                            unsigned int sampleRate, unsigned int channels) {
    if (m_impulseResponse.getSampleCount() == 0) return;

    const std::int16_t* irSamples = m_impulseResponse.getSamples();
    std::size_t irLen = m_impulseResponse.getSampleCount();
    unsigned int irChannels = m_impulseResponse.getChannelCount();

    std::vector<std::int16_t> output(samples.size(), 0);

    for (std::size_t i = 0; i < samples.size(); ++i) {
        for (std::size_t j = 0; j < irLen && (i + j) < samples.size(); ++j) {
            float irSample = static_cast<float>(irSamples[j * irChannels]) / 32768.0f;
            float inputSample = static_cast<float>(samples[i]) / 32768.0f;
            output[i + j] += static_cast<std::int16_t>(
                std::clamp(inputSample * irSample * 32768.0f * m_params.convolution.irGain, -32768.f, 32767.f)
            );
        }
    }

    for (std::size_t i = 0; i < samples.size(); ++i) {
        float dry = static_cast<float>(samples[i]) * (1.0f - m_params.convolution.wetMix);
        float wet = static_cast<float>(output[i]) * m_params.convolution.wetMix;
        samples[i] = static_cast<std::int16_t>(std::clamp(dry + wet, -32768.f, 32767.f));
    }
}

void AudioEffect::processParametricEQ(std::vector<std::int16_t>& samples,
                                       unsigned int sampleRate, unsigned int channels) {
    if (m_params.eq.bands.empty()) return;

    std::vector<double> dbuf(samples.size());
    for (std::size_t i = 0; i < samples.size(); ++i) {
        dbuf[i] = static_cast<double>(samples[i]) / 32768.0;
    }

    for (const auto& band : m_params.eq.bands) {
        double w0 = 2.0 * 3.14159265358979323846 * band.frequency / sampleRate;
        double alpha = std::sin(w0) / (2.0 * band.Q);
        double A = std::pow(10.0, band.gain / 40.0);
        double cosW0 = std::cos(w0);

        double b0, b1, b2, a0, a1, a2;

        switch (band.type) {
            case EQBand::BandType::LowShelf: {
                double sqrtA = std::sqrt(A);
                a0 = (A + 1.0) + (A - 1.0) * cosW0 + 2.0 * sqrtA * alpha;
                b0 = A * ((A + 1.0) - (A - 1.0) * cosW0 + 2.0 * sqrtA * alpha);
                b1 = 2.0 * A * ((A - 1.0) - (A + 1.0) * cosW0);
                b2 = A * ((A + 1.0) - (A - 1.0) * cosW0 - 2.0 * sqrtA * alpha);
                a1 = -2.0 * ((A - 1.0) + (A + 1.0) * cosW0);
                a2 = (A + 1.0) + (A - 1.0) * cosW0 - 2.0 * sqrtA * alpha;
                break;
            }
            case EQBand::BandType::HighShelf: {
                double sqrtA = std::sqrt(A);
                a0 = (A + 1.0) - (A - 1.0) * cosW0 + 2.0 * sqrtA * alpha;
                b0 = A * ((A + 1.0) + (A - 1.0) * cosW0 + 2.0 * sqrtA * alpha);
                b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cosW0);
                b2 = A * ((A + 1.0) + (A - 1.0) * cosW0 - 2.0 * sqrtA * alpha);
                a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cosW0);
                a2 = (A + 1.0) - (A - 1.0) * cosW0 - 2.0 * sqrtA * alpha;
                break;
            }
            case EQBand::BandType::Peaking: {
                a0 = 1.0 + alpha / A;
                b0 = 1.0 + alpha * A;
                b1 = -2.0 * cosW0;
                b2 = 1.0 - alpha * A;
                a1 = -2.0 * cosW0;
                a2 = 1.0 - alpha / A;
                break;
            }
            case EQBand::BandType::BandPass: {
                a0 = 1.0 + alpha;
                b0 = alpha;
                b1 = 0.0;
                b2 = -alpha;
                a1 = -2.0 * cosW0;
                a2 = 1.0 - alpha;
                break;
            }
            case EQBand::BandType::Notch: {
                a0 = 1.0 + alpha;
                b0 = 1.0;
                b1 = -2.0 * cosW0;
                b2 = 1.0;
                a1 = -2.0 * cosW0;
                a2 = 1.0 - alpha;
                break;
            }
        }

        double invA0 = 1.0 / a0;
        double x1 = 0, x2 = 0, y1 = 0, y2 = 0;

        for (std::size_t i = 0; i < dbuf.size(); ++i) {
            double x0 = dbuf[i];
            double y0 = (b0 * x0 + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2) * invA0;
            dbuf[i] = y0;
            x2 = x1; x1 = x0;
            y2 = y1; y1 = y0;
        }
    }

    for (std::size_t i = 0; i < samples.size(); ++i) {
        samples[i] = static_cast<std::int16_t>(std::clamp(dbuf[i] * 32768.0, -32768.0, 32767.0));
    }
}

void AudioEffect::processCompressor(std::vector<std::int16_t>& samples,
                                     unsigned int sampleRate, unsigned int channels) {
    float threshold = m_params.compressor.threshold;
    float ratio = m_params.compressor.ratio;
    float attack = m_params.compressor.attack;
    float release = m_params.compressor.release;
    float makeupGain = m_params.compressor.makeupGain;

    float thresholdLinear = std::pow(10.0f, threshold / 20.0f);
    float slope = 1.0f / ratio;

    float envelope = 0.0f;

    for (auto& sample : samples) {
        float input = static_cast<float>(sample) / 32768.0f;
        updateCompressorEnvelope(envelope, input, static_cast<float>(sampleRate), attack, release);

        float gainReduction = 1.0f;
        if (envelope > thresholdLinear) {
            float envDB = 20.0f * std::log10(envelope);
            float targetDB = threshold + (envDB - threshold) * slope;
            float targetLinear = std::pow(10.0f, targetDB / 20.0f);
            gainReduction = targetLinear / envelope;
        }

        float output = input * gainReduction * std::pow(10.0f, makeupGain / 20.0f);
        sample = static_cast<std::int16_t>(std::clamp(output * 32768.0f, -32768.f, 32767.f));
    }
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
