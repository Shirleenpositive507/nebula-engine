#include "TexturePacker.h"
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>

namespace nebula {

    struct Node {
        sf::IntRect rect;
        Node* child[2] = {};
        bool occupied = false;
    };

    static Node* insertNode(Node* node, int w, int h, int padding) {
        if (!node) return nullptr;
        if (node->occupied) {
            Node* r = insertNode(node->child[0], w, h, padding);
            if (r) return r;
            return insertNode(node->child[1], w, h, padding);
        }
        int rw = node->rect.width;
        int rh = node->rect.height;
        if (rw < w + padding || rh < h + padding) return nullptr;
        if (rw == w + padding && rh == h + padding) {
            node->occupied = true;
            return node;
        }
        int dw = rw - w - padding;
        int dh = rh - h - padding;
        node->child[0] = new Node();
        node->child[1] = new Node();
        if (dw > dh) {
            node->child[0]->rect = {node->rect.left, node->rect.top, w + padding, rh};
            node->child[1]->rect = {node->rect.left + w + padding, node->rect.top, rw - w - padding, rh};
        } else {
            node->child[0]->rect = {node->rect.left, node->rect.top, rw, h + padding};
            node->child[1]->rect = {node->rect.left, node->rect.top + h + padding, rw, rh - h - padding};
        }
        return insertNode(node->child[0], w, h, padding);
    }

    static void freeTree(Node* node) {
        if (!node) return;
        freeTree(node->child[0]);
        freeTree(node->child[1]);
        delete node;
    }

    void TexturePacker::addTexture(const std::string& name, const std::string& path) {
        sf::Image img;
        if (img.loadFromFile(path)) {
            m_images.emplace_back(name, std::move(img));
        }
    }

    bool TexturePacker::pack(int atlasSize, int padding) {
        if (m_images.empty()) return false;

        std::vector<size_t> indices(m_images.size());
        for (size_t i = 0; i < indices.size(); i++) indices[i] = i;

        std::sort(indices.begin(), indices.end(), [&](size_t a, size_t b) {
            int ha = m_images[a].second.getSize().y;
            int hb = m_images[b].second.getSize().y;
            if (ha != hb) return ha > hb;
            return m_images[a].second.getSize().x > m_images[b].second.getSize().x;
        });

        Node* root = new Node();
        root->rect = {0, 0, atlasSize, atlasSize};

        m_packed.clear();

        sf::Image atlas;
        atlas.create(atlasSize, atlasSize, sf::Color::Transparent);

        for (size_t i : indices) {
            auto& [name, img] = m_images[i];
            sf::Vector2u size = img.getSize();
            Node* node = insertNode(root, size.x, size.y, padding);
            if (!node) {
                freeTree(root);
                return false;
            }
            PackedTexture pt;
            pt.name = name;
            pt.region = {node->rect.left, node->rect.top, (int)size.x, (int)size.y};
            pt.rotated = false;
            m_packed[name] = pt;
            atlas.copy(img, node->rect.left, node->rect.top);
        }

        m_atlas = std::make_shared<sf::Texture>();
        m_atlas->loadFromImage(atlas);

        freeTree(root);
        return true;
    }

    std::shared_ptr<sf::Texture> TexturePacker::getAtlas() const {
        return m_atlas;
    }

    const TexturePacker::PackedTexture& TexturePacker::getPacked(const std::string& name) const {
        static PackedTexture empty;
        auto it = m_packed.find(name);
        if (it != m_packed.end()) return it->second;
        return empty;
    }

    bool TexturePacker::hasTexture(const std::string& name) const {
        return m_packed.find(name) != m_packed.end();
    }

    void TexturePacker::clear() {
        m_images.clear();
        m_packed.clear();
        m_atlas.reset();
    }

    void TexturePacker::saveAtlas(const std::string& path) {
        if (m_atlas) {
            m_atlas->copyToImage().saveToFile(path);
        }
    }

    void TexturePacker::saveJSON(const std::string& path) {
        nlohmann::json j;
        for (auto& [name, pt] : m_packed) {
            j[name] = {
                {"x", pt.region.left},
                {"y", pt.region.top},
                {"w", pt.region.width},
                {"h", pt.region.height},
                {"rotated", pt.rotated}
            };
        }
        std::ofstream file(path);
        file << j.dump(4);
    }
}
