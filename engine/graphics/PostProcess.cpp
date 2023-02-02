#include "PostProcess.h"
#include <algorithm>
#include <sstream>
#include <cmath>

namespace nebula {
    namespace graphics {

        PostProcess::PostProcess()
            : m_initialized(false)
            , m_toneMapOp(ToneMapOperator::None)
            , m_exposure(1.0f)
            , m_gamma(2.2f)
            , m_gammaCorrection(false) {}

        void PostProcess::addEffect(const std::string& name, EffectType type) {
            PostEffect effect;
            effect.name = name;
            effect.type = type;
            effect.enabled = true;
            effect.shader = std::make_shared<sf::Shader>();

            switch (type) {
                case EffectType::Blur:
                    effect.parameters["intensity"] = EffectParameter(1.f);
                    effect.parameters["direction"] = EffectParameter(sf::Vector2f(1.f, 0.f));
                    break;
                case EffectType::Glow:
                    effect.parameters["threshold"] = EffectParameter(0.5f);
                    effect.parameters["blurAmount"] = EffectParameter(3.f);
                    effect.parameters["intensity"] = EffectParameter(1.f);
                    break;
                case EffectType::ColorGrading:
                    effect.parameters["intensity"] = EffectParameter(1.f);
                    break;
                case EffectType::ChromaticAberration:
                    effect.parameters["strength"] = EffectParameter(0.005f);
                    break;
                case EffectType::Vignette:
                    effect.parameters["intensity"] = EffectParameter(0.5f);
                    effect.parameters["radius"] = EffectParameter(0.5f);
                    break;
                case EffectType::Bloom:
                    effect.parameters["threshold"] = EffectParameter(0.8f);
                    effect.parameters["blurAmount"] = EffectParameter(4.f);
                    effect.parameters["intensity"] = EffectParameter(1.f);
                    break;
                default:
                    break;
            }

            m_effects.push_back(effect);
        }

        void PostProcess::addEffect(const std::string& name, const std::shared_ptr<sf::Shader>& customShader) {
            PostEffect effect;
            effect.name = name;
            effect.type = EffectType::Custom;
            effect.enabled = true;
            effect.shader = customShader;
            m_effects.push_back(effect);
        }

        bool PostProcess::removeEffect(const std::string& name) {
            auto it = std::find_if(m_effects.begin(), m_effects.end(),
                [&](const PostEffect& e) { return e.name == name; });
            if (it == m_effects.end()) return false;
            m_effects.erase(it);
            return true;
        }

        void PostProcess::enable(const std::string& name) {
            for (auto& effect : m_effects) {
                if (effect.name == name) {
                    effect.enabled = true;
                    return;
                }
            }
        }

        void PostProcess::disable(const std::string& name) {
            for (auto& effect : m_effects) {
                if (effect.name == name) {
                    effect.enabled = false;
                    return;
                }
            }
        }

        void PostProcess::setActive(const std::string& name, bool active) {
            if (active) enable(name);
            else disable(name);
        }

        void PostProcess::render(sf::RenderTexture& input, sf::RenderTarget& output) {
            unsigned int width = input.getSize().x;
            unsigned int height = input.getSize().y;

            if (!m_initialized || m_ping.getSize().x != width || m_ping.getSize().y != height) {
                initializeTempTextures(width, height);
            }

            bool usePing = true;
            sf::RenderTexture* currentInput = &input;
            sf::RenderTexture* currentOutput = &m_ping;

            m_sprite.setTexture(input.getTexture());

            for (auto& effect : m_effects) {
                if (!effect.enabled) continue;

                if (!effect.shader || !effect.shader->isAvailable()) {
                    setupDefaultShaders();
                }

                currentOutput->clear(sf::Color::Transparent);

                sf::RenderStates states;
                states.shader = effect.shader.get();

                m_sprite.setTexture(currentInput->getTexture());
                currentOutput->draw(m_sprite, states);
                currentOutput->display();

                std::swap(currentInput, currentOutput);
                usePing = !usePing;
            }

            m_sprite.setTexture(currentInput->getTexture());
            output.draw(m_sprite);
        }

        void PostProcess::clearEffects() {
            m_effects.clear();
        }

        void PostProcess::setEffectParameter(const std::string& effectName, const std::string& paramName, const EffectParameter& value) {
            for (auto& effect : m_effects) {
                if (effect.name == effectName) {
                    effect.parameters[paramName] = value;
                    return;
                }
            }
        }

        EffectParameter PostProcess::getEffectParameter(const std::string& effectName, const std::string& paramName) const {
            for (const auto& effect : m_effects) {
                if (effect.name == effectName) {
                    auto it = effect.parameters.find(paramName);
                    if (it != effect.parameters.end()) {
                        return it->second;
                    }
                }
            }
            return EffectParameter();
        }

        void PostProcess::setBlur(float intensity, const sf::Vector2f& direction) {
            for (auto& effect : m_effects) {
                if (effect.type == EffectType::Blur) {
                    effect.parameters["intensity"] = EffectParameter(intensity);
                    effect.parameters["direction"] = EffectParameter(direction);
                }
            }
        }

        void PostProcess::setGlow(float threshold, float blurAmount, float intensity) {
            for (auto& effect : m_effects) {
                if (effect.type == EffectType::Glow) {
                    effect.parameters["threshold"] = EffectParameter(threshold);
                    effect.parameters["blurAmount"] = EffectParameter(blurAmount);
                    effect.parameters["intensity"] = EffectParameter(intensity);
                }
            }
        }

        void PostProcess::setColorGrading(const std::shared_ptr<sf::Texture>& lookupTable, float intensity) {
            for (auto& effect : m_effects) {
                if (effect.type == EffectType::ColorGrading) {
                    effect.parameters["lookupTexture"] = EffectParameter(lookupTable);
                    effect.parameters["intensity"] = EffectParameter(intensity);
                }
            }
        }

        void PostProcess::setChromaticAberration(float strength) {
            for (auto& effect : m_effects) {
                if (effect.type == EffectType::ChromaticAberration) {
                    effect.parameters["strength"] = EffectParameter(strength);
                }
            }
        }

        void PostProcess::setVignette(float intensity, const sf::Color& color, float radius) {
            for (auto& effect : m_effects) {
                if (effect.type == EffectType::Vignette) {
                    effect.parameters["intensity"] = EffectParameter(intensity);
                    effect.parameters["color"] = EffectParameter(sf::Glsl::Vec4(
                        color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f));
                    effect.parameters["radius"] = EffectParameter(radius);
                }
            }
        }

        void PostProcess::setBloom(float threshold, float blurAmount, float intensity) {
            for (auto& effect : m_effects) {
                if (effect.type == EffectType::Bloom) {
                    effect.parameters["threshold"] = EffectParameter(threshold);
                    effect.parameters["blurAmount"] = EffectParameter(blurAmount);
                    effect.parameters["intensity"] = EffectParameter(intensity);
                }
            }
        }

        PostEffect* PostProcess::getEffect(const std::string& name) {
            for (auto& effect : m_effects) {
                if (effect.name == name) return &effect;
            }
            return nullptr;
        }

        std::vector<std::string> PostProcess::getEffectNames() const {
            std::vector<std::string> names;
            names.reserve(m_effects.size());
            for (const auto& effect : m_effects) {
                names.push_back(effect.name);
            }
            return names;
        }

        bool PostProcess::isInitialized() const {
            return m_initialized;
        }

        void PostProcess::initializeTempTextures(unsigned int width, unsigned int height) {
            if (m_ping.getSize().x != width || m_ping.getSize().y != height) {
                m_ping.create(width, height);
            }
            if (m_pong.getSize().x != width || m_pong.getSize().y != height) {
                m_pong.create(width, height);
            }
            m_sprite.setTexture(m_ping.getTexture());
            m_initialized = true;
        }

        void PostProcess::applyEffect(const PostEffect& effect, sf::RenderTexture& input, sf::RenderTexture& output) {
            if (!effect.shader || !effect.shader->isAvailable()) return;

            sf::Sprite sprite(input.getTexture());
            sf::RenderStates states;
            states.shader = effect.shader.get();

            for (const auto& param : effect.parameters) {
                const std::string& name = param.first;
                const EffectParameter& value = param.second;

                switch (value.type) {
                    case EffectParameter::Float:
                        effect.shader->setUniform(name, value.floatValue);
                        break;
                    case EffectParameter::Vec2:
                        effect.shader->setUniform(name, value.vec2Value);
                        break;
                    case EffectParameter::Vec3:
                        effect.shader->setUniform(name, value.vec3Value);
                        break;
                    case EffectParameter::Vec4:
                        effect.shader->setUniform(name, value.vec4Value);
                        break;
                    case EffectParameter::Int:
                        effect.shader->setUniform(name, value.intValue);
                        break;
                    case EffectParameter::Texture:
                        if (value.textureValue) {
                            effect.shader->setUniform(name, *value.textureValue);
                        }
                        break;
                }
            }

            output.draw(sprite, states);
            output.display();
        }

        void PostProcess::applyBlur(sf::RenderTexture& input, sf::RenderTexture& output, float intensity, const sf::Vector2f& direction) {
            sf::Shader shader;
            if (!generateBlurShader(shader, intensity, direction)) return;

            shader.setUniform("texture", input.getTexture());
            shader.setUniform("intensity", intensity);
            shader.setUniform("direction", direction);
            shader.setUniform("texelSize", sf::Vector2f(
                1.f / static_cast<float>(input.getSize().x),
                1.f / static_cast<float>(input.getSize().y)));

            sf::Sprite sprite(input.getTexture());
            sf::RenderStates states;
            states.shader = &shader;
            output.draw(sprite, states);
            output.display();
        }

        void PostProcess::setupDefaultShaders() {
            for (auto& effect : m_effects) {
                if (!effect.shader) {
                    effect.shader = std::make_shared<sf::Shader>();
                }

                switch (effect.type) {
                    case EffectType::Blur: {
                        float intensity = effect.parameters["intensity"].floatValue;
                        sf::Vector2f dir = effect.parameters["direction"].vec2Value;
                        generateBlurShader(*effect.shader, intensity, dir);
                        break;
                    }
                    case EffectType::Glow: {
                        float threshold = effect.parameters["threshold"].floatValue;
                        float intensity = effect.parameters["intensity"].floatValue;
                        generateGlowShader(*effect.shader, threshold, intensity);
                        break;
                    }
                    case EffectType::ChromaticAberration: {
                        float strength = effect.parameters["strength"].floatValue;
                        generateChromaticAberrationShader(*effect.shader, strength);
                        break;
                    }
                    case EffectType::Vignette: {
                        float intensity = effect.parameters["intensity"].floatValue;
                        float radius = effect.parameters["radius"].floatValue;
                        generateVignetteShader(*effect.shader, intensity, sf::Color::Black, radius);
                        break;
                    }
                    case EffectType::Bloom: {
                        float threshold = effect.parameters["threshold"].floatValue;
                        float intensity = effect.parameters["intensity"].floatValue;
                        generateBloomShader(*effect.shader, threshold, intensity);
                        break;
                    }
                    default:
                        break;
                }
            }
        }

        bool PostProcess::generateBlurShader(sf::Shader& shader, float intensity, const sf::Vector2f& direction) {
            std::string fragment;
            fragment += "uniform sampler2D texture;\n";
            fragment += "uniform vec2 direction;\n";
            fragment += "uniform float intensity;\n";
            fragment += "uniform vec2 texelSize;\n";
            fragment += "void main() {\n";
            fragment += "    vec2 uv = gl_TexCoord[0].xy;\n";
            fragment += "    vec4 color = vec4(0.0);\n";
            fragment += "    float weights[9];\n";
            fragment += "    weights[0] = 0.05; weights[1] = 0.09; weights[2] = 0.12;\n";
            fragment += "    weights[3] = 0.15; weights[4] = 0.18; weights[5] = 0.15;\n";
            fragment += "    weights[6] = 0.12; weights[7] = 0.09; weights[8] = 0.05;\n";
            fragment += "    for (int i = -4; i <= 4; i++) {\n";
            fragment += "        vec2 offset = vec2(float(i)) * direction * texelSize * intensity;\n";
            fragment += "        color += texture2D(texture, uv + offset) * weights[i + 4];\n";
            fragment += "    }\n";
            fragment += "    gl_FragColor = color;\n";
            fragment += "}\n";
            return shader.loadFromMemory(fragment, sf::Shader::Fragment);
        }

        bool PostProcess::generateGlowShader(sf::Shader& shader, float threshold, float intensity) {
            std::string fragment;
            fragment += "uniform sampler2D texture;\n";
            fragment += "uniform float threshold;\n";
            fragment += "uniform float intensity;\n";
            fragment += "void main() {\n";
            fragment += "    vec4 color = texture2D(texture, gl_TexCoord[0].xy);\n";
            fragment += "    float luminance = dot(color.rgb, vec3(0.299, 0.587, 0.114));\n";
            fragment += "    if (luminance > threshold) {\n";
            fragment += "        gl_FragColor = color * intensity;\n";
            fragment += "    } else {\n";
            fragment += "        gl_FragColor = vec4(0.0);\n";
            fragment += "    }\n";
            fragment += "}\n";
            return shader.loadFromMemory(fragment, sf::Shader::Fragment);
        }

        bool PostProcess::generateChromaticAberrationShader(sf::Shader& shader, float strength) {
            std::string fragment;
            fragment += "uniform sampler2D texture;\n";
            fragment += "uniform float strength;\n";
            fragment += "void main() {\n";
            fragment += "    vec2 uv = gl_TexCoord[0].xy;\n";
            fragment += "    float r = texture2D(texture, uv + vec2(strength, 0.0)).r;\n";
            fragment += "    float g = texture2D(texture, uv).g;\n";
            fragment += "    float b = texture2D(texture, uv - vec2(strength, 0.0)).b;\n";
            fragment += "    gl_FragColor = vec4(r, g, b, 1.0);\n";
            fragment += "}\n";
            return shader.loadFromMemory(fragment, sf::Shader::Fragment);
        }

        bool PostProcess::generateVignetteShader(sf::Shader& shader, float intensity, const sf::Color& color, float radius) {
            std::string fragment;
            fragment += "uniform sampler2D texture;\n";
            fragment += "uniform float intensity;\n";
            fragment += "uniform vec4 color;\n";
            fragment += "uniform float radius;\n";
            fragment += "void main() {\n";
            fragment += "    vec2 uv = gl_TexCoord[0].xy;\n";
            fragment += "    vec2 center = vec2(0.5, 0.5);\n";
            fragment += "    float dist = distance(uv, center);\n";
            fragment += "    float vignette = 1.0 - smoothstep(radius, 1.0, dist * 1.414);\n";
            fragment += "    vec4 texColor = texture2D(texture, uv);\n";
            fragment += "    gl_FragColor = mix(texColor, color, (1.0 - vignette) * intensity);\n";
            fragment += "}\n";
            return shader.loadFromMemory(fragment, sf::Shader::Fragment);
        }

        bool PostProcess::generateBloomShader(sf::Shader& shader, float threshold, float intensity) {
            std::string fragment;
            fragment += "uniform sampler2D texture;\n";
            fragment += "uniform float threshold;\n";
            fragment += "uniform float intensity;\n";
            fragment += "void main() {\n";
            fragment += "    vec4 color = texture2D(texture, gl_TexCoord[0].xy);\n";
            fragment += "    float luminance = dot(color.rgb, vec3(0.299, 0.587, 0.114));\n";
            fragment += "    float amount = max(0.0, luminance - threshold) / (1.0 - threshold);\n";
            fragment += "    gl_FragColor = vec4(color.rgb * amount * intensity, color.a);\n";
            fragment += "}\n";
            return shader.loadFromMemory(fragment, sf::Shader::Fragment);
        }

        // --- Effect chain reordering ---

        void PostProcess::moveEffectUp(const std::string& name) {
            for (std::size_t i = 1; i < m_effects.size(); ++i) {
                if (m_effects[i].name == name) {
                    std::swap(m_effects[i], m_effects[i - 1]);
                    return;
                }
            }
        }

        void PostProcess::moveEffectDown(const std::string& name) {
            for (std::size_t i = 0; i + 1 < m_effects.size(); ++i) {
                if (m_effects[i].name == name) {
                    std::swap(m_effects[i], m_effects[i + 1]);
                    return;
                }
            }
        }

        void PostProcess::moveEffectTo(const std::string& name, std::size_t position) {
            if (position >= m_effects.size()) position = m_effects.size() - 1;
            for (std::size_t i = 0; i < m_effects.size(); ++i) {
                if (m_effects[i].name == name) {
                    PostEffect effect = m_effects[i];
                    m_effects.erase(m_effects.begin() + static_cast<ptrdiff_t>(i));
                    m_effects.insert(m_effects.begin() + static_cast<ptrdiff_t>(position), effect);
                    return;
                }
            }
        }

        // --- Enable/Disable ---

        bool PostProcess::isEnabled(const std::string& name) const {
            for (const auto& effect : m_effects) {
                if (effect.name == name) return effect.enabled;
            }
            return false;
        }

        // --- Tone Mapping ---

        void PostProcess::setToneMapping(ToneMapOperator op) {
            m_toneMapOp = op;
            if (op != ToneMapOperator::None) {
                std::string effectName = "_toneMap";
                bool found = false;
                for (auto& e : m_effects) {
                    if (e.name == effectName) { found = true; break; }
                }
                if (!found) {
                    addEffect(effectName, EffectType::ToneMapReinhard);
                    auto* effect = getEffect(effectName);
                    if (effect) effect->shader = std::make_shared<sf::Shader>();
                }
                auto* effect = getEffect(effectName);
                if (effect) {
                    sf::Shader s;
                    generateToneMapShader(s, op, m_exposure);
                    *effect->shader = s;
                }
            }
        }

        ToneMapOperator PostProcess::getToneMapping() const {
            return m_toneMapOp;
        }

        void PostProcess::setExposure(float exposure) {
            m_exposure = exposure;
        }

        float PostProcess::getExposure() const {
            return m_exposure;
        }

        // --- Gamma Correction ---

        void PostProcess::setGamma(float gamma) {
            m_gamma = gamma;
            if (m_gammaCorrection) {
                std::string effectName = "_gamma";
                auto* effect = getEffect(effectName);
                if (effect) {
                    sf::Shader s;
                    generateGammaShader(s, m_gamma);
                    *effect->shader = s;
                }
            }
        }

        float PostProcess::getGamma() const {
            return m_gamma;
        }

        void PostProcess::setGammaCorrectionEnabled(bool enabled) {
            m_gammaCorrection = enabled;
            std::string effectName = "_gamma";
            if (enabled) {
                bool found = false;
                for (auto& e : m_effects) {
                    if (e.name == effectName) { found = true; break; }
                }
                if (!found) {
                    addEffect(effectName, EffectType::GammaCorrection);
                    auto* effect = getEffect(effectName);
                    if (effect) {
                        effect->shader = std::make_shared<sf::Shader>();
                        sf::Shader s;
                        generateGammaShader(s, m_gamma);
                        *effect->shader = s;
                    }
                }
            } else {
                removeEffect(effectName);
            }
        }

        bool PostProcess::isGammaCorrectionEnabled() const {
            return m_gammaCorrection;
        }

        // --- Full-screen quad ---

        sf::VertexArray PostProcess::createFullScreenQuad() {
            sf::VertexArray quad(sf::PrimitiveType::TriangleStrip, 4);
            quad[0] = sf::Vertex(sf::Vector2f(-1.f, -1.f), sf::Vector2f(0.f, 1.f));
            quad[1] = sf::Vertex(sf::Vector2f(-1.f, 1.f), sf::Vector2f(0.f, 0.f));
            quad[2] = sf::Vertex(sf::Vector2f(1.f, -1.f), sf::Vector2f(1.f, 1.f));
            quad[3] = sf::Vertex(sf::Vector2f(1.f, 1.f), sf::Vector2f(1.f, 0.f));
            return quad;
        }

        void PostProcess::renderFullScreenQuad(sf::RenderTarget& target, sf::Shader* shader) {
            sf::RenderStates states;
            if (shader) states.shader = shader;
            static sf::VertexArray quad = createFullScreenQuad();
            target.draw(quad, states);
        }

        // --- Shader generators ---

        bool PostProcess::generateToneMapShader(sf::Shader& shader, ToneMapOperator op, float exposure) {
            std::string fragment;
            fragment += "uniform sampler2D texture;\n";
            fragment += "uniform float exposure;\n";
            fragment += "void main() {\n";
            fragment += "    vec4 color = texture2D(texture, gl_TexCoord[0].xy);\n";
            fragment += "    vec3 hdr = color.rgb * exposure;\n";
            switch (op) {
                case ToneMapOperator::Reinhard:
                    fragment += "    vec3 mapped = hdr / (hdr + vec3(1.0));\n";
                    break;
                case ToneMapOperator::ACES:
                    fragment += "    vec3 aces = hdr * (hdr * 2.51 + vec3(0.03));\n";
                    fragment += "    vec3 mapped = aces / (aces * (hdr * 2.43 + vec3(0.59)) + vec3(0.14));\n";
                    break;
                case ToneMapOperator::Filmic:
                    fragment += "    vec3 x = max(vec3(0.0), hdr - vec3(0.004));\n";
                    fragment += "    vec3 mapped = (x * (hdr * 6.2 + vec3(0.5))) / (x * (hdr * 6.2 + vec3(1.7)) + vec3(0.06));\n";
                    break;
                default:
                    fragment += "    vec3 mapped = hdr;\n";
                    break;
            }
            fragment += "    gl_FragColor = vec4(mapped, color.a);\n";
            fragment += "}\n";
            return shader.loadFromMemory(fragment, sf::Shader::Fragment);
        }

        bool PostProcess::generateGammaShader(sf::Shader& shader, float gamma) {
            std::string fragment;
            fragment += "uniform sampler2D texture;\n";
            fragment += "uniform float gamma;\n";
            fragment += "void main() {\n";
            fragment += "    vec4 color = texture2D(texture, gl_TexCoord[0].xy);\n";
            fragment += "    gl_FragColor = vec4(pow(color.rgb, vec3(1.0 / gamma)), color.a);\n";
            fragment += "}\n";
            return shader.loadFromMemory(fragment, sf::Shader::Fragment);
        }

    }
}
