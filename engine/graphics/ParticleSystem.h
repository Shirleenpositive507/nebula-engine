#pragma once

#include "ParticleEmitter.h"
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

namespace nebula {
    namespace graphics {

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

        private:
            std::unordered_map<std::string, std::shared_ptr<ParticleEmitter>> m_emitters;
            sf::VertexArray m_vertexArray;
            std::size_t m_maxParticles;
            sf::Vector2f m_globalGravity;
            sf::Vector2f m_globalWind;

            void buildVertexArray();
        };

    }
}
