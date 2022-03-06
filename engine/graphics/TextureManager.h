#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Image.hpp>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <cstddef>

namespace nebula {
    namespace graphics {

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
            sf::Texture::FilterMode m_defaultFiltering;
            bool m_texturePacking;
            unsigned int m_maxTextureSize;
        };

    }
}
