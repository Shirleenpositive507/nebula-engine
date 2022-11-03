#include "IMGUI.h"
#include <sstream>
#include <algorithm>

namespace nebula {
namespace tools {
namespace debug {

IMGUI::IMGUI()
    : m_window(nullptr)
    , m_fontSize(14)
    , m_styleColor(60, 60, 60)
    , m_sameLine(false)
    , m_treeDepth(0)
{
    m_state.frameCount = 0;
}

IMGUI::~IMGUI() {
    shutdown();
}

IMGUI& IMGUI::getInstance() {
    static IMGUI instance;
    return instance;
}

void IMGUI::init(sf::RenderWindow& window) {
    m_window = &window;
    if (!m_font.loadFromFile("resources/fonts/consolas.ttf")) {
        if (!m_font.loadFromFile("resources/fonts/DejaVuSansMono.ttf")) {
            m_font.loadFromFile("C:/Windows/Fonts/consola.ttf");
        }
    }
    m_rectShape.setOutlineThickness(1.0f);
    m_rectShape.setOutlineColor(sf::Color(100, 100, 100));
}

void IMGUI::shutdown() {
    m_windows.clear();
    m_widgetStates.clear();
}

void IMGUI::newFrame() {
    m_state.frameCount++;
    m_state.mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);
    m_state.mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(*m_window));
    m_cursorPos = sf::Vector2f(0, 0);
    m_sameLine = false;
}

void IMGUI::render(sf::RenderWindow& window) {
    for (auto& [name, win] : m_windows) {
        if (!win.open) continue;

        m_rectShape.setPosition(win.rect.left, win.rect.top);
        m_rectShape.setSize(sf::Vector2f(win.rect.width, win.rect.height));
        m_rectShape.setFillColor(sf::Color(40, 40, 40, 220));
        window.draw(m_rectShape);

        sf::Text titleText;
        titleText.setFont(m_font);
        titleText.setString(win.id);
        titleText.setCharacterSize(m_fontSize);
        titleText.setFillColor(sf::Color::White);
        titleText.setPosition(win.rect.left + 5, win.rect.top + 3);
        window.draw(titleText);
    }
}

bool IMGUI::beginWindow(const std::string& name, sf::FloatRect rect, bool* open) {
    auto it = m_windows.find(name);
    if (it == m_windows.end()) {
        GUIWindow win;
        win.id = name;
        win.rect = rect;
        win.open = true;
        win.collapsed = false;
        win.docked = false;
        win.dockSide = -1;
        win.dragging = false;
        win.resizing = false;
        win.resizeEdge = -1;
        m_windows[name] = win;
        it = m_windows.find(name);
    }

    GUIWindow& win = it->second;
    if (open) win.open = *open;
    if (!win.open) return false;

    m_windowStack.push_back(name);
    m_windowRect = sf::Vector2f(win.rect.width, win.rect.height);
    m_cursorPos = sf::Vector2f(win.rect.left + 5, win.rect.top + 25);

    sf::Text titleText;
    titleText.setFont(m_font);
    titleText.setString(win.id);
    titleText.setCharacterSize(m_fontSize);

    sf::FloatRect titleRect(win.rect.left, win.rect.top, win.rect.width, 22.0f);
    handleWindowInteraction(name);

    return true;
}

void IMGUI::endWindow() {
    if (!m_windowStack.empty()) {
        m_windowStack.pop_back();
    }
}

void IMGUI::handleWindowInteraction(const std::string& name) {
    auto it = m_windows.find(name);
    if (it == m_windows.end()) return;

    GUIWindow& win = it->second;
    sf::FloatRect titleBar(win.rect.left, win.rect.top, win.rect.width, 22.0f);

    if (m_state.mousePressed && isMouseOver(titleBar)) {
        win.dragging = true;
        win.dragOffset = m_state.mousePos - sf::Vector2f(win.rect.left, win.rect.top);
    }

    if (win.dragging) {
        if (!sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            win.dragging = false;
        } else {
            win.rect.left = m_state.mousePos.x - win.dragOffset.x;
            win.rect.top = m_state.mousePos.y - win.dragOffset.y;
        }
    }
}

bool IMGUI::button(const std::string& label) {
    std::string wid = getWidgetID(label);
    sf::Vector2f btnSize(100, 25);
    sf::FloatRect btnRect(m_cursorPos.x, m_cursorPos.y, btnSize.x, btnSize.y);

    bool clicked = false;
    if (m_state.mousePressed && isMouseOver(btnRect)) {
        clicked = true;
    }

    m_rectShape.setPosition(btnRect.left, btnRect.top);
    m_rectShape.setSize(btnSize);
    m_rectShape.setFillColor(isMouseOver(btnRect) ? sf::Color(80, 80, 80) : sf::Color(60, 60, 60));
    m_window->draw(m_rectShape);

    sf::Text btnText;
    btnText.setFont(m_font);
    btnText.setString(label);
    btnText.setCharacterSize(m_fontSize);
    btnText.setFillColor(sf::Color::White);
    btnText.setPosition(btnRect.left + 5, btnRect.top + 3);
    m_window->draw(btnText);

    m_cursorPos.y += btnSize.y + 4;
    return clicked;
}

void IMGUI::label(const std::string& text) {
    sf::Text labelText;
    labelText.setFont(m_font);
    labelText.setString(text);
    labelText.setCharacterSize(m_fontSize);
    labelText.setFillColor(sf::Color(200, 200, 200));
    labelText.setPosition(m_cursorPos.x, m_cursorPos.y);
    m_window->draw(labelText);
    m_cursorPos.y += m_fontSize + 4;
}

bool IMGUI::sliderInt(const std::string& label, int* value, int minVal, int maxVal) {
    std::string wid = getWidgetID(label);
    sf::Vector2f sliderSize(150, 20);
    sf::FloatRect sliderRect(m_cursorPos.x, m_cursorPos.y, sliderSize.x, sliderSize.y);

    m_rectShape.setPosition(sliderRect.left, sliderRect.top);
    m_rectShape.setSize(sliderSize);
    m_rectShape.setFillColor(sf::Color(50, 50, 50));
    m_window->draw(m_rectShape);

    float t = static_cast<float>(*value - minVal) / static_cast<float>(maxVal - minVal);
    t = std::clamp(t, 0.0f, 1.0f);

    sf::RectangleShape fill(sf::Vector2f(t * sliderSize.x, sliderSize.y));
    fill.setPosition(sliderRect.left, sliderRect.top);
    fill.setFillColor(sf::Color(100, 150, 200));
    m_window->draw(fill);

    std::string display = label + ": " + std::to_string(*value);
    sf::Text sliderText;
    sliderText.setFont(m_font);
    sliderText.setString(display);
    sliderText.setCharacterSize(m_fontSize);
    sliderText.setFillColor(sf::Color::White);
    sliderText.setPosition(sliderRect.left + 5, sliderRect.top + 2);
    m_window->draw(sliderText);

    bool changed = false;
    if (m_state.mousePressed && isMouseOver(sliderRect)) {
        float nt = (m_state.mousePos.x - sliderRect.left) / sliderSize.x;
        *value = static_cast<int>(minVal + nt * (maxVal - minVal));
        *value = std::clamp(*value, minVal, maxVal);
        changed = true;
    }

    m_cursorPos.y += sliderSize.y + 4;
    return changed;
}

bool IMGUI::sliderFloat(const std::string& label, float* value, float minVal, float maxVal) {
    std::string wid = getWidgetID(label);
    sf::Vector2f sliderSize(150, 20);
    sf::FloatRect sliderRect(m_cursorPos.x, m_cursorPos.y, sliderSize.x, sliderSize.y);

    m_rectShape.setPosition(sliderRect.left, sliderRect.top);
    m_rectShape.setSize(sliderSize);
    m_rectShape.setFillColor(sf::Color(50, 50, 50));
    m_window->draw(m_rectShape);

    float t = (*value - minVal) / (maxVal - minVal);
    t = std::clamp(t, 0.0f, 1.0f);

    sf::RectangleShape fill(sf::Vector2f(t * sliderSize.x, sliderSize.y));
    fill.setPosition(sliderRect.left, sliderRect.top);
    fill.setFillColor(sf::Color(100, 150, 200));
    m_window->draw(fill);

    std::ostringstream oss;
    oss << label << ": " << std::fixed << std::setprecision(2) << *value;
    sf::Text sliderText;
    sliderText.setFont(m_font);
    sliderText.setString(oss.str());
    sliderText.setCharacterSize(m_fontSize);
    sliderText.setFillColor(sf::Color::White);
    sliderText.setPosition(sliderRect.left + 5, sliderRect.top + 2);
    m_window->draw(sliderText);

    bool changed = false;
    if (m_state.mousePressed && isMouseOver(sliderRect)) {
        float nt = (m_state.mousePos.x - sliderRect.left) / sliderSize.x;
        *value = minVal + nt * (maxVal - minVal);
        *value = std::clamp(*value, minVal, maxVal);
        changed = true;
    }

    m_cursorPos.y += sliderSize.y + 4;
    return changed;
}

bool IMGUI::checkbox(const std::string& label, bool* value) {
    std::string wid = getWidgetID(label);
    sf::Vector2f boxSize(16, 16);
    sf::FloatRect boxRect(m_cursorPos.x, m_cursorPos.y, boxSize.x, boxSize.y);

    m_rectShape.setPosition(boxRect.left, boxRect.top);
    m_rectShape.setSize(boxSize);
    m_rectShape.setFillColor(*value ? sf::Color(100, 200, 100) : sf::Color(50, 50, 50));
    m_window->draw(m_rectShape);

    if (*value) {
        sf::Text check;
        check.setFont(m_font);
        check.setString("X");
        check.setCharacterSize(m_fontSize);
        check.setFillColor(sf::Color::White);
        check.setPosition(boxRect.left + 2, boxRect.top - 1);
        m_window->draw(check);
    }

    sf::Text cbText;
    cbText.setFont(m_font);
    cbText.setString(label);
    cbText.setCharacterSize(m_fontSize);
    cbText.setFillColor(sf::Color::White);
    cbText.setPosition(boxRect.left + boxSize.x + 6, boxRect.top);
    m_window->draw(cbText);

    bool changed = false;
    if (m_state.mousePressed && isMouseOver(boxRect)) {
        *value = !*value;
        changed = true;
    }

    m_cursorPos.y += boxSize.y + 4;
    return changed;
}

std::string IMGUI::textBox(const std::string& label, const std::string& text) {
    std::string wid = getWidgetID(label);
    sf::Vector2f tbSize(200, 22);
    sf::FloatRect tbRect(m_cursorPos.x, m_cursorPos.y, tbSize.x, tbSize.y);

    m_rectShape.setPosition(tbRect.left, tbRect.top);
    m_rectShape.setSize(tbSize);
    m_rectShape.setFillColor(sf::Color(30, 30, 30));
    m_window->draw(m_rectShape);

    sf::Text tbText;
    tbText.setFont(m_font);
    tbText.setString(text + (m_state.focusedWidget == wid ? "_" : ""));
    tbText.setCharacterSize(m_fontSize);
    tbText.setFillColor(sf::Color::White);
    tbText.setPosition(tbRect.left + 3, tbRect.top + 2);
    m_window->draw(tbText);

    m_cursorPos.y += tbSize.y + 4;
    return text;
}

int IMGUI::comboBox(const std::string& label, const std::vector<std::string>& items, int selected) {
    std::string wid = getWidgetID(label);
    sf::Vector2f cbSize(150, 22);
    sf::FloatRect cbRect(m_cursorPos.x, m_cursorPos.y, cbSize.x, cbSize.y);

    m_rectShape.setPosition(cbRect.left, cbRect.top);
    m_rectShape.setSize(cbSize);
    m_rectShape.setFillColor(sf::Color(50, 50, 50));
    m_window->draw(m_rectShape);

    std::string display = label + ": " + (selected >= 0 && selected < static_cast<int>(items.size()) ? items[selected] : "");
    sf::Text cbText;
    cbText.setFont(m_font);
    cbText.setString(display);
    cbText.setCharacterSize(m_fontSize);
    cbText.setFillColor(sf::Color::White);
    cbText.setPosition(cbRect.left + 3, cbRect.top + 2);
    m_window->draw(cbText);

    m_cursorPos.y += cbSize.y + 4;
    return selected;
}

bool IMGUI::colorEdit(const std::string& label, sf::Color* color) {
    std::string wid = getWidgetID(label);
    sf::Vector2f colorSize(30, 20);
    sf::FloatRect colorRect(m_cursorPos.x, m_cursorPos.y, colorSize.x, colorSize.y);

    m_rectShape.setPosition(colorRect.left, colorRect.top);
    m_rectShape.setSize(colorSize);
    m_rectShape.setFillColor(*color);
    m_window->draw(m_rectShape);

    m_rectShape.setOutlineColor(sf::Color::White);
    m_rectShape.setOutlineThickness(1);
    m_window->draw(m_rectShape);
    m_rectShape.setOutlineThickness(0);

    sf::Text colorLabel;
    colorLabel.setFont(m_font);
    colorLabel.setString(label);
    colorLabel.setCharacterSize(m_fontSize);
    colorLabel.setFillColor(sf::Color::White);
    colorLabel.setPosition(colorRect.left + colorSize.x + 5, colorRect.top);
    m_window->draw(colorLabel);

    m_cursorPos.y += colorSize.y + 4;
    return false;
}

void IMGUI::separator() {
    sf::Vertex line[] = {
        sf::Vertex(sf::Vector2f(m_cursorPos.x, m_cursorPos.y + m_fontSize / 2), sf::Color(80, 80, 80)),
        sf::Vertex(sf::Vector2f(m_cursorPos.x + 250, m_cursorPos.y + m_fontSize / 2), sf::Color(80, 80, 80))
    };
    m_window->draw(line, 2, sf::Lines);
    m_cursorPos.y += m_fontSize;
}

void IMGUI::sameLine() {
    m_cursorPos.x += 120.0f;
}

bool IMGUI::progressBar(const std::string& label, float value, const sf::Vector2f& size) {
    sf::FloatRect pbRect(m_cursorPos.x, m_cursorPos.y, size.x, size.y);

    m_rectShape.setPosition(pbRect.left, pbRect.top);
    m_rectShape.setSize(size);
    m_rectShape.setFillColor(sf::Color(40, 40, 40));
    m_window->draw(m_rectShape);

    float fillWidth = std::clamp(value, 0.0f, 1.0f) * size.x;
    sf::RectangleShape fill(sf::Vector2f(fillWidth, size.y));
    fill.setPosition(pbRect.left, pbRect.top);
    fill.setFillColor(sf::Color(60, 180, 75));
    m_window->draw(fill);

    std::ostringstream oss;
    oss << label << ": " << static_cast<int>(value * 100.0f) << "%";
    sf::Text pbText;
    pbText.setFont(m_font);
    pbText.setString(oss.str());
    pbText.setCharacterSize(m_fontSize);
    pbText.setFillColor(sf::Color::White);
    pbText.setPosition(pbRect.left + 3, pbRect.top + 1);
    m_window->draw(pbText);

    m_cursorPos.y += size.y + 4;
    return false;
}

bool IMGUI::treeNode(const std::string& label) {
    bool& open = m_widgetStates[getWidgetID("tree_" + label)].toggle;
    std::string display = (open ? "v " : "> ") + label;

    sf::Text treeText;
    treeText.setFont(m_font);
    treeText.setString(display);
    treeText.setCharacterSize(m_fontSize);
    treeText.setFillColor(sf::Color(255, 200, 100));

    sf::FloatRect treeRect(m_cursorPos.x, m_cursorPos.y, 200, m_fontSize + 4);
    treeText.setPosition(m_cursorPos.x + m_treeDepth * 10, m_cursorPos.y);
    m_window->draw(treeText);

    if (m_state.mousePressed && isMouseOver(treeRect)) {
        open = !open;
    }

    m_cursorPos.y += m_fontSize + 4;
    if (open) m_treeDepth++;
    return open;
}

void IMGUI::treePop() {
    if (m_treeDepth > 0) m_treeDepth--;
}

bool IMGUI::collapsingHeader(const std::string& label) {
    return treeNode(label);
}

void IMGUI::setFont(const sf::Font& font) {
    m_font = font;
}

void IMGUI::setFontSize(unsigned int size) {
    m_fontSize = size;
}

void IMGUI::setStyleColor(const sf::Color& color) {
    m_styleColor = color;
}

void IMGUI::dockWindow(const std::string& windowName, int side) {
    auto it = m_windows.find(windowName);
    if (it != m_windows.end()) {
        it->second.docked = true;
        it->second.dockSide = side;
    }
}

void IMGUI::undockWindow(const std::string& windowName) {
    auto it = m_windows.find(windowName);
    if (it != m_windows.end()) {
        it->second.docked = false;
        it->second.dockSide = -1;
    }
}

sf::Vector2f IMGUI::getContentRegionAvailable() const {
    return sf::Vector2f(m_windowRect.x - 10, m_windowRect.y - 30);
}

std::string IMGUI::getWidgetID(const std::string& base) {
    std::string fullID;
    for (const auto& win : m_windowStack) {
        fullID += win + "::";
    }
    fullID += base;
    return fullID;
}

bool IMGUI::isMouseOver(const sf::FloatRect& rect) const {
    return rect.contains(m_state.mousePos);
}

} // namespace debug
} // namespace tools
} // namespace nebula
