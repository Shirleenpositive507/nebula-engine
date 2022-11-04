#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <functional>

namespace nebula {
namespace tools {
namespace editor {

struct SelectableObject {
    int id;
    sf::FloatRect bounds;
    bool selected;
};

class EditorViewport {
public:
    EditorViewport();
    ~EditorViewport();

    void init(unsigned int width, unsigned int height);
    void update(float dt);
    void render(const std::vector<SelectableObject>& objects);
    void handleInput(const sf::Event& event);
    void shutdown();

    void setViewRect(const sf::FloatRect& rect);
    sf::FloatRect getViewRect() const;

    void enableGrid(bool enable);
    bool isGridEnabled() const;

    void enableSnapToGrid(bool enable);
    bool isSnapToGridEnabled() const;

    void setSnapSize(float size);
    float getSnapSize() const;

    void enableRulers(bool enable);
    bool isRulersEnabled() const;

    std::vector<int> getSelectedObjectIds() const;
    void clearSelection();

    sf::Vector2f screenToWorld(const sf::Vector2i& screenPos) const;
    sf::Vector2i worldToScreen(const sf::Vector2f& worldPos) const;

    sf::Sprite getRenderSprite() const;

private:
    void renderGrid(sf::RenderWindow& window);
    void renderOriginAxes(sf::RenderWindow& window);
    void renderSelection(sf::RenderWindow& window, const std::vector<SelectableObject>& objects);
    void renderRulers(sf::RenderWindow& window);
    void renderPixelCoordinates(sf::RenderWindow& window);

    void handleCameraPan(const sf::Event& event);
    void handleCameraZoom(const sf::Event& event);
    void handleObjectSelection(const sf::Event& event, const std::vector<SelectableObject>& objects);

    sf::RenderTexture m_renderTexture;
    sf::View m_view;

    sf::Vector2f m_cameraPos;
    float m_zoomLevel;

    bool m_gridEnabled;
    bool m_snapToGrid;
    float m_snapSize;
    bool m_rulersEnabled;

    bool m_panning;
    sf::Vector2i m_lastMousePos;
    bool m_selecting;
    sf::FloatRect m_selectionRect;

    std::vector<int> m_selectedIds;

    sf::Font m_font;
};

} // namespace editor
} // namespace tools
} // namespace nebula
