#include "ParticleSystem.h"
#include <algorithm>
#include <stdexcept>

namespace nebula {
    namespace graphics {

        ParticleSystem::ParticleSystem()
            : m_vertexArray(sf::PrimitiveType::Quads)
            , m_maxParticles(10000)
            , m_globalGravity(0.f, 0.f)
            , m_globalWind(0.f, 0.f) {}

        ParticleSystem::ParticleSystem(std::size_t maxParticles)
            : m_vertexArray(sf::PrimitiveType::Quads)
            , m_maxParticles(maxParticles)
            , m_globalGravity(0.f, 0.f)
            , m_globalWind(0.f, 0.f) {}

        void ParticleSystem::addEmitter(const std::string& name, const std::shared_ptr<ParticleEmitter>& emitter) {
            m_emitters[name] = emitter;
        }

        void ParticleSystem::removeEmitter(const std::string& name) {
            m_emitters.erase(name);
        }

        std::shared_ptr<ParticleEmitter> ParticleSystem::getEmitter(const std::string& name) {
            auto it = m_emitters.find(name);
            return it != m_emitters.end() ? it->second : nullptr;
        }

        std::shared_ptr<const ParticleEmitter> ParticleSystem::getEmitter(const std::string& name) const {
            auto it = m_emitters.find(name);
            return it != m_emitters.end() ? it->second : nullptr;
        }

        void ParticleSystem::updateAll(float dt) {
            for (auto& pair : m_emitters) {
                pair.second->setGravity(pair.second->getGravity() + m_globalGravity);
                pair.second->update(dt);
            }
        }

        void ParticleSystem::renderAll(sf::RenderTarget& target) {
            buildVertexArray();

            if (m_vertexArray.getVertexCount() == 0) return;

            sf::RenderStates states;
            states.blendMode = sf::BlendAdd;

            target.draw(m_vertexArray, states);
        }

        void ParticleSystem::prewarm(float seconds) {
            float step = 1.f / 60.f;
            float elapsed = 0.f;

            while (elapsed < seconds) {
                float dt = std::min(step, seconds - elapsed);
                updateAll(dt);
                elapsed += dt;
            }
        }

        void ParticleSystem::setGlobalGravity(const sf::Vector2f& gravity) {
            m_globalGravity = gravity;
        }

        sf::Vector2f ParticleSystem::getGlobalGravity() const {
            return m_globalGravity;
        }

        void ParticleSystem::setGlobalWind(const sf::Vector2f& wind) {
            m_globalWind = wind;
        }

        sf::Vector2f ParticleSystem::getGlobalWind() const {
            return m_globalWind;
        }

        void ParticleSystem::clear() {
            m_vertexArray.clear();
            for (auto& pair : m_emitters) {
                pair.second->stop();
            }
        }

        void ParticleSystem::removeAllEmitters() {
            m_emitters.clear();
            m_vertexArray.clear();
        }

        int ParticleSystem::getTotalParticleCount() const {
            int total = 0;
            for (const auto& pair : m_emitters) {
                total += pair.second->getParticleCount();
            }
            return total;
        }

        void ParticleSystem::setMaxParticles(std::size_t maxParticles) {
            m_maxParticles = maxParticles;
        }

        std::size_t ParticleSystem::getMaxParticles() const {
            return m_maxParticles;
        }

        std::vector<std::string> ParticleSystem::getEmitterNames() const {
            std::vector<std::string> names;
            names.reserve(m_emitters.size());
            for (const auto& pair : m_emitters) {
                names.push_back(pair.first);
            }
            return names;
        }

        void ParticleSystem::buildVertexArray() {
            m_vertexArray.clear();
            m_vertexArray.resize(0);

            for (const auto& pair : m_emitters) {
                pair.second->render(m_vertexArray);
            }

            std::size_t totalVerts = m_vertexArray.getVertexCount();
            if (totalVerts > m_maxParticles * 4) {
                m_vertexArray.resize(m_maxParticles * 4);
            }
        }

    }
}
