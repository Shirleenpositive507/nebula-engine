#pragma once

#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace nebula {
    namespace graphics {

        enum class EffectType {
            Blur,
            Glow,
            ColorGrading,
            ChromaticAberration,
            Vignette,
            Bloom,
            Custom
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

            void setActive(const std::string& name, bool active);

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

            PostEffect* getEffect(const std::string& name);
            std::vector<std::string> getEffectNames() const;

            bool isInitialized() const;

        private:
            std::vector<PostEffect> m_effects;
            sf::RenderTexture m_ping;
            sf::RenderTexture m_pong;
            sf::Sprite m_sprite;
            bool m_initialized;

            void initializeTempTextures(unsigned int width, unsigned int height);
            void applyEffect(const PostEffect& effect, sf::RenderTexture& input, sf::RenderTexture& output);
            void applyBlur(sf::RenderTexture& input, sf::RenderTexture& output, float intensity, const sf::Vector2f& direction);
            void setupDefaultShaders();
            bool generateBlurShader(sf::Shader& shader, float intensity, const sf::Vector2f& direction);
            bool generateGlowShader(sf::Shader& shader, float threshold, float intensity);
            bool generateChromaticAberrationShader(sf::Shader& shader, float strength);
            bool generateVignetteShader(sf::Shader& shader, float intensity, const sf::Color& color, float radius);
            bool generateBloomShader(sf::Shader& shader, float threshold, float intensity);
        };

    }
}
