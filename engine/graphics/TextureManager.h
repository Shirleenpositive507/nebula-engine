#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Image.hpp>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <cstddef>
#include <future>
#include <functional>
#include <map>

namespace nebula {
    namespace graphics {

        struct AtlasRegion {
            std::string name;
            sf::IntRect rect;
            bool rotated;

            AtlasRegion() : rect(), rotated(false) {}
            AtlasRegion(const std::string& n, const sf::IntRect& r, bool rot = false)
                : name(n), rect(r), rotated(rot) {}
        };

        struct AtlasPage {
            std::shared_ptr<sf::Texture> texture;
            std::vector<AtlasRegion> regions;
            unsigned int width;
            unsigned int height;
            bool packed;

            AtlasPage() : width(0), height(0), packed(false) {}
        };

        enum class TextureCompression {
            None,
            DXT1,
            DXT3,
            DXT5,
            ETC2,
            ASTC
        };

        enum class MipmapMode {
            None,
            Linear,
            Nearest,
            Trilinear
        };

        class TextureManager {
        public:
            static TextureManager& getInstance();

            std::shared_ptr<sf::Texture> loadTexture(const std::string& name, const std::string& filepath);
            std::shared_ptr<sf::Texture> getTexture(const std::string& name) const;
            bool unloadTexture(const std::string& name);
            bool reloadTexture(const std::string& name);
            bool hasTexture(const std::string& name) const;

            std::vector<std::string> getTextureList() const;

            void setDefaultFiltering(sf::Texture::FilterMode mode);
            void setTexturePacking(bool enabled);
            bool isTexturePackingEnabled() const;

            std::shared_ptr<sf::RenderTexture> createRenderTexture(const std::string& name, unsigned int width, unsigned int height, bool depthBuffer = false);
            std::shared_ptr<sf::RenderTexture> getRenderTexture(const std::string& name) const;
            bool destroyRenderTexture(const std::string& name);

            std::size_t getMemoryUsage() const;
            void clearUnused();
            void setMaxTextureSize(unsigned int maxSize);
            unsigned int getMaxTextureSize() const;

            // Texture atlas support
            int createAtlasPage(const std::string& name, unsigned int width, unsigned int height);
            bool packTextureIntoAtlas(const std::string& atlasName, const std::string& textureName, const std::string& filepath);
            std::shared_ptr<sf::Texture> getAtlasPage(const std::string& name) const;
            const AtlasRegion* getAtlasRegion(const std::string& atlasName, const std::string& regionName) const;
            std::vector<std::string> getAtlasNames() const;

            // Compression detection
            TextureCompression detectCompression(const std::string& filepath) const;
            void setCompressionMode(TextureCompression mode);
            TextureCompression getCompressionMode() const;

            // Async streaming
            std::future<std::shared_ptr<sf::Texture>> loadTextureAsync(const std::string& name, const std::string& filepath);
            bool isLoaded(const std::string& name) const;
            void waitForLoad(const std::string& name);

            // Mipmap generation
            void setMipmapMode(MipmapMode mode);
            MipmapMode getMipmapMode() const;
            void generateMipmaps(const std::string& name);
            void generateMipmapsAll();

            // Anisotropy filtering
            void setAnisotropyLevel(unsigned int level);
            unsigned int getAnisotropyLevel() const;

            void releaseAll();

        private:
            TextureManager();
            ~TextureManager();
            TextureManager(const TextureManager&) = delete;
            TextureManager& operator=(const TextureManager&) = delete;

            struct TextureEntry {
                std::shared_ptr<sf::Texture> texture;
                std::string filepath;
                bool isRenderTexture;
            };

            struct RenderTextureEntry {
                std::shared_ptr<sf::RenderTexture> texture;
                unsigned int width;
                unsigned int height;
            };

            std::unordered_map<std::string, TextureEntry> m_textures;
            std::unordered_map<std::string, RenderTextureEntry> m_renderTextures;
            std::unordered_map<std::string, AtlasPage> m_atlasPages;
            sf::Texture::FilterMode m_defaultFiltering;
            bool m_texturePacking;
            unsigned int m_maxTextureSize;
            TextureCompression m_compressionMode;
            MipmapMode m_mipmapMode;
            unsigned int m_anisotropyLevel;
        };

    }
}
