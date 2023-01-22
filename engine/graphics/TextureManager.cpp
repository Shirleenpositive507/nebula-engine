#include "TextureManager.h"
#include <algorithm>
#include <thread>

namespace nebula {
    namespace graphics {

        TextureManager::TextureManager()
            : m_defaultFiltering(sf::Texture::FilterMode::Trilinear)
            , m_texturePacking(false)
            , m_maxTextureSize(8192)
            , m_compressionMode(TextureCompression::None)
            , m_mipmapMode(MipmapMode::Linear)
            , m_anisotropyLevel(1) {}

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
            for (const auto& pair : m_atlasPages) {
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

        // --- Atlas support ---

        int TextureManager::createAtlasPage(const std::string& name, unsigned int width, unsigned int height) {
            AtlasPage page;
            page.width = width;
            page.height = height;
            page.packed = false;
            page.texture = std::make_shared<sf::Texture>();
            sf::Image img;
            img.create(width, height, sf::Color(0, 0, 0, 0));
            if (!page.texture->loadFromImage(img)) {
                return -1;
            }
            m_atlasPages[name] = page;
            return static_cast<int>(m_atlasPages.size() - 1);
        }

        bool TextureManager::packTextureIntoAtlas(const std::string& atlasName, const std::string& textureName, const std::string& filepath) {
            auto it = m_atlasPages.find(atlasName);
            if (it == m_atlasPages.end()) return false;

            sf::Image srcImg;
            if (!srcImg.loadFromFile(filepath)) return false;
            AtlasPage& page = it->second;
            sf::Image pageImg = page.texture->copyToImage();

            sf::Vector2u srcSize = srcImg.getSize();
            unsigned int x = 0, y = 0;
            for (const auto& region : page.regions) {
                x = std::max(x, static_cast<unsigned int>(region.rect.left + region.rect.width));
                y = std::max(y, static_cast<unsigned int>(region.rect.top + region.rect.height));
            }
            if (x + srcSize.x > page.width) {
                x = 0;
                y += srcSize.y;
            }
            if (y + srcSize.y > page.height) return false;

            pageImg.copy(srcImg, x, y, sf::IntRect(0, 0, srcSize.x, srcSize.y));
            page.texture->loadFromImage(pageImg);

            sf::IntRect rect(static_cast<int>(x), static_cast<int>(y),
                             static_cast<int>(srcSize.x), static_cast<int>(srcSize.y));
            AtlasRegion region(textureName, rect);
            page.regions.push_back(region);
            page.packed = true;
            return true;
        }

        std::shared_ptr<sf::Texture> TextureManager::getAtlasPage(const std::string& name) const {
            auto it = m_atlasPages.find(name);
            if (it != m_atlasPages.end()) {
                return it->second.texture;
            }
            return nullptr;
        }

        const AtlasRegion* TextureManager::getAtlasRegion(const std::string& atlasName, const std::string& regionName) const {
            auto it = m_atlasPages.find(atlasName);
            if (it == m_atlasPages.end()) return nullptr;
            for (const auto& region : it->second.regions) {
                if (region.name == regionName) return &region;
            }
            return nullptr;
        }

        std::vector<std::string> TextureManager::getAtlasNames() const {
            std::vector<std::string> names;
            names.reserve(m_atlasPages.size());
            for (const auto& pair : m_atlasPages) {
                names.push_back(pair.first);
            }
            return names;
        }

        // --- Compression ---

        TextureCompression TextureManager::detectCompression(const std::string& filepath) const {
            std::string ext = filepath.substr(filepath.find_last_of('.'));
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == ".dds") return TextureCompression::DXT5;
            if (ext == ".pvr") return TextureCompression::ETC2;
            if (ext == ".astc") return TextureCompression::ASTC;
            return TextureCompression::None;
        }

        void TextureManager::setCompressionMode(TextureCompression mode) {
            m_compressionMode = mode;
        }

        TextureCompression TextureManager::getCompressionMode() const {
            return m_compressionMode;
        }

        // --- Async loading ---

        std::future<std::shared_ptr<sf::Texture>> TextureManager::loadTextureAsync(const std::string& name, const std::string& filepath) {
            return std::async(std::launch::async, [this, name, filepath]() {
                return loadTexture(name, filepath);
            });
        }

        bool TextureManager::isLoaded(const std::string& name) const {
            return m_textures.find(name) != m_textures.end();
        }

        void TextureManager::waitForLoad(const std::string& name) {
            while (!isLoaded(name)) {
                std::this_thread::yield();
            }
        }

        // --- Mipmaps ---

        void TextureManager::setMipmapMode(MipmapMode mode) {
            m_mipmapMode = mode;
        }

        MipmapMode TextureManager::getMipmapMode() const {
            return m_mipmapMode;
        }

        void TextureManager::generateMipmaps(const std::string& name) {
            auto it = m_textures.find(name);
            if (it != m_textures.end()) {
                it->second.texture->generateMipmap();
            }
        }

        void TextureManager::generateMipmapsAll() {
            for (auto& pair : m_textures) {
                pair.second.texture->generateMipmap();
            }
        }

        // --- Anisotropy ---

        void TextureManager::setAnisotropyLevel(unsigned int level) {
            m_anisotropyLevel = std::min(level, 16u);
        }

        unsigned int TextureManager::getAnisotropyLevel() const {
            return m_anisotropyLevel;
        }

        void TextureManager::releaseAll() {
            m_textures.clear();
            m_renderTextures.clear();
            m_atlasPages.clear();
        }

    }
}
