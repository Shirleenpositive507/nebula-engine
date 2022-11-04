#include "EditorViewport.h"
#include <sstream>
#include <algorithm>
#include <cmath>

namespace nebula {
namespace tools {
namespace editor {

EditorViewport::EditorViewport()
    : m_cameraPos(0.0f, 0.0f)
    , m_zoomLevel(1.0f)
    , m_gridEnabled(true)
    , m_snapToGrid(false)
    , m_snapSize(32.0f)
    , m_rulersEnabled(true)
    , m_panning(false)
    , m_selecting(false)
{
}

EditorViewport::~EditorViewport() {
    shutdown();
}

void EditorViewport::init(unsigned int width, unsigned int height) {
    m_renderTexture.create(width, height);
    m_view.setSize(static_cast<float>(width), static_cast<float>(height));
    m_view.setCenter(m_cameraPos);

    if (!m_font.loadFromFile("resources/fonts/consolas.ttf")) {
        m_font.loadFromFile("C:/Windows/Fonts/consola.ttf");
    }
}

void EditorViewport::update(float dt) {
}

void EditorViewport::render(const std::vector<SelectableObject>& objects) {
    m_renderTexture.clear(sf::Color(60, 60, 60));
    m_renderTexture.setView(m_view);

    if (m_gridEnabled) {
        renderGrid(m_renderTexture);
    }

    renderOriginAxes(m_renderTexture);
    renderSelection(m_renderTexture, objects);

    if (m_rulersEnabled) {
        renderRulers(m_renderTexture);
    }

    renderPixelCoordinates(m_renderTexture);
    m_renderTexture.display();
}

void EditorViewport::renderGrid(sf::RenderWindow& window) {
    sf::Vector2f viewSize = m_view.getSize();
    sf::Vector2f viewCenter = m_view.getCenter();
    sf::Vector2f topLeft = viewCenter - viewSize / 2.0f;
    sf::Vector2f bottomRight = viewCenter + viewSize / 2.0f;

    float gridSpacing = m_snapSize * m_zoomLevel;
    if (gridSpacing < 8.0f) gridSpacing = m_snapSize * 4.0f * m_zoomLevel;

    float startX = std::floor(topLeft.x / gridSpacing) * gridSpacing;
    float startY = std::floor(topLeft.y / gridSpacing) * gridSpacing;

    sf::Color gridColor(80, 80, 80, 100);
    sf::Color majorGridColor(100, 100, 100, 150);

    for (float x = startX; x <= bottomRight.x; x += gridSpacing) {
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(x, topLeft.y), gridColor),
            sf::Vertex(sf::Vector2f(x, bottomRight.y), gridColor)
        };
        window.draw(line, 2, sf::Lines);
    }

    for (float y = startY; y <= bottomRight.y; y += gridSpacing) {
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(topLeft.x, y), gridColor),
            sf::Vertex(sf::Vector2f(bottomRight.x, y), gridColor)
        };
        window.draw(line, 2, sf::Lines);
    }
}

void EditorViewport::renderOriginAxes(sf::RenderWindow& window) {
    float axisLength = 50.0f * m_zoomLevel;

    sf::Vertex xAxis[] = {
        sf::Vertex(sf::Vector2f(0, 0), sf::Color::Red),
        sf::Vertex(sf::Vector2f(axisLength, 0), sf::Color::Red)
    };
    window.draw(xAxis, 2, sf::Lines);

    sf::Vertex yAxis[] = {
        sf::Vertex(sf::Vector2f(0, 0), sf::Color::Green),
        sf::Vertex(sf::Vector2f(0, -axisLength), sf::Color::Green)
    };
    window.draw(yAxis, 2, sf::Lines);

    sf::CircleShape originDot(3.0f);
    originDot.setFillColor(sf::Color::White);
    originDot.setPosition(-3.0f, -3.0f);
    window.draw(originDot);
}

void EditorViewport::renderSelection(sf::RenderWindow& window, const std::vector<SelectableObject>& objects) {
    for (const auto& obj : objects) {
        if (!obj.selected) continue;

        sf::RectangleShape outline;
        outline.setPosition(obj.bounds.left, obj.bounds.top);
        outline.setSize(sf::Vector2f(obj.bounds.width, obj.bounds.height));
        outline.setFillColor(sf::Color::Transparent);
        outline.setOutlineColor(sf::Color(255, 200, 50));
        outline.setOutlineThickness(2.0f);
        window.draw(outline);

        sf::RectangleShape handles;
        handles.setPosition(obj.bounds.left - 3, obj.bounds.top - 3);
        handles.setSize(sf::Vector2f(6, 6));
        handles.setFillColor(sf::Color::White);
        window.draw(handles);

        handles.setPosition(obj.bounds.left + obj.bounds.width - 3, obj.bounds.top - 3);
        window.draw(handles);

        handles.setPosition(obj.bounds.left - 3, obj.bounds.top + obj.bounds.height - 3);
        window.draw(handles);

        handles.setPosition(obj.bounds.left + obj.bounds.width - 3, obj.bounds.top + obj.bounds.height - 3);
        window.draw(handles);
    }

    if (m_selecting) {
        sf::RectangleShape selectionBox;
        selectionBox.setPosition(m_selectionRect.left, m_selectionRect.top);
        selectionBox.setSize(sf::Vector2f(m_selectionRect.width, m_selectionRect.height));
        selectionBox.setFillColor(sf::Color(100, 150, 255, 50));
        selectionBox.setOutlineColor(sf::Color(100, 150, 255));
        selectionBox.setOutlineThickness(1.0f);
        window.draw(selectionBox);
    }
}

void EditorViewport::renderRulers(sf::RenderWindow& window) {
    sf::Vector2f viewSize = m_view.getSize();
    sf::Vector2f viewCenter = m_view.getCenter();
    sf::Vector2f topLeft = viewCenter - viewSize / 2.0f;

    float rulerSpacing = m_snapSize * m_zoomLevel;
    if (rulerSpacing < 20.0f) rulerSpacing = m_snapSize * 4.0f * m_zoomLevel;

    float startX = std::floor(topLeft.x / rulerSpacing) * rulerSpacing;

    for (float x = startX; x <= topLeft.x + viewSize.x; x += rulerSpacing) {
        sf::Text rulerText;
        rulerText.setFont(m_font);
        rulerText.setString(std::to_string(static_cast<int>(x)));
        rulerText.setCharacterSize(10);
        rulerText.setFillColor(sf::Color(180, 180, 180));
        rulerText.setPosition(x, topLeft.y);
        window.draw(rulerText);
    }

    float startY = std::floor(topLeft.y / rulerSpacing) * rulerSpacing;
    for (float y = startY; y <= topLeft.y + viewSize.y; y += rulerSpacing) {
        sf::Text rulerText;
        rulerText.setFont(m_font);
        rulerText.setString(std::to_string(static_cast<int>(y)));
        rulerText.setCharacterSize(10);
        rulerText.setFillColor(sf::Color(180, 180, 180));
        rulerText.setPosition(topLeft.x, y);
        window.draw(rulerText);
    }
}

void EditorViewport::renderPixelCoordinates(sf::RenderWindow& window) {
    sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
    sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, m_view);

    std::ostringstream oss;
    oss << "Pixel: (" << pixelPos.x << ", " << pixelPos.y << ")  "
        << "World: (" << std::fixed << std::setprecision(1)
        << worldPos.x << ", " << worldPos.y << ")";

    sf::Text coordText;
    coordText.setFont(m_font);
    coordText.setString(oss.str());
    coordText.setCharacterSize(12);
    coordText.setFillColor(sf::Color(200, 200, 200));
    coordText.setPosition(10.0f, 10.0f);
    window.draw(coordText);
}

void EditorViewport::handleInput(const sf::Event& event) {
    handleCameraPan(event);
    handleCameraZoom(event);
}

void EditorViewport::handleCameraPan(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Middle) {
            m_panning = true;
            m_lastMousePos = sf::Mouse::getPosition(m_renderTexture);
        }
    }
    else if (event.type == sf::Event::MouseButtonReleased) {
        if (event.mouseButton.button == sf::Mouse::Middle) {
            m_panning = false;
        }
    }
    else if (event.type == sf::Event::MouseMoved && m_panning) {
        sf::Vector2i currentPos = sf::Mouse::getPosition(m_renderTexture);
        sf::Vector2f delta = static_cast<sf::Vector2f>(currentPos - m_lastMousePos);
        delta *= m_zoomLevel;
        m_cameraPos -= delta;
        m_view.setCenter(m_cameraPos);
        m_lastMousePos = currentPos;
    }
}

void EditorViewport::handleCameraZoom(const sf::Event& event) {
    if (event.type == sf::Event::MouseWheelScrolled) {
        float zoomFactor = 1.1f;
        if (event.mouseWheelScroll.delta > 0) {
            m_zoomLevel /= zoomFactor;
        } else {
            m_zoomLevel *= zoomFactor;
        }
        m_zoomLevel = std::clamp(m_zoomLevel, 0.1f, 10.0f);

        sf::Vector2u texSize = m_renderTexture.getSize();
        m_view.setSize(static_cast<float>(texSize.x) * m_zoomLevel,
                       static_cast<float>(texSize.y) * m_zoomLevel);
    }
}

void EditorViewport::handleObjectSelection(const sf::Event& event, const std::vector<SelectableObject>& objects) {
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f clickPos = screenToWorld(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));

            if (!sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
                m_selectedIds.clear();
            }

            for (const auto& obj : objects) {
                if (obj.bounds.contains(clickPos)) {
                    m_selectedIds.push_back(obj.id);
                    break;
                }
            }

            m_selecting = true;
            m_selectionRect.left = clickPos.x;
            m_selectionRect.top = clickPos.y;
            m_selectionRect.width = 0;
            m_selectionRect.height = 0;
        }
    }
    else if (event.type == sf::Event::MouseButtonReleased) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            m_selecting = false;
        }
    }
    else if (event.type == sf::Event::MouseMoved && m_selecting) {
        sf::Vector2f currentPos = screenToWorld(sf::Mouse::getPosition(m_renderTexture));
        m_selectionRect.width = currentPos.x - m_selectionRect.left;
        m_selectionRect.height = currentPos.y - m_selectionRect.top;

        if (m_selectionRect.width < 0) {
            m_selectionRect.left += m_selectionRect.width;
            m_selectionRect.width = -m_selectionRect.width;
        }
        if (m_selectionRect.height < 0) {
            m_selectionRect.top += m_selectionRect.height;
            m_selectionRect.height = -m_selectionRect.height;
        }
    }
}

void EditorViewport::setViewRect(const sf::FloatRect& rect) {
    m_view.setCenter(rect.left + rect.width / 2, rect.top + rect.height / 2);
    m_view.setSize(rect.width, rect.height);
    m_cameraPos = sf::Vector2f(rect.left, rect.top);
}

sf::FloatRect EditorViewport::getViewRect() const {
    sf::Vector2f center = m_view.getCenter();
    sf::Vector2f size = m_view.getSize();
    return sf::FloatRect(center.x - size.x / 2, center.y - size.y / 2, size.x, size.y);
}

void EditorViewport::enableGrid(bool enable) {
    m_gridEnabled = enable;
}

bool EditorViewport::isGridEnabled() const {
    return m_gridEnabled;
}

void EditorViewport::enableSnapToGrid(bool enable) {
    m_snapToGrid = enable;
}

bool EditorViewport::isSnapToGridEnabled() const {
    return m_snapToGrid;
}

void EditorViewport::setSnapSize(float size) {
    m_snapSize = std::max(1.0f, size);
}

float EditorViewport::getSnapSize() const {
    return m_snapSize;
}

void EditorViewport::enableRulers(bool enable) {
    m_rulersEnabled = enable;
}

bool EditorViewport::isRulersEnabled() const {
    return m_rulersEnabled;
}

std::vector<int> EditorViewport::getSelectedObjectIds() const {
    return m_selectedIds;
}

void EditorViewport::clearSelection() {
    m_selectedIds.clear();
}

sf::Vector2f EditorViewport::screenToWorld(const sf::Vector2i& screenPos) const {
    return m_renderTexture.mapPixelToCoords(screenPos, m_view);
}

sf::Vector2i EditorViewport::worldToScreen(const sf::Vector2f& worldPos) const {
    return m_renderTexture.mapCoordsToPixel(worldPos, m_view);
}

sf::Sprite EditorViewport::getRenderSprite() const {
    sf::Sprite sprite(m_renderTexture.getTexture());
    return sprite;
}

void EditorViewport::shutdown() {
    m_selectedIds.clear();
}

} // namespace editor
} // namespace tools
} // namespace nebula
