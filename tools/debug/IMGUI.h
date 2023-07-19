#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <stack>
#include <functional>
#include <unordered_map>
#include <set>

namespace nebula {
namespace tools {
namespace debug {

struct GUIWindow {
    std::string id;
    sf::FloatRect rect;
    bool open;
    bool collapsed;
    bool minimized;
    bool docked;
    std::string dockTarget;
    int dockSide;
    bool dragging;
    sf::Vector2f dragOffset;
    bool resizing;
    int resizeEdge;
    bool snapLeft;
    bool snapRight;
    bool snapTop;
    bool snapBottom;
};

struct DockRegion {
    std::string id;
    sf::FloatRect rect;
    std::vector<std::string> windowIds;
    int splitDirection;
    float splitRatio;
    std::string parentRegion;
};

struct DragDropPayload {
    std::string type;
    std::string data;
    void* userData;
    bool active;
    sf::Vector2f startPos;
};

struct GUISeparator {
    bool vertical;
    float thickness;
    sf::Color color;
};

struct GUIGroup {
    std::string id;
    sf::FloatRect rect;
    bool collapsible;
    bool collapsed;
    std::vector<std::string> childWidgets;
};

struct GUIState {
    std::string focusedWidget;
    std::string hoveredWidget;
    std::string activeWidget;
    std::string lastWidget;
    int frameCount;
    bool mousePressed;
    bool mouseDown;
    bool mouseReleased;
    sf::Vector2f mousePos;
};

class IMGUI {
public:
    IMGUI();
    ~IMGUI();

    void init(sf::RenderWindow& window);
    void shutdown();
    void newFrame();
    void render(sf::RenderWindow& window);

    bool beginWindow(const std::string& name, sf::FloatRect rect, bool* open = nullptr);
    void endWindow();

    bool button(const std::string& label);
    void label(const std::string& text);
    bool sliderInt(const std::string& label, int* value, int minVal, int maxVal);
    bool sliderFloat(const std::string& label, float* value, float minVal, float maxVal);
    bool checkbox(const std::string& label, bool* value);
    std::string textBox(const std::string& label, const std::string& text);
    int comboBox(const std::string& label, const std::vector<std::string>& items, int selected);
    bool colorEdit(const std::string& label, sf::Color* color);
    void separator();
    void separatorEx(const sf::Color& color, float thickness);
    void sameLine();
    bool progressBar(const std::string& label, float value, const sf::Vector2f& size = sf::Vector2f(100, 20));
    bool treeNode(const std::string& label);
    void treePop();
    bool collapsingHeader(const std::string& label);

    void beginGroup(const std::string& label, bool collapsible = false);
    void endGroup();

    void setFont(const sf::Font& font);
    void setFontSize(unsigned int size);
    void setStyleColor(const sf::Color& color);

    void dockWindow(const std::string& windowName, int side);
    void undockWindow(const std::string& windowName);
    void setDockRegion(const std::string& region, const sf::FloatRect& rect);

    void minimizeWindow(const std::string& windowName);
    void expandWindow(const std::string& windowName);
    bool isWindowMinimized(const std::string& windowName) const;

    void snapWindowToEdge(const std::string& windowName, int edge);
    void releaseSnap(const std::string& windowName);

    bool beginDragDrop(const std::string& type, const std::string& data, void* userData = nullptr);
    void endDragDrop();
    bool acceptDragDrop(const std::string& type, std::string& outData);
    DragDropPayload* getCurrentDragDrop();

    void saveTreeState(const std::string& filename);
    void loadTreeState(const std::string& filename);

    sf::Vector2f getContentRegionAvailable() const;

    static IMGUI& getInstance();

private:
    std::string getWidgetID(const std::string& base);
    bool isMouseOver(const sf::FloatRect& rect) const;
    void handleWindowInteraction(const std::string& name);
    void handleDocking(const std::string& name);
    void renderDockRegions(sf::RenderWindow& window);

    sf::RenderWindow* m_window;
    sf::Font m_font;
    unsigned int m_fontSize;
    sf::Color m_styleColor;

    GUIState m_state;
    std::unordered_map<std::string, GUIWindow> m_windows;
    std::vector<std::string> m_windowStack;

    struct WidgetState {
        bool active;
        bool hovered;
        bool clicked;
        std::string text;
        bool toggle;
    };
    std::unordered_map<std::string, WidgetState> m_widgetStates;

    sf::Vector2f m_cursorPos;
    sf::Vector2f m_windowRect;
    bool m_sameLine;

    int m_treeDepth;
    std::vector<bool> m_treeOpen;
    std::unordered_map<std::string, bool> m_treeNodeStates;

    sf::RectangleShape m_rectShape;

    std::unordered_map<std::string, DockRegion> m_dockRegions;
    std::vector<std::string> m_dockOrder;

    std::stack<GUIGroup> m_groupStack;

    DragDropPayload m_dragDrop;
    bool m_dragDropActive;
    std::string m_dragDropAcceptType;

    std::string m_treeStateFilePath;
};

} // namespace debug
} // namespace tools
} // namespace nebula
