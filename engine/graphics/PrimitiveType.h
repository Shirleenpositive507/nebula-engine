#pragma once

#include <SFML/Graphics/PrimitiveType.hpp>

namespace nebula {
    namespace graphics {

        enum class PrimitiveType {
            Points = static_cast<int>(sf::PrimitiveType::Points),
            Lines = static_cast<int>(sf::PrimitiveType::Lines),
            LineStrip = static_cast<int>(sf::PrimitiveType::LineStrip),
            Triangles = static_cast<int>(sf::PrimitiveType::Triangles),
            TriangleStrip = static_cast<int>(sf::PrimitiveType::TriangleStrip),
            TriangleFan = static_cast<int>(sf::PrimitiveType::TriangleFan),
            Quads = static_cast<int>(sf::PrimitiveType::Quads)
        };

        inline sf::PrimitiveType toSFML(PrimitiveType type) {
            return static_cast<sf::PrimitiveType>(static_cast<int>(type));
        }

        inline PrimitiveType fromSFML(sf::PrimitiveType type) {
            return static_cast<PrimitiveType>(static_cast<int>(type));
        }

        inline const char* toString(PrimitiveType type) {
            switch (type) {
                case PrimitiveType::Points: return "Points";
                case PrimitiveType::Lines: return "Lines";
                case PrimitiveType::LineStrip: return "LineStrip";
                case PrimitiveType::Triangles: return "Triangles";
                case PrimitiveType::TriangleStrip: return "TriangleStrip";
                case PrimitiveType::TriangleFan: return "TriangleFan";
                case PrimitiveType::Quads: return "Quads";
                default: return "Unknown";
            }
        }

    }
}
