#include "LightSystem.h"
#include <cmath>

namespace nebula {

    void LightSystem::addLight(const Light& light) {
        m_lights.push_back(light);
    }

    void LightSystem::removeLight(size_t index) {
        if (index < m_lights.size()) {
            m_lights.erase(m_lights.begin() + index);
        }
    }

    void LightSystem::clear() {
        m_lights.clear();
    }

    void LightSystem::setAmbientColor(const sf::Color& color) {
        m_ambientColor = color;
    }

    sf::Color LightSystem::getAmbientColor() const {
        return m_ambientColor;
    }

    void LightSystem::update(float dt) {
        (void)dt;
    }

    void LightSystem::render(sf::RenderTarget& target, const std::vector<sf::Drawable*>& occluders) {
        auto winSize = target.getSize();

        if (!m_lightMap || m_lightMap->getSize() != winSize) {
            m_lightMap = std::make_unique<sf::RenderTexture>();
            m_lightMap->create(winSize.x, winSize.y);
        }

        m_lightMap->clear(sf::Color::Transparent);

        sf::RenderStates states;
        states.blendMode = sf::BlendAdd;

        for (auto& light : m_lights) {
            sf::CircleShape lightShape(light.radius);
            lightShape.setOrigin(light.radius, light.radius);
            lightShape.setPosition(light.position);
            sf::Color c = light.color;
            c.a = static_cast<sf::Uint8>(light.intensity * 255.0f);
            lightShape.setFillColor(c);

            if (light.coneAngle > 0.0f) {
                sf::VertexArray fan(sf::TrianglesFan, 36);
                fan[0].position = light.position;
                fan[0].color = c;
                for (int i = 0; i < 35; i++) {
                    float a = light.angle - light.coneAngle / 2.0f +
                              (light.coneAngle * i) / 33.0f;
                    fan[i + 1].position = light.position + sf::Vector2f(
                        std::cos(a) * light.radius,
                        std::sin(a) * light.radius
                    );
                    fan[i + 1].color = sf::Color(c.r, c.g, c.b, 0);
                }
                m_lightMap->draw(fan, states);
            } else {
                m_lightMap->draw(lightShape, states);
            }
        }

        m_lightMap->display();

        sf::Sprite lightSprite(m_lightMap->getTexture());

        sf::RenderStates combineStates;
        combineStates.blendMode = sf::BlendMultiply;
        target.draw(lightSprite, combineStates);
    }

    size_t LightSystem::getLightCount() const {
        return m_lights.size();
    }

    Light& LightSystem::getLight(size_t index) {
        return m_lights[index];
    }
}
