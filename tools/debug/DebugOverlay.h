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

struct GraphData {
    std::deque<float> history;
    sf::Color color;
    std::string label;
    float minValue;
    float maxValue;
    bool autoScale;
    size_t maxPoints;
};

enum class GraphType {
    CPU,
    GPU,
    Memory,
    EntityCount,
    DrawCall
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

    void setCPUUsage(float usage);
    void setGPUUsage(float usage);
    void setMemoryUsageMB(float mb);
    void setEntityCountGraph(int count);
    void setDrawCallGraph(int calls);

    void setGraphColor(GraphType type, const sf::Color& color);
    void setGraphAutoScale(GraphType type, bool autoScale);
    void setGraphMaxPoints(GraphType type, size_t maxPoints);
    void clearGraph(GraphType type);
    void clearAllGraphs();

private:
    void updateDefaultStats();
    void renderFPSGraph(sf::RenderWindow& window);
    void renderStatsPanel(sf::RenderWindow& window);
    void renderMousePosition(sf::RenderWindow& window);
    void renderPerformanceMetrics(sf::RenderWindow& window);
    void renderCPUUsageGraph(sf::RenderWindow& window);
    void renderGPUUsageGraph(sf::RenderWindow& window);
    void renderMemoryUsageGraph(sf::RenderWindow& window);
    void renderEntityCountGraph(sf::RenderWindow& window);
    void renderDrawCallGraph(sf::RenderWindow& window);
    void renderLineGraph(sf::RenderWindow& window, const GraphData& data,
                         const sf::FloatRect& bounds, const std::string& title);

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

    std::unordered_map<GraphType, GraphData> m_graphs;
    static constexpr size_t DEFAULT_GRAPH_POINTS = 200;
};

} // namespace debug
} // namespace tools
} // namespace nebula
