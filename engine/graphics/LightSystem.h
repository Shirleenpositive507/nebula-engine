#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

namespace nebula {
    struct Light {
        sf::Vector2f position;
        sf::Color color;
        float radius;
        float intensity;
        float angle;
        float coneAngle; // 0 = point light
    };

    class LightSystem {
    public:
        void addLight(const Light& light);
        void removeLight(size_t index);
        void clear();
        void setAmbientColor(const sf::Color& color);
        sf::Color getAmbientColor() const;
        void render(sf::RenderTarget& target, const std::vector<sf::Drawable*>& occluders);
        void update(float dt);
        size_t getLightCount() const;
        Light& getLight(size_t index);
    private:
        std::vector<Light> m_lights;
        sf::Color m_ambientColor{20, 20, 40};
        std::unique_ptr<sf::RenderTexture> m_lightMap;
    };
}
