#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <vector>
#include <string>
#include <functional>
#include <memory>

namespace nebula {
    namespace graphics {

        enum class SpawnShape {
            Point,
            Circle,
            Rectangle,
            Cone,
            Ring,
            Line
        };

        struct Particle {
            sf::Vector2f position;
            sf::Vector2f velocity;
            sf::Color color;
            sf::Vector2f size;
            float rotation;
            float rotationSpeed;
            float lifetime;
            float elapsed;
            float stretchFactor;

            Particle()
                : position(0.f, 0.f)
                , velocity(0.f, 0.f)
                , color(sf::Color::White)
                , size(1.f, 1.f)
                , rotation(0.f)
                , rotationSpeed(0.f)
                , lifetime(1.f)
                , elapsed(0.f)
                , stretchFactor(0.f) {}
        };

        class ParticleEmitter {
        public:
            ParticleEmitter();
            explicit ParticleEmitter(const std::string& name);
            ~ParticleEmitter() = default;

            void start();
            void stop();
            void burst(int count);

            void update(float dt);
            void render(sf::VertexArray& vertexArray) const;

            void setEmissionRate(float rate);
            float getEmissionRate() const;

            void setSpawnShape(SpawnShape shape);
            SpawnShape getSpawnShape() const;

            void setSpawnSize(const sf::Vector2f& size);
            sf::Vector2f getSpawnSize() const;

            void setPosition(const sf::Vector2f& position);
            sf::Vector2f getPosition() const;

            void setSpreadAngle(float degrees);
            float getSpreadAngle() const;

            void setSpeed(float minSpeed, float maxSpeed);
            void setSpeedMin(float min);
            void setSpeedMax(float max);
            float getSpeedMin() const;
            float getSpeedMax() const;

            void setLifetime(float minLifetime, float maxLifetime);
            float getLifetimeMin() const;
            float getLifetimeMax() const;

            void setStartColor(const sf::Color& color);
            void setEndColor(const sf::Color& color);
            sf::Color getStartColor() const;
            sf::Color getEndColor() const;

            void setStartSize(const sf::Vector2f& size);
            void setEndSize(const sf::Vector2f& size);
            sf::Vector2f getStartSize() const;
            sf::Vector2f getEndSize() const;

            void setGravity(const sf::Vector2f& gravity);
            sf::Vector2f getGravity() const;

            void setRotation(float rotation);
            float getRotation() const;

            void setRotationOverLifetime(float minSpeed, float maxSpeed);
            float getRotationSpeedMin() const;
            float getRotationSpeedMax() const;

            void setTrailStretch(float factor);
            float getTrailStretch() const;

            void setConeAngle(float degrees);
            float getConeAngle() const;

            void setRingRadius(float inner, float outer);
            float getRingInnerRadius() const;
            float getRingOuterRadius() const;

            void setLineEndpoints(const sf::Vector2f& start, const sf::Vector2f& end);
            sf::Vector2f getLineStart() const;
            sf::Vector2f getLineEnd() const;

            void setSubEmitter(std::shared_ptr<ParticleEmitter> emitter);
            std::shared_ptr<ParticleEmitter> getSubEmitter() const;

            void setTexture(const std::shared_ptr<sf::Texture>& texture);
            std::shared_ptr<sf::Texture> getTexture() const;

            void setEmitCountPerBurst(int count);
            int getEmitCountPerBurst() const;

            int getParticleCount() const;
            bool isEmitting() const;

            const std::string& getName() const;

        private:
            std::string m_name;
            std::vector<Particle> m_particles;

            sf::Vector2f m_position;
            float m_emissionRate;
            float m_emissionAccumulator;
            bool m_emitting;
            SpawnShape m_spawnShape;
            sf::Vector2f m_spawnSize;

            float m_spreadAngle;
            float m_speedMin;
            float m_speedMax;
            float m_lifetimeMin;
            float m_lifetimeMax;

            sf::Color m_startColor;
            sf::Color m_endColor;
            sf::Vector2f m_startSize;
            sf::Vector2f m_endSize;

            sf::Vector2f m_gravity;
            float m_rotation;
            float m_rotationSpeedMin;
            float m_rotationSpeedMax;
            float m_trailStretch;

            float m_coneAngle;
            float m_ringInnerRadius;
            float m_ringOuterRadius;
            sf::Vector2f m_lineStart;
            sf::Vector2f m_lineEnd;

            std::shared_ptr<ParticleEmitter> m_subEmitter;

            int m_emitCountPerBurst;

            std::shared_ptr<sf::Texture> m_texture;

            void emitParticle(int count);
            sf::Vector2f randomSpawnOffset() const;
            static float randomRange(float min, float max);
        };

    }
}
