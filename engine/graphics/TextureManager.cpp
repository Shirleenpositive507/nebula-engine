#include "TextureManager.h"
#include <algorithm>

namespace nebula {
    namespace graphics {

        TextureManager::TextureManager()
            : m_defaultFiltering(sf::Texture::FilterMode::Trilinear)
            , m_texturePacking(false)
            , m_maxTextureSize(8192) {}

        TextureManager::~TextureManager() {
            releaseAll();
        }

        TextureManager& TextureManager::getInstance() {
            static TextureManager instance;
            return instance;
        }

        std::shared_ptr<sf::Texture> TextureManager::loadTexture(const std::string& name, const std::string& filepath) {
            auto it = m_textures.find(name);
            if (it != m_textures.end()) {
                return it->second.texture;
            }

            auto texture = std::make_shared<sf::Texture>();
            if (!texture->loadFromFile(filepath)) {
                return nullptr;
            }

            texture->setFilterMode(m_defaultFiltering);
            if (m_maxTextureSize > 0) {
                sf::Vector2u size = texture->getSize();
                if (size.x > m_maxTextureSize || size.y > m_maxTextureSize) {
                    float scale = static_cast<float>(m_maxTextureSize) / std::max(size.x, size.y);
                    sf::Image img = texture->copyToImage();
                    sf::Vector2u newSize(
                        static_cast<unsigned int>(static_cast<float>(size.x) * scale),
                        static_cast<unsigned int>(static_cast<float>(size.y) * scale)
                    );
                    // Resize would need a custom implementation
                }
            }

            TextureEntry entry;
            entry.texture = texture;
            entry.filepath = filepath;
            entry.isRenderTexture = false;
            m_textures[name] = entry;

            return texture;
        }

        std::shared_ptr<sf::Texture> TextureManager::getTexture(const std::string& name) const {
            auto it = m_textures.find(name);
            if (it != m_textures.end()) {
                return it->second.texture;
            }
            return nullptr;
        }

        bool TextureManager::unloadTexture(const std::string& name) {
            auto it = m_textures.find(name);
            if (it == m_textures.end()) {
                return false;
            }
            m_textures.erase(it);
            return true;
        }

        bool TextureManager::reloadTexture(const std::string& name) {
            auto it = m_textures.find(name);
            if (it == m_textures.end() || it->second.isRenderTexture) {
                return false;
            }

            return it->second.texture->loadFromFile(it->second.filepath);
        }

        bool TextureManager::hasTexture(const std::string& name) const {
            return m_textures.find(name) != m_textures.end();
        }

        std::vector<std::string> TextureManager::getTextureList() const {
            std::vector<std::string> names;
            names.reserve(m_textures.size());
            for (const auto& pair : m_textures) {
                names.push_back(pair.first);
            }
            return names;
        }

        void TextureManager::setDefaultFiltering(sf::Texture::FilterMode mode) {
            m_defaultFiltering = mode;
            for (auto& pair : m_textures) {
                pair.second.texture->setFilterMode(mode);
            }
        }

        void TextureManager::setTexturePacking(bool enabled) {
            m_texturePacking = enabled;
        }

        bool TextureManager::isTexturePackingEnabled() const {
            return m_texturePacking;
        }

        std::shared_ptr<sf::RenderTexture> TextureManager::createRenderTexture(
            const std::string& name, unsigned int width, unsigned int height, bool depthBuffer)
        {
            auto rt = std::make_shared<sf::RenderTexture>();
            sf::ContextSettings settings;
            settings.depthBits = depthBuffer ? 24 : 0;
            if (!rt->create(width, height, settings)) {
                return nullptr;
            }

            RenderTextureEntry entry;
            entry.texture = rt;
            entry.width = width;
            entry.height = height;
            m_renderTextures[name] = entry;

            return rt;
        }

        std::shared_ptr<sf::RenderTexture> TextureManager::getRenderTexture(const std::string& name) const {
            auto it = m_renderTextures.find(name);
            if (it != m_renderTextures.end()) {
                return it->second.texture;
            }
            return nullptr;
        }

        bool TextureManager::destroyRenderTexture(const std::string& name) {
            auto it = m_renderTextures.find(name);
            if (it == m_renderTextures.end()) {
                return false;
            }
            m_renderTextures.erase(it);
            return true;
        }

        std::size_t TextureManager::getMemoryUsage() const {
            std::size_t total = 0;
            for (const auto& pair : m_textures) {
                sf::Vector2u size = pair.second.texture->getSize();
                total += static_cast<std::size_t>(size.x) * static_cast<std::size_t>(size.y) * 4;
            }
            for (const auto& pair : m_renderTextures) {
                total += static_cast<std::size_t>(pair.second.width) *
                         static_cast<std::size_t>(pair.second.height) * 4;
            }
            return total;
        }

        void TextureManager::clearUnused() {
            for (auto it = m_textures.begin(); it != m_textures.end(); ) {
                if (it->second.texture.use_count() == 1) {
                    it = m_textures.erase(it);
                } else {
                    ++it;
                }
            }
        }

        void TextureManager::setMaxTextureSize(unsigned int maxSize) {
            m_maxTextureSize = maxSize;
        }

        unsigned int TextureManager::getMaxTextureSize() const {
            return m_maxTextureSize;
        }

        void TextureManager::releaseAll() {
            m_textures.clear();
            m_renderTextures.clear();
        }

    }
}
