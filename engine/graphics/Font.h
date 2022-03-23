#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Glyph.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

namespace nebula {
    namespace graphics {

        class Font {
        public:
            Font();
            explicit Font(const std::string& name);
            ~Font() = default;

            bool loadFromFile(const std::string& filepath);
            bool loadFromMemory(const void* data, std::size_t size);

            sf::Font& getFont();
            const sf::Font& getFont() const;

            sf::Glyph getGlyph(uint32_t codePoint, unsigned int characterSize, bool bold = false) const;
            float getKerning(uint32_t first, uint32_t second, unsigned int characterSize = 30) const;
            float getLineSpacing(unsigned int characterSize = 30) const;
            float getUnderlinePosition(unsigned int characterSize = 30) const;
            float getUnderlineThickness(unsigned int characterSize = 30) const;

            sf::Vector2u getTextureSize(unsigned int characterSize = 30) const;
            bool hasGlyph(uint32_t codePoint) const;

            void setSmooth(bool smooth);
            bool isSmooth() const;

            const std::string& getName() const;
            const std::string& getFilepath() const;
            bool isValid() const;

        private:
            std::string m_name;
            std::string m_filepath;
            sf::Font m_font;
            bool m_valid;
        };

        class FontManager {
        public:
            static FontManager& getInstance();

            std::shared_ptr<Font> loadFont(const std::string& name, const std::string& filepath);
            std::shared_ptr<Font> getFont(const std::string& name) const;
            bool unloadFont(const std::string& name);
            bool hasFont(const std::string& name) const;

            void preloadDefault();
            std::shared_ptr<Font> getDefaultFont() const;

            std::vector<std::string> getFontList() const;
            void releaseAll();

        private:
            FontManager();
            ~FontManager();
            FontManager(const FontManager&) = delete;
            FontManager& operator=(const FontManager&) = delete;

            std::unordered_map<std::string, std::shared_ptr<Font>> m_fonts;
            std::string m_defaultFont;
        };

    }
}
