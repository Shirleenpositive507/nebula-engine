#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

namespace nebula {
namespace tools {
namespace editor {

struct SelectableObject {
    int id;
    sf::FloatRect bounds;
    bool selected;
};

enum class ViewportRenderMode {
    Shaded,
    Wireframe,
    LightingOnly,
    CollisionDebug,
    ShadedWithGrid,
    MaterialPreview
};

struct GizmoState {
    bool active;
    int type;
    int axis;
    sf::Vector2f dragStart;
    sf::Vector2f currentPos;
    bool dragging;
    float snapStep;
};

struct ViewportCamera {
    std::string name;
    sf::Vector3f position;
    sf::Vector3f target;
    float fov;
    float nearPlane;
    float farPlane;
    bool active;
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

    void setRenderMode(ViewportRenderMode mode);
    ViewportRenderMode getRenderMode() const;
    void cycleRenderMode();

    void enableGizmo(bool enable);
    bool isGizmoEnabled() const;
    void setGizmoType(int type);
    int getGizmoType() const;
    GizmoState& getGizmoState();
    void snapToGrid(sf::Vector2f& pos) const;

    int addCamera(const std::string& name);
    void removeCamera(int cameraId);
    void setActiveCamera(int cameraId);
    ViewportCamera* getActiveCamera();
    size_t getCameraCount() const;

    sf::Vector2f getGizmoPosition() const;
    void setGizmoPosition(const sf::Vector2f& pos);

private:
    void renderGrid(sf::RenderWindow& window);
    void renderOriginAxes(sf::RenderWindow& window);
    void renderSelection(sf::RenderWindow& window, const std::vector<SelectableObject>& objects);
    void renderRulers(sf::RenderWindow& window);
    void renderPixelCoordinates(sf::RenderWindow& window);
    void renderGizmo(sf::RenderWindow& window);
    void renderWireframeOverlay(sf::RenderWindow& window);

    void handleCameraPan(const sf::Event& event);
    void handleCameraZoom(const sf::Event& event);
    void handleObjectSelection(const sf::Event& event, const std::vector<SelectableObject>& objects);
    void handleGizmoDrag(const sf::Event& event);

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

    ViewportRenderMode m_renderMode;
    GizmoState m_gizmo;
    bool m_gizmoEnabled;

    std::vector<ViewportCamera> m_cameras;
    int m_activeCameraIndex;
    int m_nextCameraId;
};

} // namespace editor
} // namespace tools
} // namespace nebula
