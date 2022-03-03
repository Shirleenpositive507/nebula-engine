#pragma once

#include <SFML/Graphics/BlendMode.hpp>
#include <string>

namespace nebula {
    namespace graphics {

        enum class BlendMode {
            Alpha,
            Add,
            Multiply,
            None,
            Custom
        };

        class BlendState {
        public:
            BlendState();
            explicit BlendState(BlendMode mode);
            explicit BlendState(const sf::BlendMode& sfmlMode);

            void setColorSrcFactor(sf::BlendMode::Factor factor);
            void setColorDstFactor(sf::BlendMode::Factor factor);
            void setAlphaSrcFactor(sf::BlendMode::Factor factor);
            void setAlphaDstFactor(sf::BlendMode::Factor factor);
            void setColorEquation(sf::BlendMode::Equation equation);
            void setAlphaEquation(sf::BlendMode::Equation equation);

            sf::BlendMode::Factor getColorSrcFactor() const;
            sf::BlendMode::Factor getColorDstFactor() const;
            sf::BlendMode::Factor getAlphaSrcFactor() const;
            sf::BlendMode::Factor getAlphaDstFactor() const;
            sf::BlendMode::Equation getColorEquation() const;
            sf::BlendMode::Equation getAlphaEquation() const;

            sf::BlendMode toSFML() const;
            static BlendState fromSFML(const sf::BlendMode& mode);

            static BlendState createAlpha();
            static BlendState createAdd();
            static BlendState createMultiply();
            static BlendState createNone();
            static BlendState createPremultipliedAlpha();

            BlendMode getMode() const;

            bool operator==(const BlendState& other) const;
            bool operator!=(const BlendState& other) const;

        private:
            BlendMode m_mode;
            sf::BlendMode m_sfmlMode;

            void syncFromSFML();
            void syncToSFML();
        };

    }
}
