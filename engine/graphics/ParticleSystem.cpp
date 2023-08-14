#include "ParticleSystem.h"
#include <algorithm>
#include <stdexcept>
#include <cmath>

namespace nebula {
    namespace graphics {

        ParticleSystem::ParticleSystem()
            : m_vertexArray(sf::PrimitiveType::Quads)
            , m_maxParticles(10000)
            , m_globalGravity(0.f, 0.f)
            , m_globalWind(0.f, 0.f)
            , m_sortMode(ParticleSortMode::None)
            , m_updateMode(ParticleUpdateMode::CPU)
            , m_trailEnabled(false)
            , m_trailLifetime(0.5f)
            , m_subFrameEmission(false)
            , m_subFrameSteps(4) {}

        ParticleSystem::ParticleSystem(std::size_t maxParticles)
            : m_vertexArray(sf::PrimitiveType::Quads)
            , m_maxParticles(maxParticles)
            , m_globalGravity(0.f, 0.f)
            , m_globalWind(0.f, 0.f)
            , m_sortMode(ParticleSortMode::None)
            , m_updateMode(ParticleUpdateMode::CPU)
            , m_trailEnabled(false)
            , m_trailLifetime(0.5f)
            , m_subFrameEmission(false)
            , m_subFrameSteps(4) {}

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
            applyAttractors(dt);

            if (m_subFrameEmission && m_subFrameSteps > 1) {
                float subDt = dt / static_cast<float>(m_subFrameSteps);
                for (unsigned int step = 0; step < m_subFrameSteps; ++step) {
                    for (auto& pair : m_emitters) {
                        pair.second->setGravity(pair.second->getGravity() + m_globalGravity);
                        pair.second->update(subDt);
                    }
                }
            } else {
                for (auto& pair : m_emitters) {
                    pair.second->setGravity(pair.second->getGravity() + m_globalGravity);
                    pair.second->update(dt);
                }
            }

            if (m_trailEnabled) {
                updateTrails(dt);
            }
        }

        void ParticleSystem::renderAll(sf::RenderTarget& target) {
            buildVertexArray();

            if (m_vertexArray.getVertexCount() == 0) return;

            if (m_sortMode != ParticleSortMode::None) {
                sortParticles();
            }

            sf::RenderStates states;
            states.blendMode = sf::BlendAdd;

            if (m_renderCallback) {
                m_renderCallback(target, m_vertexArray);
            } else {
                target.draw(m_vertexArray, states);
            }
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
                if (pair.second->getParticleCount() > m_maxParticles) {
                    pair.second->setMaxParticles(m_maxParticles);
                }
                pair.second->render(m_vertexArray);
            }
            std::size_t totalVerts = m_vertexArray.getVertexCount();
            if (totalVerts > m_maxParticles * 4) {
                m_vertexArray.resize(m_maxParticles * 4);
            }
        }

        // --- Attractors ---

        void ParticleSystem::addAttractor(const ParticleAttractor& attractor) {
            m_attractors.push_back(attractor);
        }

        void ParticleSystem::removeAttractor(std::size_t index) {
            if (index < m_attractors.size()) {
                m_attractors.erase(m_attractors.begin() + static_cast<ptrdiff_t>(index));
            }
        }

        void ParticleSystem::clearAttractors() {
            m_attractors.clear();
        }

        std::size_t ParticleSystem::getAttractorCount() const {
            return m_attractors.size();
        }

        const ParticleAttractor& ParticleSystem::getAttractor(std::size_t index) const {
            return m_attractors[index];
        }

        void ParticleSystem::applyAttractors(float dt) {
            if (m_attractors.empty()) return;
            for (auto& pair : m_emitters) {
                auto& particles = pair.second->getParticles();
                for (auto& p : particles) {
                    for (const auto& attr : m_attractors) {
                        sf::Vector2f diff = attr.position - p.position;
                        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
                        if (dist < attr.radius && dist > 0.01f) {
                            float force = attr.strength / (dist * 0.1f + 1.0f);
                            if (attr.repeller) force = -force;
                            p.velocity += diff / dist * force * dt;
                        }
                    }
                }
            }
        }

        // --- Sorting ---

        void ParticleSystem::setSortMode(ParticleSortMode mode) {
            m_sortMode = mode;
        }

        ParticleSortMode ParticleSystem::getSortMode() const {
            return m_sortMode;
        }

        void ParticleSystem::sortParticles() {
            float sortDir = (m_sortMode == ParticleSortMode::FrontToBack) ? -1.0f : 1.0f;
            for (auto& pair : m_emitters) {
                auto& particles = pair.second->getParticles();
                std::sort(particles.begin(), particles.end(),
                    [sortDir](const Particle& a, const Particle& b) {
                        return (a.position.y * sortDir) < (b.position.y * sortDir);
                    });
            }
        }

        // --- Trails ---

        void ParticleSystem::setTrailEnabled(bool enabled) {
            m_trailEnabled = enabled;
        }

        bool ParticleSystem::isTrailEnabled() const {
            return m_trailEnabled;
        }

        void ParticleSystem::setTrailLifetime(float lifetime) {
            m_trailLifetime = lifetime;
        }

        float ParticleSystem::getTrailLifetime() const {
            return m_trailLifetime;
        }

        void ParticleSystem::updateTrails(float dt) {
            for (auto& pair : m_emitters) {
                auto& particles = pair.second->getParticles();
                for (auto& p : particles) {
                    ParticleTrail* trail = nullptr;
                    for (auto& t : m_trails) {
                        if (t.points.size() > 0) {
                            sf::Vector2f last = t.points.back();
                            sf::Vector2f diff = p.position - last;
                            if (diff.x * diff.x + diff.y * diff.y > 4.0f) {
                                t.points.push_back(p.position);
                                break;
                            }
                        }
                    }
                    if (!trail) {
                        ParticleTrail newTrail;
                        newTrail.enabled = true;
                        newTrail.lifetime = m_trailLifetime;
                        newTrail.points.push_back(p.position);
                        m_trails.push_back(newTrail);
                    }
                }
            }

            for (auto it = m_trails.begin(); it != m_trails.end(); ) {
                it->time += dt;
                if (it->time >= it->lifetime) {
                    it = m_trails.erase(it);
                } else {
                    ++it;
                }
            }
        }

        // --- GPU / Sub-frame ---

        void ParticleSystem::setUpdateMode(ParticleUpdateMode mode) {
            m_updateMode = mode;
        }

        ParticleUpdateMode ParticleSystem::getUpdateMode() const {
            return m_updateMode;
        }

        bool ParticleSystem::isGPUAccelerated() const {
            return m_updateMode == ParticleUpdateMode::GPU;
        }

        void ParticleSystem::setSubFrameEmission(bool enabled) {
            m_subFrameEmission = enabled;
        }

        bool ParticleSystem::isSubFrameEmissionEnabled() const {
            return m_subFrameEmission;
        }

        void ParticleSystem::setSubFrameSteps(unsigned int steps) {
            m_subFrameSteps = std::max(1u, steps);
        }

        unsigned int ParticleSystem::getSubFrameSteps() const {
            return m_subFrameSteps;
        }

        void ParticleSystem::setRenderCallback(ParticleRenderCallback callback) {
            m_renderCallback = callback;
        }

    }
}
