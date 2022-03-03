#include "BlendMode.h"

namespace nebula {
    namespace graphics {

        BlendState::BlendState()
            : m_mode(BlendMode::Alpha), m_sfmlMode(sf::BlendAlpha) {}

        BlendState::BlendState(BlendMode mode) {
            switch (mode) {
                case BlendMode::Alpha:
                    m_sfmlMode = sf::BlendAlpha;
                    break;
                case BlendMode::Add:
                    m_sfmlMode = sf::BlendAdd;
                    break;
                case BlendMode::Multiply:
                    m_sfmlMode = sf::BlendMultiply;
                    break;
                case BlendMode::None:
                    m_sfmlMode = sf::BlendNone;
                    break;
                case BlendMode::Custom:
                    m_sfmlMode = sf::BlendMode(sf::BlendMode::SrcAlpha, sf::BlendMode::OneMinusSrcAlpha);
                    break;
            }
            m_mode = mode;
        }

        BlendState::BlendState(const sf::BlendMode& sfmlMode)
            : m_sfmlMode(sfmlMode) {
            syncFromSFML();
        }

        void BlendState::setColorSrcFactor(sf::BlendMode::Factor factor) {
            m_sfmlMode.colorSrcFactor = factor;
            syncFromSFML();
        }

        void BlendState::setColorDstFactor(sf::BlendMode::Factor factor) {
            m_sfmlMode.colorDstFactor = factor;
            syncFromSFML();
        }

        void BlendState::setAlphaSrcFactor(sf::BlendMode::Factor factor) {
            m_sfmlMode.alphaSrcFactor = factor;
            syncFromSFML();
        }

        void BlendState::setAlphaDstFactor(sf::BlendMode::Factor factor) {
            m_sfmlMode.alphaDstFactor = factor;
            syncFromSFML();
        }

        void BlendState::setColorEquation(sf::BlendMode::Equation equation) {
            m_sfmlMode.colorEquation = equation;
            syncFromSFML();
        }

        void BlendState::setAlphaEquation(sf::BlendMode::Equation equation) {
            m_sfmlMode.alphaEquation = equation;
            syncFromSFML();
        }

        sf::BlendMode::Factor BlendState::getColorSrcFactor() const {
            return m_sfmlMode.colorSrcFactor;
        }

        sf::BlendMode::Factor BlendState::getColorDstFactor() const {
            return m_sfmlMode.colorDstFactor;
        }

        sf::BlendMode::Factor BlendState::getAlphaSrcFactor() const {
            return m_sfmlMode.alphaSrcFactor;
        }

        sf::BlendMode::Factor BlendState::getAlphaDstFactor() const {
            return m_sfmlMode.alphaDstFactor;
        }

        sf::BlendMode::Equation BlendState::getColorEquation() const {
            return m_sfmlMode.colorEquation;
        }

        sf::BlendMode::Equation BlendState::getAlphaEquation() const {
            return m_sfmlMode.alphaEquation;
        }

        sf::BlendMode BlendState::toSFML() const {
            return m_sfmlMode;
        }

        BlendState BlendState::fromSFML(const sf::BlendMode& mode) {
            return BlendState(mode);
        }

        BlendState BlendState::createAlpha() {
            return BlendState(BlendMode::Alpha);
        }

        BlendState BlendState::createAdd() {
            return BlendState(BlendMode::Add);
        }

        BlendState BlendState::createMultiply() {
            return BlendState(BlendMode::Multiply);
        }

        BlendState BlendState::createNone() {
            return BlendState(BlendMode::None);
        }

        BlendState BlendState::createPremultipliedAlpha() {
            BlendState state;
            state.m_sfmlMode = sf::BlendMode(
                sf::BlendMode::One, sf::BlendMode::OneMinusSrcAlpha,
                sf::BlendMode::One, sf::BlendMode::OneMinusSrcAlpha
            );
            state.m_mode = BlendMode::Custom;
            return state;
        }

        BlendMode BlendState::getMode() const {
            return m_mode;
        }

        bool BlendState::operator==(const BlendState& other) const {
            return m_sfmlMode.colorSrcFactor == other.m_sfmlMode.colorSrcFactor &&
                   m_sfmlMode.colorDstFactor == other.m_sfmlMode.colorDstFactor &&
                   m_sfmlMode.alphaSrcFactor == other.m_sfmlMode.alphaSrcFactor &&
                   m_sfmlMode.alphaDstFactor == other.m_sfmlMode.alphaDstFactor &&
                   m_sfmlMode.colorEquation == other.m_sfmlMode.colorEquation &&
                   m_sfmlMode.alphaEquation == other.m_sfmlMode.alphaEquation;
        }

        bool BlendState::operator!=(const BlendState& other) const {
            return !(*this == other);
        }

        void BlendState::syncFromSFML() {
            if (m_sfmlMode == sf::BlendAlpha) m_mode = BlendMode::Alpha;
            else if (m_sfmlMode == sf::BlendAdd) m_mode = BlendMode::Add;
            else if (m_sfmlMode == sf::BlendMultiply) m_mode = BlendMode::Multiply;
            else if (m_sfmlMode == sf::BlendNone) m_mode = BlendMode::None;
            else m_mode = BlendMode::Custom;
        }

    }
}
