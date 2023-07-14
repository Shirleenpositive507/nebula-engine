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
    , m_renderMode(ViewportRenderMode::Shaded)
    , m_gizmoEnabled(true)
    , m_activeCameraIndex(-1)
    , m_nextCameraId(0)
{
    m_gizmo.active = false;
    m_gizmo.type = 0;
    m_gizmo.axis = -1;
    m_gizmo.dragging = false;
    m_gizmo.snapStep = m_snapSize;
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
    if (m_gizmo.dragging) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(m_renderTexture);
        m_gizmo.currentPos = screenToWorld(mousePos);
    }
}

void EditorViewport::render(const std::vector<SelectableObject>& objects) {
    sf::Color clearColor;
    switch (m_renderMode) {
        case ViewportRenderMode::Wireframe:
            clearColor = sf::Color(20, 20, 20);
            break;
        case ViewportRenderMode::LightingOnly:
            clearColor = sf::Color(10, 10, 30);
            break;
        case ViewportRenderMode::CollisionDebug:
            clearColor = sf::Color(15, 15, 25);
            break;
        default:
            clearColor = sf::Color(60, 60, 60);
            break;
    }

    m_renderTexture.clear(clearColor);
    m_renderTexture.setView(m_view);

    if (m_gridEnabled) {
        renderGrid(m_renderTexture);
    }

    renderOriginAxes(m_renderTexture);

    if (m_renderMode == ViewportRenderMode::Wireframe) {
        renderWireframeOverlay(m_renderTexture);
    }

    renderSelection(m_renderTexture, objects);

    if (m_gizmoEnabled && !m_selectedIds.empty()) {
        renderGizmo(m_renderTexture);
    }

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

void EditorViewport::renderGizmo(sf::RenderWindow& window) {
    sf::Vector2f center = m_gizmo.currentPos;
    if (!m_gizmo.dragging) {
        center = sf::Vector2f(0, 0);
    }

    float gizmoLength = 40.0f * m_zoomLevel;
    float arrowSize = 8.0f * m_zoomLevel;

    auto drawAxis = [&](sf::Vector2f dir, sf::Color color, bool highlighted) {
        sf::Vector2f end = center + dir * gizmoLength;
        sf::Vertex line[] = {
            sf::Vertex(center, highlighted ? sf::Color::White : color),
            sf::Vertex(end, highlighted ? sf::Color::White : color)
        };
        window.draw(line, 2, sf::Lines);

        sf::ConvexShape arrow(3);
        arrow.setPoint(0, end);
        sf::Vector2f perp(-dir.y, dir.x);
        arrow.setPoint(1, end - dir * arrowSize + perp * arrowSize * 0.5f);
        arrow.setPoint(2, end - dir * arrowSize - perp * arrowSize * 0.5f);
        arrow.setFillColor(highlighted ? sf::Color::White : color);
        window.draw(arrow);
    };

    drawAxis(sf::Vector2f(1, 0), sf::Color::Red, m_gizmo.axis == 0);
    drawAxis(sf::Vector2f(0, -1), sf::Color::Green, m_gizmo.axis == 1);

    sf::CircleStyle rotateCircle(20.0f * m_zoomLevel);
    rotateCircle.setFillColor(sf::Color::Transparent);
    rotateCircle.setOutlineColor(m_gizmo.axis == 2 ? sf::Color::White : sf::Color::Blue);
    rotateCircle.setOutlineThickness(2.0f);
    rotateCircle.setPosition(center.x - 20.0f * m_zoomLevel, center.y - 20.0f * m_zoomLevel);
    window.draw(rotateCircle);

    sf::Text gizmoLabel;
    gizmoLabel.setFont(m_font);
    gizmoLabel.setCharacterSize(10);
    gizmoLabel.setFillColor(sf::Color::White);

    gizmoLabel.setString("X");
    gizmoLabel.setPosition(center.x + gizmoLength + 2, center.y - 6);
    window.draw(gizmoLabel);

    gizmoLabel.setString("Y");
    gizmoLabel.setPosition(center.x - 4, center.y - gizmoLength - 12);
    window.draw(gizmoLabel);

    sf::CircleShape centerDot(4.0f);
    centerDot.setFillColor(sf::Color::White);
    centerDot.setPosition(center.x - 4, center.y - 4);
    window.draw(centerDot);
}

void EditorViewport::renderWireframeOverlay(sf::RenderWindow& window) {
    sf::Vector2f viewSize = m_view.getSize();
    sf::Vector2f viewCenter = m_view.getCenter();
    sf::Vector2f topLeft = viewCenter - viewSize / 2.0f;

    sf::RectangleShape border(sf::Vector2f(viewSize.x - 2, viewSize.y - 2));
    border.setPosition(topLeft + sf::Vector2f(1, 1));
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color(0, 255, 0, 80));
    border.setOutlineThickness(1.0f);
    window.draw(border);

    sf::Text wireframeLabel;
    wireframeLabel.setFont(m_font);
    wireframeLabel.setString("WIREFRAME MODE");
    wireframeLabel.setCharacterSize(14);
    wireframeLabel.setFillColor(sf::Color(0, 255, 0, 120));
    wireframeLabel.setPosition(topLeft.x + 10, topLeft.y + 30);
    window.draw(wireframeLabel);
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
    handleGizmoDrag(event);
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::W) {
            cycleRenderMode();
        }
    }
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

void EditorViewport::handleGizmoDrag(const sf::Event& event) {
    if (!m_gizmoEnabled || m_selectedIds.empty()) return;

    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mouseWorld = screenToWorld(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
            sf::Vector2f gPos = m_gizmo.currentPos;
            float threshold = 10.0f * m_zoomLevel;

            if (std::abs(mouseWorld.x - (gPos.x + 40.0f * m_zoomLevel)) < threshold &&
                std::abs(mouseWorld.y - gPos.y) < threshold) {
                m_gizmo.active = true;
                m_gizmo.axis = 0;
                m_gizmo.dragging = true;
                m_gizmo.dragStart = mouseWorld;
            } else if (std::abs(mouseWorld.x - gPos.x) < threshold &&
                       std::abs(mouseWorld.y - (gPos.y - 40.0f * m_zoomLevel)) < threshold) {
                m_gizmo.active = true;
                m_gizmo.axis = 1;
                m_gizmo.dragging = true;
                m_gizmo.dragStart = mouseWorld;
            }
        }
    } else if (event.type == sf::Event::MouseButtonReleased) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            m_gizmo.dragging = false;
            m_gizmo.active = false;
            m_gizmo.axis = -1;
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
    m_gizmo.snapStep = m_snapSize;
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

void EditorViewport::setRenderMode(ViewportRenderMode mode) {
    m_renderMode = mode;
}

ViewportRenderMode EditorViewport::getRenderMode() const {
    return m_renderMode;
}

void EditorViewport::cycleRenderMode() {
    int mode = static_cast<int>(m_renderMode);
    mode = (mode + 1) % 5;
    m_renderMode = static_cast<ViewportRenderMode>(mode);
}

void EditorViewport::enableGizmo(bool enable) {
    m_gizmoEnabled = enable;
    if (!enable) {
        m_gizmo.active = false;
        m_gizmo.dragging = false;
    }
}

bool EditorViewport::isGizmoEnabled() const {
    return m_gizmoEnabled;
}

void EditorViewport::setGizmoType(int type) {
    m_gizmo.type = type;
}

int EditorViewport::getGizmoType() const {
    return m_gizmo.type;
}

GizmoState& EditorViewport::getGizmoState() {
    return m_gizmo;
}

void EditorViewport::snapToGrid(sf::Vector2f& pos) const {
    if (!m_snapToGrid) return;
    pos.x = std::round(pos.x / m_snapSize) * m_snapSize;
    pos.y = std::round(pos.y / m_snapSize) * m_snapSize;
}

int EditorViewport::addCamera(const std::string& name) {
    ViewportCamera cam;
    cam.name = name;
    cam.position = sf::Vector3f(0, 0, 10);
    cam.target = sf::Vector3f(0, 0, 0);
    cam.fov = 60.0f;
    cam.nearPlane = 0.1f;
    cam.farPlane = 1000.0f;
    cam.active = m_cameras.empty();
    m_cameras.push_back(cam);
    if (m_activeCameraIndex < 0) m_activeCameraIndex = 0;
    return m_nextCameraId++;
}

void EditorViewport::removeCamera(int cameraId) {
    if (cameraId < 0 || cameraId >= static_cast<int>(m_cameras.size())) return;
    m_cameras.erase(m_cameras.begin() + cameraId);
    if (m_activeCameraIndex >= static_cast<int>(m_cameras.size())) {
        m_activeCameraIndex = static_cast<int>(m_cameras.size()) - 1;
    }
}

void EditorViewport::setActiveCamera(int cameraId) {
    for (size_t i = 0; i < m_cameras.size(); ++i) {
        m_cameras[i].active = (static_cast<int>(i) == cameraId);
    }
    m_activeCameraIndex = cameraId;
}

ViewportCamera* EditorViewport::getActiveCamera() {
    if (m_activeCameraIndex >= 0 && m_activeCameraIndex < static_cast<int>(m_cameras.size())) {
        return &m_cameras[m_activeCameraIndex];
    }
    return nullptr;
}

size_t EditorViewport::getCameraCount() const {
    return m_cameras.size();
}

sf::Vector2f EditorViewport::getGizmoPosition() const {
    return m_gizmo.currentPos;
}

void EditorViewport::setGizmoPosition(const sf::Vector2f& pos) {
    m_gizmo.currentPos = pos;
}

void EditorViewport::shutdown() {
    m_selectedIds.clear();
    m_cameras.clear();
}

} // namespace editor
} // namespace tools
} // namespace nebula
