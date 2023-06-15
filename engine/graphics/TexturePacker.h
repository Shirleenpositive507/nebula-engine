#pragma once
#include "TextureManager.h"
#include <vector>
#include <string>

namespace nebula {
    class TexturePacker {
    public:
        struct PackedTexture {
            std::string name;
            sf::IntRect region;
            bool rotated;
        };
        void addTexture(const std::string& name, const std::string& path);
        bool pack(int atlasSize = 2048, int padding = 2);
        std::shared_ptr<sf::Texture> getAtlas() const;
        const PackedTexture& getPacked(const std::string& name) const;
        bool hasTexture(const std::string& name) const;
        void clear();
        void saveAtlas(const std::string& path);
        void saveJSON(const std::string& path);
    private:
        std::vector<std::pair<std::string, sf::Image>> m_images;
        std::unordered_map<std::string, PackedTexture> m_packed;
        std::shared_ptr<sf::Texture> m_atlas;
    };
}
