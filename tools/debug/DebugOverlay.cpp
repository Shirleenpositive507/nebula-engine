#include "DebugOverlay.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

namespace nebula {
namespace tools {
namespace debug {

DebugOverlay::DebugOverlay()
    : m_visible(false)
    , m_toggleKey(sf::Keyboard::F3)
    , m_fontSize(14)
    , m_position(10.0f, 10.0f)
    , m_backgroundAlpha(0.7f)
    , m_currentFPS(0.0f)
    , m_frameTime(0.0f)
    , m_drawCalls(0)
    , m_entityCount(0)
    , m_physicsBodyCount(0)
    , m_memoryUsage(0)
    , m_showMousePosition(true)
    , m_showCollisionWireframe(false)
{
}

DebugOverlay::~DebugOverlay() {
    shutdown();
}

void DebugOverlay::init() {
    if (!m_font.loadFromFile("resources/fonts/consolas.ttf")) {
        if (!m_font.loadFromFile("resources/fonts/DejaVuSansMono.ttf")) {
            m_font.loadFromFile("C:/Windows/Fonts/consola.ttf");
        }
    }

    m_fpsHistory.resize(MAX_FPS_HISTORY, FPSPoint{0.0f, sf::Color::Green});

    addStat("FPS", "0");
    addStat("Frame Time", "0.0 ms");
    addStat("Draw Calls", "0");
    addStat("Entities", "0");
    addStat("Physics Bodies", "0");
    addStat("Memory", "0 MB");
}

void DebugOverlay::update(float dt) {
    if (!m_visible) return;

    if (dt > 0.0f) {
        m_currentFPS = 1.0f / dt;
        m_frameTime = dt * 1000.0f;
    }

    m_fpsHistory.push_back({m_currentFPS, sf::Color::Green});
    if (m_fpsHistory.size() > MAX_FPS_HISTORY) {
        m_fpsHistory.pop_front();
    }

    m_background.setSize(sf::Vector2f(280.0f, 40.0f + m_stats.size() * (m_fontSize + 4)));
    m_background.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(m_backgroundAlpha * 255)));

    updateDefaultStats();
}

void DebugOverlay::updateDefaultStats() {
    std::ostringstream fpsStream;
    fpsStream << std::fixed << std::setprecision(1) << m_currentFPS;
    for (auto& stat : m_stats) {
        if (stat.name == "FPS") stat.value = fpsStream.str();
        else if (stat.name == "Frame Time") {
            std::ostringstream ft;
            ft << std::fixed << std::setprecision(2) << m_frameTime << " ms";
            stat.value = ft.str();
        }
        else if (stat.name == "Draw Calls") stat.value = std::to_string(m_drawCalls);
        else if (stat.name == "Entities") stat.value = std::to_string(m_entityCount);
        else if (stat.name == "Physics Bodies") stat.value = std::to_string(m_physicsBodyCount);
        else if (stat.name == "Memory") {
            std::ostringstream mem;
            mem << std::fixed << std::setprecision(1) << (m_memoryUsage / (1024.0 * 1024.0)) << " MB";
            stat.value = mem.str();
        }
    }
}

void DebugOverlay::render(sf::RenderWindow& window) {
    if (!m_visible) return;

    renderStatsPanel(window);
    renderFPSGraph(window);
    renderPerformanceMetrics(window);

    if (m_showMousePosition) {
        renderMousePosition(window);
    }
}

void DebugOverlay::renderStatsPanel(sf::RenderWindow& window) {
    sf::Vector2f panelPos = m_position;

    m_background.setPosition(panelPos);
    window.draw(m_background);

    sf::Text titleText;
    titleText.setFont(m_font);
    titleText.setString("Debug Overlay");
    titleText.setCharacterSize(m_fontSize + 2);
    titleText.setFillColor(sf::Color::Yellow);
    titleText.setPosition(panelPos.x + 5, panelPos.y + 2);
    window.draw(titleText);

    float yOffset = panelPos.y + m_fontSize + 8;
    for (const auto& stat : m_stats) {
        sf::Text statText;
        statText.setFont(m_font);
        statText.setCharacterSize(m_fontSize);

        std::string display = stat.name + ": " + stat.value;
        statText.setString(display);

        if (stat.name == "FPS") {
            float fps = std::stof(stat.value);
            if (fps >= 60.0f) statText.setFillColor(sf::Color::Green);
            else if (fps >= 30.0f) statText.setFillColor(sf::Color::Yellow);
            else statText.setFillColor(sf::Color::Red);
        } else {
            statText.setFillColor(sf::Color::White);
        }

        statText.setPosition(m_position.x + 5, yOffset);
        window.draw(statText);
        yOffset += m_fontSize + 4;
    }
}

void DebugOverlay::renderFPSGraph(sf::RenderWindow& window) {
    if (m_fpsHistory.empty()) return;

    sf::Vector2f graphPos(m_position.x, m_position.y + 60.0f + m_stats.size() * (m_fontSize + 4));
    float graphWidth = 260.0f;
    float graphHeight = 60.0f;
    float padding = 5.0f;

    sf::RectangleShape graphBg(sf::Vector2f(graphWidth + padding * 2, graphHeight + padding * 2));
    graphBg.setPosition(graphPos);
    graphBg.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(m_backgroundAlpha * 255)));
    window.draw(graphBg);

    sf::Text graphLabel;
    graphLabel.setFont(m_font);
    graphLabel.setString("FPS Graph");
    graphLabel.setCharacterSize(m_fontSize - 2);
    graphLabel.setFillColor(sf::Color::Cyan);
    graphLabel.setPosition(graphPos.x + padding, graphPos.y + 2);
    window.draw(graphLabel);

    float graphTop = graphPos.y + m_fontSize + 4;
    float graphBottom = graphPos.y + graphHeight + padding - 2;
    float graphLeft = graphPos.x + padding;
    float graphRight = graphPos.x + graphWidth + padding;

    float maxFPS = 120.0f;
    size_t pointCount = m_fpsHistory.size();
    float stepX = graphWidth / static_cast<float>(pointCount - 1);

    std::vector<sf::Vertex> lines;
    for (size_t i = 0; i < pointCount; ++i) {
        float normalized = std::min(m_fpsHistory[i].fps / maxFPS, 1.0f);
        float x = graphLeft + i * stepX;
        float y = graphBottom - normalized * (graphBottom - graphTop);
        sf::Color color;
        float f = m_fpsHistory[i].fps;
        if (f >= 60.0f) color = sf::Color::Green;
        else if (f >= 30.0f) color = sf::Color::Yellow;
        else color = sf::Color::Red;
        lines.push_back(sf::Vertex(sf::Vector2f(x, y), color));
    }

    if (lines.size() >= 2) {
        window.draw(lines.data(), lines.size(), sf::LinesStrip);
    }
}

void DebugOverlay::renderPerformanceMetrics(sf::RenderWindow& window) {
    sf::Vector2f metricsPos(
        m_position.x,
        m_position.y + 130.0f + m_stats.size() * (m_fontSize + 4) + 10.0f
    );

    sf::Text metricsText;
    metricsText.setFont(m_font);
    metricsText.setCharacterSize(m_fontSize);
    metricsText.setFillColor(sf::Color(200, 200, 200));

    std::ostringstream oss;
    oss << "Performance Metrics:\n"
        << "  FPS: " << std::fixed << std::setprecision(1) << m_currentFPS << "\n"
        << "  Frame: " << std::fixed << std::setprecision(2) << m_frameTime << " ms\n"
        << "  Draw Calls: " << m_drawCalls;
    metricsText.setString(oss.str());
    metricsText.setPosition(metricsPos.x + 5, metricsPos.y);
    window.draw(metricsText);
}

void DebugOverlay::renderMousePosition(sf::RenderWindow& window) {
    sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
    sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos);

    sf::Text mouseText;
    mouseText.setFont(m_font);
    mouseText.setCharacterSize(m_fontSize);
    mouseText.setFillColor(sf::Color::White);

    std::ostringstream oss;
    oss << "Mouse: (" << pixelPos.x << ", " << pixelPos.y << ") "
        << "World: (" << std::fixed << std::setprecision(1)
        << worldPos.x << ", " << worldPos.y << ")";
    mouseText.setString(oss.str());
    mouseText.setPosition(10.0f, static_cast<float>(window.getSize().y) - 30.0f);
    window.draw(mouseText);
}

void DebugOverlay::shutdown() {
    m_stats.clear();
    m_fpsHistory.clear();
}

void DebugOverlay::addStat(const std::string& name, const std::string& value) {
    for (auto& stat : m_stats) {
        if (stat.name == name) {
            stat.value = value;
            return;
        }
    }
    m_stats.push_back({name, value});
}

void DebugOverlay::removeStat(const std::string& name) {
    m_stats.erase(
        std::remove_if(m_stats.begin(), m_stats.end(),
            [&name](const StatEntry& s) { return s.name == name; }),
        m_stats.end()
    );
}

void DebugOverlay::clear() {
    m_stats.clear();
}

void DebugOverlay::setVisible(bool visible) {
    m_visible = visible;
}

void DebugOverlay::toggle() {
    m_visible = !m_visible;
}

bool DebugOverlay::isVisible() const {
    return m_visible;
}

void DebugOverlay::setPosition(const sf::Vector2f& pos) {
    m_position = pos;
}

void DebugOverlay::setFontSize(unsigned int size) {
    m_fontSize = size;
}

void DebugOverlay::setBackgroundAlpha(float alpha) {
    m_backgroundAlpha = std::clamp(alpha, 0.0f, 1.0f);
}

void DebugOverlay::setKeyToggle(sf::Keyboard::Key key) {
    m_toggleKey = key;
}

void DebugOverlay::showMousePosition(bool show) {
    m_showMousePosition = show;
}

bool DebugOverlay::isShowingMousePosition() const {
    return m_showMousePosition;
}

void DebugOverlay::showCollisionWireframe(bool show) {
    m_showCollisionWireframe = show;
}

bool DebugOverlay::isShowingCollisionWireframe() const {
    return m_showCollisionWireframe;
}

} // namespace debug
} // namespace tools
} // namespace nebula
