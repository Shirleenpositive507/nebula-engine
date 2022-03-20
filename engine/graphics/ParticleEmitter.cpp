#include "ParticleEmitter.h"
#include <cmath>
#include <random>
#include <algorithm>
#include <cstdlib>

namespace nebula {
    namespace graphics {

        static std::mt19937& rng() {
            static std::mt19937 instance(std::random_device{}());
            return instance;
        }

        ParticleEmitter::ParticleEmitter()
            : m_name("emitter")
            , m_position(0.f, 0.f)
            , m_emissionRate(10.f)
            , m_emissionAccumulator(0.f)
            , m_emitting(false)
            , m_spawnShape(SpawnShape::Point)
            , m_spawnSize(0.f, 0.f)
            , m_spreadAngle(360.f)
            , m_speedMin(50.f)
            , m_speedMax(150.f)
            , m_lifetimeMin(0.5f)
            , m_lifetimeMax(2.f)
            , m_startColor(sf::Color::White)
            , m_endColor(sf::Color(255, 255, 255, 0))
            , m_startSize(4.f, 4.f)
            , m_endSize(1.f, 1.f)
            , m_gravity(0.f, 0.f)
            , m_rotation(0.f)
            , m_emitCountPerBurst(10) {}

        ParticleEmitter::ParticleEmitter(const std::string& name)
            : ParticleEmitter() {
            m_name = name;
        }

        void ParticleEmitter::start() {
            m_emitting = true;
            m_emissionAccumulator = 0.f;
        }

        void ParticleEmitter::stop() {
            m_emitting = false;
        }

        void ParticleEmitter::burst(int count) {
            emitParticle(count);
        }

        void ParticleEmitter::update(float dt) {
            if (dt <= 0.f) return;

            for (auto it = m_particles.begin(); it != m_particles.end();) {
                it->elapsed += dt;

                if (it->elapsed >= it->lifetime) {
                    it = m_particles.erase(it);
                    continue;
                }

                float t = it->elapsed / it->lifetime;

                it->velocity += m_gravity * dt;
                it->position += it->velocity * dt;

                it->color.r = static_cast<sf::Uint8>(
                    std::lerp(static_cast<float>(m_startColor.r),
                              static_cast<float>(m_endColor.r), t));
                it->color.g = static_cast<sf::Uint8>(
                    std::lerp(static_cast<float>(m_startColor.g),
                              static_cast<float>(m_endColor.g), t));
                it->color.b = static_cast<sf::Uint8>(
                    std::lerp(static_cast<float>(m_startColor.b),
                              static_cast<float>(m_endColor.b), t));
                it->color.a = static_cast<sf::Uint8>(
                    std::lerp(static_cast<float>(m_startColor.a),
                              static_cast<float>(m_endColor.a), t));

                it->size.x = std::lerp(m_startSize.x, m_endSize.x, t);
                it->size.y = std::lerp(m_startSize.y, m_endSize.y, t);

                it->rotation += m_rotation * dt;

                ++it;
            }

            if (m_emitting && m_emissionRate > 0.f) {
                m_emissionAccumulator += m_emissionRate * dt;
                int count = static_cast<int>(m_emissionAccumulator);
                if (count > 0) {
                    emitParticle(count);
                    m_emissionAccumulator -= static_cast<float>(count);
                }
            }
        }

        void ParticleEmitter::render(sf::VertexArray& vertexArray) const {
            if (m_texture) {
                float halfTexW = static_cast<float>(m_texture->getSize().x) * 0.5f;
                float halfTexH = static_cast<float>(m_texture->getSize().y) * 0.5f;

                for (const auto& p : m_particles) {
                    float halfW = p.size.x * 0.5f;
                    float halfH = p.size.y * 0.5f;
                    float cosR = std::cos(p.rotation * 3.14159265f / 180.f);
                    float sinR = std::sin(p.rotation * 3.14159265f / 180.f);

                    auto rot = [&](float dx, float dy) -> sf::Vector2f {
                        return sf::Vector2f(
                            dx * cosR - dy * sinR,
                            dx * sinR + dy * cosR
                        );
                    };

                    sf::Vector2f corners[4] = {
                        rot(-halfW, -halfH),
                        rot( halfW, -halfH),
                        rot( halfW,  halfH),
                        rot(-halfW,  halfH)
                    };

                    sf::Vector2f texCoords[4] = {
                        sf::Vector2f(0.f, 0.f),
                        sf::Vector2f(m_texture->getSize().x, 0.f),
                        sf::Vector2f(m_texture->getSize().x, m_texture->getSize().y),
                        sf::Vector2f(0.f, m_texture->getSize().y)
                    };

                    for (int i = 0; i < 4; ++i) {
                        sf::Vertex v;
                        v.position = p.position + corners[i];
                        v.color = p.color;
                        v.texCoords = texCoords[i];
                        vertexArray.append(v);
                    }
                }
            } else {
                for (const auto& p : m_particles) {
                    float halfW = p.size.x * 0.5f;
                    float halfH = p.size.y * 0.5f;

                    sf::Vector2f corners[4] = {
                        sf::Vector2f(-halfW, -halfH),
                        sf::Vector2f( halfW, -halfH),
                        sf::Vector2f( halfW,  halfH),
                        sf::Vector2f(-halfW,  halfH)
                    };

                    float cosR = std::cos(p.rotation * 3.14159265f / 180.f);
                    float sinR = std::sin(p.rotation * 3.14159265f / 180.f);

                    for (int i = 0; i < 4; ++i) {
                        sf::Vertex v;
                        float rx = corners[i].x * cosR - corners[i].y * sinR;
                        float ry = corners[i].x * sinR + corners[i].y * cosR;
                        v.position = p.position + sf::Vector2f(rx, ry);
                        v.color = p.color;
                        v.texCoords = sf::Vector2f(0.f, 0.f);
                        vertexArray.append(v);
                    }
                }
            }
        }

        void ParticleEmitter::setEmissionRate(float rate) {
            m_emissionRate = std::max(0.f, rate);
        }

        float ParticleEmitter::getEmissionRate() const {
            return m_emissionRate;
        }

        void ParticleEmitter::setSpawnShape(SpawnShape shape) {
            m_spawnShape = shape;
        }

        SpawnShape ParticleEmitter::getSpawnShape() const {
            return m_spawnShape;
        }

        void ParticleEmitter::setSpawnSize(const sf::Vector2f& size) {
            m_spawnSize = size;
        }

        sf::Vector2f ParticleEmitter::getSpawnSize() const {
            return m_spawnSize;
        }

        void ParticleEmitter::setPosition(const sf::Vector2f& position) {
            m_position = position;
        }

        sf::Vector2f ParticleEmitter::getPosition() const {
            return m_position;
        }

        void ParticleEmitter::setSpreadAngle(float degrees) {
            m_spreadAngle = std::clamp(degrees, 0.f, 360.f);
        }

        float ParticleEmitter::getSpreadAngle() const {
            return m_spreadAngle;
        }

        void ParticleEmitter::setSpeed(float minSpeed, float maxSpeed) {
            m_speedMin = std::min(minSpeed, maxSpeed);
            m_speedMax = std::max(minSpeed, maxSpeed);
        }

        void ParticleEmitter::setSpeedMin(float min) {
            m_speedMin = std::max(0.f, min);
            if (m_speedMin > m_speedMax) m_speedMax = m_speedMin;
        }

        void ParticleEmitter::setSpeedMax(float max) {
            m_speedMax = std::max(0.f, max);
            if (m_speedMax < m_speedMin) m_speedMin = m_speedMax;
        }

        float ParticleEmitter::getSpeedMin() const {
            return m_speedMin;
        }

        float ParticleEmitter::getSpeedMax() const {
            return m_speedMax;
        }

        void ParticleEmitter::setLifetime(float minLifetime, float maxLifetime) {
            m_lifetimeMin = std::max(0.01f, std::min(minLifetime, maxLifetime));
            m_lifetimeMax = std::max(m_lifetimeMin, std::max(minLifetime, maxLifetime));
        }

        float ParticleEmitter::getLifetimeMin() const {
            return m_lifetimeMin;
        }

        float ParticleEmitter::getLifetimeMax() const {
            return m_lifetimeMax;
        }

        void ParticleEmitter::setStartColor(const sf::Color& color) {
            m_startColor = color;
        }

        void ParticleEmitter::setEndColor(const sf::Color& color) {
            m_endColor = color;
        }

        sf::Color ParticleEmitter::getStartColor() const {
            return m_startColor;
        }

        sf::Color ParticleEmitter::getEndColor() const {
            return m_endColor;
        }

        void ParticleEmitter::setStartSize(const sf::Vector2f& size) {
            m_startSize = size;
        }

        void ParticleEmitter::setEndSize(const sf::Vector2f& size) {
            m_endSize = size;
        }

        sf::Vector2f ParticleEmitter::getStartSize() const {
            return m_startSize;
        }

        sf::Vector2f ParticleEmitter::getEndSize() const {
            return m_endSize;
        }

        void ParticleEmitter::setGravity(const sf::Vector2f& gravity) {
            m_gravity = gravity;
        }

        sf::Vector2f ParticleEmitter::getGravity() const {
            return m_gravity;
        }

        void ParticleEmitter::setRotation(float rotation) {
            m_rotation = rotation;
        }

        float ParticleEmitter::getRotation() const {
            return m_rotation;
        }

        void ParticleEmitter::setTexture(const std::shared_ptr<sf::Texture>& texture) {
            m_texture = texture;
        }

        std::shared_ptr<sf::Texture> ParticleEmitter::getTexture() const {
            return m_texture;
        }

        void ParticleEmitter::setEmitCountPerBurst(int count) {
            m_emitCountPerBurst = std::max(1, count);
        }

        int ParticleEmitter::getEmitCountPerBurst() const {
            return m_emitCountPerBurst;
        }

        int ParticleEmitter::getParticleCount() const {
            return static_cast<int>(m_particles.size());
        }

        bool ParticleEmitter::isEmitting() const {
            return m_emitting;
        }

        const std::string& ParticleEmitter::getName() const {
            return m_name;
        }

        void ParticleEmitter::emitParticle(int count) {
            for (int i = 0; i < count; ++i) {
                Particle p;

                sf::Vector2f offset = randomSpawnOffset();
                p.position = m_position + offset;

                float angle = randomRange(0.f, m_spreadAngle) * 3.14159265f / 180.f;
                float speed = randomRange(m_speedMin, m_speedMax);
                p.velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);

                p.lifetime = randomRange(m_lifetimeMin, m_lifetimeMax);
                p.elapsed = 0.f;
                p.color = m_startColor;
                p.size = m_startSize;
                p.rotation = 0.f;

                m_particles.push_back(p);
            }
        }

        sf::Vector2f ParticleEmitter::randomSpawnOffset() const {
            switch (m_spawnShape) {
                case SpawnShape::Circle: {
                    float angle = randomRange(0.f, 360.f) * 3.14159265f / 180.f;
                    float radius = randomRange(0.f, 1.f) * std::max(m_spawnSize.x, m_spawnSize.y) * 0.5f;
                    return sf::Vector2f(std::cos(angle) * radius, std::sin(angle) * radius);
                }
                case SpawnShape::Rectangle: {
                    float halfW = m_spawnSize.x * 0.5f;
                    float halfH = m_spawnSize.y * 0.5f;
                    return sf::Vector2f(randomRange(-halfW, halfW), randomRange(-halfH, halfH));
                }
                case SpawnShape::Point:
                default:
                    return sf::Vector2f(0.f, 0.f);
            }
        }

        float ParticleEmitter::randomRange(float min, float max) {
            std::uniform_real_distribution<float> dist(min, max);
            return dist(rng());
        }

    }
}
