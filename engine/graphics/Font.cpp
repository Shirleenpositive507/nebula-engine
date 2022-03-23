#include "Font.h"
#include <algorithm>
#include <stdexcept>

namespace nebula {
    namespace graphics {

        Font::Font()
            : m_name("unnamed")
            , m_valid(false) {}

        Font::Font(const std::string& name)
            : m_name(name)
            , m_valid(false) {}

        bool Font::loadFromFile(const std::string& filepath) {
            m_filepath = filepath;
            m_valid = m_font.loadFromFile(filepath);
            return m_valid;
        }

        bool Font::loadFromMemory(const void* data, std::size_t size) {
            m_filepath = "";
            m_valid = m_font.loadFromMemory(data, size);
            return m_valid;
        }

        sf::Font& Font::getFont() {
            return m_font;
        }

        const sf::Font& Font::getFont() const {
            return m_font;
        }

        sf::Glyph Font::getGlyph(uint32_t codePoint, unsigned int characterSize, bool bold) const {
            return m_font.getGlyph(codePoint, characterSize, bold);
        }

        float Font::getKerning(uint32_t first, uint32_t second, unsigned int characterSize) const {
            return m_font.getKerning(first, second, characterSize);
        }

        float Font::getLineSpacing(unsigned int characterSize) const {
            return m_font.getLineSpacing(characterSize);
        }

        float Font::getUnderlinePosition(unsigned int characterSize) const {
            return m_font.getUnderlinePosition(characterSize);
        }

        float Font::getUnderlineThickness(unsigned int characterSize) const {
            return m_font.getUnderlineThickness(characterSize);
        }

        sf::Vector2u Font::getTextureSize(unsigned int characterSize) const {
            return m_font.getTexture(characterSize).getSize();
        }

        bool Font::hasGlyph(uint32_t codePoint) const {
            return m_font.hasGlyph(codePoint);
        }

        void Font::setSmooth(bool smooth) {
            const_cast<sf::Texture&>(m_font.getTexture(30)).setSmooth(smooth);
        }

        bool Font::isSmooth() const {
            return m_font.getTexture(30).isSmooth();
        }

        const std::string& Font::getName() const {
            return m_name;
        }

        const std::string& Font::getFilepath() const {
            return m_filepath;
        }

        bool Font::isValid() const {
            return m_valid;
        }

        FontManager& FontManager::getInstance() {
            static FontManager instance;
            return instance;
        }

        FontManager::FontManager()
            : m_defaultFont("") {}

        FontManager::~FontManager() {
            releaseAll();
        }

        std::shared_ptr<Font> FontManager::loadFont(const std::string& name, const std::string& filepath) {
            auto font = std::make_shared<Font>(name);
            if (!font->loadFromFile(filepath)) {
                return nullptr;
            }
            m_fonts[name] = font;
            if (m_defaultFont.empty()) {
                m_defaultFont = name;
            }
            return font;
        }

        std::shared_ptr<Font> FontManager::getFont(const std::string& name) const {
            auto it = m_fonts.find(name);
            return it != m_fonts.end() ? it->second : nullptr;
        }

        bool FontManager::unloadFont(const std::string& name) {
            auto it = m_fonts.find(name);
            if (it == m_fonts.end()) return false;

            if (m_defaultFont == name) {
                m_defaultFont = m_fonts.empty() ? "" : m_fonts.begin()->first;
            }

            m_fonts.erase(it);
            return true;
        }

        bool FontManager::hasFont(const std::string& name) const {
            return m_fonts.find(name) != m_fonts.end();
        }

        void FontManager::preloadDefault() {
            auto& instance = getInstance();
            const std::string defaultName = "default";

            if (!instance.hasFont(defaultName)) {
                auto font = std::make_shared<Font>(defaultName);

                #ifdef _WIN32
                    const std::string defaultPath = "C:/Windows/Fonts/arial.ttf";
                #elif defined(__APPLE__)
                    const std::string defaultPath = "/System/Library/Fonts/Helvetica.ttc";
                #else
                    const std::string defaultPath = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
                #endif

                if (font->loadFromFile(defaultPath)) {
                    instance.m_fonts[defaultName] = font;
                    instance.m_defaultFont = defaultName;
                }
            }
        }

        std::shared_ptr<Font> FontManager::getDefaultFont() const {
            if (m_defaultFont.empty()) return nullptr;
            auto it = m_fonts.find(m_defaultFont);
            return it != m_fonts.end() ? it->second : nullptr;
        }

        std::vector<std::string> FontManager::getFontList() const {
            std::vector<std::string> names;
            names.reserve(m_fonts.size());
            for (const auto& pair : m_fonts) {
                names.push_back(pair.first);
            }
            return names;
        }

        void FontManager::releaseAll() {
            m_fonts.clear();
            m_defaultFont = "";
        }

    }
}
