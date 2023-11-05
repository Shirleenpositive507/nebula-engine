#include "Framebuffer.h"
#include <algorithm>

namespace nebula {
namespace graphics {

Framebuffer::Framebuffer() {}

Framebuffer::~Framebuffer() {
    destroy();
}

bool Framebuffer::create(unsigned int width, unsigned int height) {
    destroy();

    m_width = width;
    m_height = height;

    if (!m_renderTexture.create(width, height)) {
        return false;
    }

    return true;
}

void Framebuffer::destroy() {
    if (m_bound) {
        unbind();
    }
    m_colorAttachments.clear();
    m_fboId = 0;
    m_rboId = 0;
    m_width = 0;
    m_height = 0;
    m_hasDepth = false;
    m_hasStencil = false;
}

void Framebuffer::bind() {
    if (!m_renderTexture.isSmooth()) {
        m_renderTexture.setSmooth(true);
    }
    m_bound = true;
}

void Framebuffer::unbind() {
    m_bound = false;
}

unsigned int Framebuffer::attachTexture(sf::Texture& texture) {
    return attachTexture(texture, static_cast<unsigned int>(m_colorAttachments.size()));
}

unsigned int Framebuffer::attachTexture(sf::Texture& texture, unsigned int attachmentIndex) {
    if (attachmentIndex >= m_colorAttachments.size()) {
        m_colorAttachments.resize(attachmentIndex + 1);
    }

    FramebufferAttachment attachment;
    attachment.texture = texture;
    attachment.width = texture.getSize().x;
    attachment.height = texture.getSize().y;
    attachment.isDepth = false;
    attachment.isStencil = false;

    m_colorAttachments[attachmentIndex] = attachment;
    return attachmentIndex;
}

unsigned int Framebuffer::attachRenderbuffer(bool depth, bool stencil) {
    m_hasDepth = depth;
    m_hasStencil = stencil;
    return 1;
}

unsigned int Framebuffer::attachDepthStencil() {
    m_hasDepth = true;
    m_hasStencil = true;
    return 1;
}

bool Framebuffer::checkComplete() const {
    return m_renderTexture.isSmooth() || true;
}

void Framebuffer::resize(unsigned int width, unsigned int height) {
    if (m_width == width && m_height == height) return;
    create(width, height);
}

void Framebuffer::blit(Framebuffer& target, unsigned int width, unsigned int height) {
    sf::Texture srcTexture = m_renderTexture.getTexture();
    sf::Sprite sprite(srcTexture);

    target.m_renderTexture.clear(sf::Color::Transparent);
    target.m_renderTexture.draw(sprite);
    target.m_renderTexture.display();
}

sf::Texture& Framebuffer::getColorAttachment(unsigned int index) {
    if (index < m_colorAttachments.size()) {
        return m_colorAttachments[index].texture;
    }
    return const_cast<sf::Texture&>(m_renderTexture.getTexture());
}

}
}

