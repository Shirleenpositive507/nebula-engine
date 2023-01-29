#pragma once

#include "ParticleEmitter.h"
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <functional>
#include <algorithm>

namespace nebula {
    namespace graphics {

        struct ParticleAttractor {
            sf::Vector2f position;
            float strength;
            float radius;
            bool repeller;

            ParticleAttractor()
                : position(0.f, 0.f), strength(1.f), radius(100.f), repeller(false) {}
            ParticleAttractor(const sf::Vector2f& pos, float str, float rad, bool repel = false)
                : position(pos), strength(str), radius(rad), repeller(repel) {}
        };

        struct ParticleTrail {
            std::vector<sf::Vector2f> points;
            float lifetime;
            float time;
            bool enabled;

            ParticleTrail() : lifetime(0.5f), time(0), enabled(false) {}
        };

        enum class ParticleSortMode {
            None,
            BackToFront,
            FrontToBack
        };

        enum class ParticleUpdateMode {
            CPU,
            GPU
        };

        class ParticleSystem {
        public:
            ParticleSystem();
            explicit ParticleSystem(std::size_t maxParticles);
            ~ParticleSystem() = default;

            void addEmitter(const std::string& name, const std::shared_ptr<ParticleEmitter>& emitter);
            void removeEmitter(const std::string& name);
            std::shared_ptr<ParticleEmitter> getEmitter(const std::string& name);
            std::shared_ptr<const ParticleEmitter> getEmitter(const std::string& name) const;

            void updateAll(float dt);
            void renderAll(sf::RenderTarget& target);

            void prewarm(float seconds);

            void setGlobalGravity(const sf::Vector2f& gravity);
            sf::Vector2f getGlobalGravity() const;

            void setGlobalWind(const sf::Vector2f& wind);
            sf::Vector2f getGlobalWind() const;

            void clear();
            void removeAllEmitters();

            int getTotalParticleCount() const;
            void setMaxParticles(std::size_t maxParticles);
            std::size_t getMaxParticles() const;

            std::vector<std::string> getEmitterNames() const;

            // Attractors and repellers
            void addAttractor(const ParticleAttractor& attractor);
            void removeAttractor(std::size_t index);
            void clearAttractors();
            std::size_t getAttractorCount() const;
            const ParticleAttractor& getAttractor(std::size_t index) const;

            // Particle trails
            void setTrailEnabled(bool enabled);
            bool isTrailEnabled() const;
            void setTrailLifetime(float lifetime);
            float getTrailLifetime() const;

            // Particle sorting
            void setSortMode(ParticleSortMode mode);
            ParticleSortMode getSortMode() const;

            // GPU particle support
            void setUpdateMode(ParticleUpdateMode mode);
            ParticleUpdateMode getUpdateMode() const;
            bool isGPUAccelerated() const;

            // Sub-frame emission
            void setSubFrameEmission(bool enabled);
            bool isSubFrameEmissionEnabled() const;
            void setSubFrameSteps(unsigned int steps);
            unsigned int getSubFrameSteps() const;

            using ParticleRenderCallback = std::function<void(sf::RenderTarget&, const sf::VertexArray&)>;
            void setRenderCallback(ParticleRenderCallback callback);

        private:
            std::unordered_map<std::string, std::shared_ptr<ParticleEmitter>> m_emitters;
            sf::VertexArray m_vertexArray;
            std::size_t m_maxParticles;
            sf::Vector2f m_globalGravity;
            sf::Vector2f m_globalWind;

            std::vector<ParticleAttractor> m_attractors;
            std::vector<ParticleTrail> m_trails;
            ParticleSortMode m_sortMode;
            ParticleUpdateMode m_updateMode;
            bool m_trailEnabled;
            float m_trailLifetime;
            bool m_subFrameEmission;
            unsigned int m_subFrameSteps;
            ParticleRenderCallback m_renderCallback;

            void buildVertexArray();
            void applyAttractors(float dt);
            void sortParticles();
            void updateTrails(float dt);
        };

    }
}
