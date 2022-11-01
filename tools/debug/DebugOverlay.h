#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include <deque>
#include <functional>

namespace nebula {
namespace tools {
namespace debug {

struct StatEntry {
    std::string name;
    std::string value;
};

struct FPSPoint {
    float fps;
    sf::Color color;
};

class DebugOverlay {
public:
    DebugOverlay();
    ~DebugOverlay();

    void init();
    void update(float dt);
    void render(sf::RenderWindow& window);
    void shutdown();

    void addStat(const std::string& name, const std::string& value);
    void removeStat(const std::string& name);
    void clear();

    void setVisible(bool visible);
    void toggle();
    bool isVisible() const;

    void setPosition(const sf::Vector2f& pos);
    void setFontSize(unsigned int size);
    void setBackgroundAlpha(float alpha);

    void setKeyToggle(sf::Keyboard::Key key);

    void showMousePosition(bool show);
    bool isShowingMousePosition() const;

    void showCollisionWireframe(bool show);
    bool isShowingCollisionWireframe() const;

private:
    void updateDefaultStats();
    void renderFPSGraph(sf::RenderWindow& window);
    void renderStatsPanel(sf::RenderWindow& window);
    void renderMousePosition(sf::RenderWindow& window);
    void renderPerformanceMetrics(sf::RenderWindow& window);

    bool m_visible;
    sf::Keyboard::Key m_toggleKey;

    sf::Font m_font;
    unsigned int m_fontSize;
    sf::Vector2f m_position;
    float m_backgroundAlpha;

    std::vector<StatEntry> m_stats;
    std::deque<FPSPoint> m_fpsHistory;
    static constexpr size_t MAX_FPS_HISTORY = 120;

    float m_currentFPS;
    float m_frameTime;
    int m_drawCalls;
    int m_entityCount;
    int m_physicsBodyCount;
    size_t m_memoryUsage;

    bool m_showMousePosition;
    bool m_showCollisionWireframe;

    sf::RectangleShape m_background;
};

} // namespace debug
} // namespace tools
} // namespace nebula
