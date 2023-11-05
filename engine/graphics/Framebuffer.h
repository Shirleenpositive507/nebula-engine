#pragma once

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Image.hpp>
#include <vector>
#include <memory>

namespace nebula {
namespace graphics {

struct FramebufferAttachment {
    sf::Texture texture;
    unsigned int width = 0;
    unsigned int height = 0;
    bool isDepth = false;
    bool isStencil = false;
};

class Framebuffer {
public:
    Framebuffer();
    ~Framebuffer();

    bool create(unsigned int width, unsigned int height);
    void destroy();

    void bind();
    void unbind();
    bool isBound() const { return m_bound; }

    unsigned int attachTexture(sf::Texture& texture);
    unsigned int attachTexture(sf::Texture& texture, unsigned int attachmentIndex);
    unsigned int attachRenderbuffer(bool depth, bool stencil);
    unsigned int attachDepthStencil();

    bool checkComplete() const;

    void resize(unsigned int width, unsigned int height);
    void blit(Framebuffer& target, unsigned int width, unsigned int height);

    unsigned int getWidth() const { return m_width; }
    unsigned int getHeight() const { return m_height; }
    unsigned int getId() const { return m_fboId; }

    unsigned int getColorAttachmentCount() const { return static_cast<unsigned int>(m_colorAttachments.size()); }
    sf::Texture& getColorAttachment(unsigned int index = 0);

    bool hasDepth() const { return m_hasDepth; }
    bool hasStencil() const { return m_hasStencil; }

    sf::RenderTexture& getSFMLTarget() { return m_renderTexture; }

private:
    unsigned int m_fboId = 0;
    unsigned int m_rboId = 0;
    unsigned int m_width = 0;
    unsigned int m_height = 0;
    bool m_bound = false;
    bool m_hasDepth = false;
    bool m_hasStencil = false;

    std::vector<FramebufferAttachment> m_colorAttachments;
    sf::RenderTexture m_renderTexture;
};

}
}

