#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <stack>
#include <functional>
#include <unordered_map>

namespace nebula {
namespace tools {
namespace debug {

struct GUIWindow {
    std::string id;
    sf::FloatRect rect;
    bool open;
    bool collapsed;
    bool docked;
    std::string dockTarget;
    int dockSide;
    bool dragging;
    sf::Vector2f dragOffset;
    bool resizing;
    int resizeEdge;
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
    void sameLine();
    bool progressBar(const std::string& label, float value, const sf::Vector2f& size = sf::Vector2f(100, 20));
    bool treeNode(const std::string& label);
    void treePop();
    bool collapsingHeader(const std::string& label);

    void setFont(const sf::Font& font);
    void setFontSize(unsigned int size);
    void setStyleColor(const sf::Color& color);

    void dockWindow(const std::string& windowName, int side);
    void undockWindow(const std::string& windowName);

    sf::Vector2f getContentRegionAvailable() const;

    static IMGUI& getInstance();

private:
    std::string getWidgetID(const std::string& base);
    bool isMouseOver(const sf::FloatRect& rect) const;
    void handleWindowInteraction(const std::string& name);

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

    sf::RectangleShape m_rectShape;
};

} // namespace debug
} // namespace tools
} // namespace nebula
