#pragma once

#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

namespace nebula {
    namespace graphics {

        enum class EffectType {
            Blur,
            Glow,
            ColorGrading,
            ChromaticAberration,
            Vignette,
            Bloom,
            ToneMapReinhard,
            ToneMapACES,
            ToneMapFilmic,
            GammaCorrection,
            Custom
        };

        enum class ToneMapOperator {
            Reinhard,
            ACES,
            Filmic,
            None
        };

        struct EffectParameter {
            enum Type { Float, Vec2, Vec3, Vec4, Int, Texture };

            Type type;
            float floatValue;
            sf::Vector2f vec2Value;
            sf::Vector3f vec3Value;
            sf::Glsl::Vec4 vec4Value;
            int intValue;
            std::shared_ptr<sf::Texture> textureValue;

            EffectParameter() : type(Float), floatValue(0.f), intValue(0) {}
            explicit EffectParameter(float v) : type(Float), floatValue(v), intValue(0) {}
            EffectParameter(const sf::Vector2f& v) : type(Vec2), vec2Value(v), floatValue(0.f), intValue(0) {}
            EffectParameter(const sf::Vector3f& v) : type(Vec3), vec3Value(v), floatValue(0.f), intValue(0) {}
            EffectParameter(const sf::Glsl::Vec4& v) : type(Vec4), vec4Value(v), floatValue(0.f), intValue(0) {}
            EffectParameter(int v) : type(Int), intValue(v), floatValue(0.f) {}
            EffectParameter(const std::shared_ptr<sf::Texture>& v) : type(Texture), textureValue(v), floatValue(0.f), intValue(0) {}
        };

        struct PostEffect {
            EffectType type;
            std::string name;
            bool enabled;
            std::shared_ptr<sf::Shader> shader;
            std::unordered_map<std::string, EffectParameter> parameters;

            PostEffect() : type(EffectType::Custom), enabled(true) {}
        };

        class PostProcess {
        public:
            PostProcess();
            ~PostProcess() = default;

            void addEffect(const std::string& name, EffectType type);
            void addEffect(const std::string& name, const std::shared_ptr<sf::Shader>& customShader);

            bool removeEffect(const std::string& name);

            void enable(const std::string& name);
            void disable(const std::string& name);
            bool isEnabled(const std::string& name) const;

            void setActive(const std::string& name, bool active);

            // Effect chain reordering
            void moveEffectUp(const std::string& name);
            void moveEffectDown(const std::string& name);
            void moveEffectTo(const std::string& name, std::size_t position);

            void render(sf::RenderTexture& input, sf::RenderTarget& output);

            void clearEffects();

            void setEffectParameter(const std::string& effectName, const std::string& paramName, const EffectParameter& value);
            EffectParameter getEffectParameter(const std::string& effectName, const std::string& paramName) const;

            void setBlur(float intensity, const sf::Vector2f& direction = sf::Vector2f(1.f, 0.f));
            void setGlow(float threshold, float blurAmount, float intensity);
            void setColorGrading(const std::shared_ptr<sf::Texture>& lookupTable, float intensity);
            void setChromaticAberration(float strength);
            void setVignette(float intensity, const sf::Color& color = sf::Color::Black, float radius = 0.5f);
            void setBloom(float threshold, float blurAmount, float intensity);

            // HDR tone mapping
            void setToneMapping(ToneMapOperator op);
            ToneMapOperator getToneMapping() const;
            void setExposure(float exposure);
            float getExposure() const;

            // Gamma correction
            void setGamma(float gamma);
            float getGamma() const;
            void setGammaCorrectionEnabled(bool enabled);
            bool isGammaCorrectionEnabled() const;

            PostEffect* getEffect(const std::string& name);
            std::vector<std::string> getEffectNames() const;

            bool isInitialized() const;

            // Full-screen quad utility
            static sf::VertexArray createFullScreenQuad();
            static void renderFullScreenQuad(sf::RenderTarget& target, sf::Shader* shader = nullptr);

        private:
            std::vector<PostEffect> m_effects;
            sf::RenderTexture m_ping;
            sf::RenderTexture m_pong;
            sf::Sprite m_sprite;
            bool m_initialized;

            ToneMapOperator m_toneMapOp;
            float m_exposure;
            float m_gamma;
            bool m_gammaCorrection;

            void initializeTempTextures(unsigned int width, unsigned int height);
            void applyEffect(const PostEffect& effect, sf::RenderTexture& input, sf::RenderTexture& output);
            void applyBlur(sf::RenderTexture& input, sf::RenderTexture& output, float intensity, const sf::Vector2f& direction);
            void setupDefaultShaders();
            bool generateBlurShader(sf::Shader& shader, float intensity, const sf::Vector2f& direction);
            bool generateGlowShader(sf::Shader& shader, float threshold, float intensity);
            bool generateChromaticAberrationShader(sf::Shader& shader, float strength);
            bool generateVignetteShader(sf::Shader& shader, float intensity, const sf::Color& color, float radius);
            bool generateBloomShader(sf::Shader& shader, float threshold, float intensity);
            bool generateToneMapShader(sf::Shader& shader, ToneMapOperator op, float exposure);
            bool generateGammaShader(sf::Shader& shader, float gamma);
        };

    }
}
