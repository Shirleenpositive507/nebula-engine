#include "EditorGUI.h"
#include <sstream>
#include <algorithm>
#include <cstring>
#include <cmath>

namespace nebula {
namespace tools {
namespace editor {

EditorGUI::EditorGUI()
    : m_fontSize(13)
    , m_selectedEntityId(-1)
    , m_playing(false)
    , m_paused(false)
    , m_filterActive(false)
    , m_filterCursor(0)
    , m_hierarchyWidth(250.0f)
    , m_propertiesWidth(300.0f)
    , m_assetBrowserHeight(150.0f)
    , m_toolbarHeight(40.0f)
    , m_menuBarHeight(24.0f)
    , m_colorPickerResultReady(false)
    , m_hasAssetPreview(false)
    , m_consolePanelVisible(true)
{
    std::memset(m_filterBuffer, 0, sizeof(m_filterBuffer));
    m_colorPicker.open = false;
    m_animationPreview.playing = false;
    m_animationPreview.currentTime = 0.0f;
    m_animationPreview.duration = 1.0f;
    m_animationPreview.currentFrame = 0;
    m_animationPreview.totalFrames = 1;
    m_animationPreview.playbackSpeed = 1.0f;
    m_animationPreview.looping = true;
    m_sceneStats = {};
}

EditorGUI::~EditorGUI() {
    shutdown();
}

void EditorGUI::init(sf::RenderWindow& window) {
    if (!m_font.loadFromFile("resources/fonts/consolas.ttf")) {
        m_font.loadFromFile("C:/Windows/Fonts/consola.ttf");
    }

    m_panelBg.setFillColor(sf::Color(30, 30, 30, 230));
    m_divider.setFillColor(sf::Color(60, 60, 60));
}

void EditorGUI::update(float dt) {
    if (m_animationPreview.playing) {
        m_animationPreview.currentTime += dt * m_animationPreview.playbackSpeed;
        if (m_animationPreview.currentTime >= m_animationPreview.duration) {
            if (m_animationPreview.looping) {
                m_animationPreview.currentTime = std::fmod(m_animationPreview.currentTime, m_animationPreview.duration);
            } else {
                m_animationPreview.currentTime = m_animationPreview.duration;
                m_animationPreview.playing = false;
            }
        }
        m_animationPreview.currentFrame = static_cast<int>(
            (m_animationPreview.currentTime / m_animationPreview.duration) * m_animationPreview.totalFrames
        );
    }
}

void EditorGUI::render(sf::RenderWindow& window) {
    renderMenuBar(window);
    renderToolbar(window);
    renderSceneHierarchy(window);
    renderProperties(window);
    renderAssetPreviewPanel(window);
    renderAnimationPreview(window);
    renderSceneStatsPanel(window);
    renderConsolePanel(window);
    renderAssetBrowser(window);
    if (m_colorPicker.open) {
        renderColorPicker(window);
    }
}

void EditorGUI::renderMenuBar(sf::RenderWindow& window) {
    sf::Vector2u winSize = window.getSize();
    m_panelBg.setPosition(0, 0);
    m_panelBg.setSize(sf::Vector2f(static_cast<float>(winSize.x), m_menuBarHeight));
    m_panelBg.setFillColor(sf::Color(25, 25, 25));
    window.draw(m_panelBg);

    struct MenuItem { std::string label; std::vector<std::string> items; };
    std::vector<MenuItem> menus = {
        {"File", {"New Scene", "Open Scene", "Save", "Save As...", "Exit"}},
        {"Edit", {"Undo", "Redo", "Cut", "Copy", "Paste", "Delete"}},
        {"View", {"Scene Hierarchy", "Properties", "Asset Browser", "Console", "Reset Layout"}},
        {"Tools", {"Physics Debug", "Profiler", "Memory Viewer", "Build Settings"}},
        {"Help", {"About Nebula Engine", "Documentation", "Report Issue"}},
    };

    float xPos = 5.0f;
    for (const auto& menu : menus) {
        sf::Text menuText;
        menuText.setFont(m_font);
        menuText.setString(menu.label);
        menuText.setCharacterSize(m_fontSize);
        menuText.setFillColor(sf::Color(200, 200, 200));
        menuText.setPosition(xPos + 5, 3);
        window.draw(menuText);

        sf::FloatRect textBounds = menuText.getLocalBounds();
        xPos += textBounds.width + 20.0f;
    }
}

void EditorGUI::renderToolbar(sf::RenderWindow& window) {
    sf::Vector2u winSize = window.getSize();
    float yStart = m_menuBarHeight;

    m_panelBg.setPosition(0, yStart);
    m_panelBg.setSize(sf::Vector2f(static_cast<float>(winSize.x), m_toolbarHeight));
    m_panelBg.setFillColor(sf::Color(35, 35, 35));
    window.draw(m_panelBg);

    float cx = 10.0f;
    float cy = yStart + 5.0f;

    auto drawToolButton = [&](const std::string& text, bool active, sf::Color color) {
        sf::RectangleShape btn(sf::Vector2f(55, 28));
        btn.setPosition(cx, cy);
        btn.setFillColor(active ? sf::Color(60, 60, 60) : sf::Color(40, 40, 40));
        btn.setOutlineThickness(1);
        btn.setOutlineColor(sf::Color(80, 80, 80));
        window.draw(btn);

        sf::Text btnText;
        btnText.setFont(m_font);
        btnText.setString(text);
        btnText.setCharacterSize(m_fontSize - 1);
        btnText.setFillColor(color);
        btnText.setPosition(cx + 5, cy + 5);
        window.draw(btnText);
        cx += 60.0f;
    };

    drawToolButton("Play", m_playing, sf::Color(100, 200, 100));
    drawToolButton("Pause", m_paused, sf::Color(200, 200, 100));
    drawToolButton("Stop", false, sf::Color(200, 100, 100));
    drawToolButton("Step", false, sf::Color(150, 150, 200));
}

void EditorGUI::renderSceneHierarchy(sf::RenderWindow& window) {
    sf::Vector2u winSize = window.getSize();
    float yStart = m_menuBarHeight + m_toolbarHeight;

    m_panelBg.setPosition(0, yStart);
    m_panelBg.setSize(sf::Vector2f(m_hierarchyWidth, static_cast<float>(winSize.y) - yStart - m_assetBrowserHeight));
    m_panelBg.setFillColor(sf::Color(28, 28, 28));
    window.draw(m_panelBg);

    sf::Text headerText;
    headerText.setFont(m_font);
    headerText.setString("Scene Hierarchy");
    headerText.setCharacterSize(m_fontSize + 1);
    headerText.setFillColor(sf::Color(255, 200, 100));
    headerText.setPosition(8, yStart + 5);
    window.draw(headerText);

    sf::RectangleShape searchBox(sf::Vector2f(m_hierarchyWidth - 16, 22));
    searchBox.setPosition(8, yStart + 28);
    searchBox.setFillColor(sf::Color(20, 20, 20));
    searchBox.setOutlineThickness(1);
    searchBox.setOutlineColor(sf::Color(60, 60, 60));
    window.draw(searchBox);

    sf::Text searchText;
    searchText.setFont(m_font);
    searchText.setString(std::string("Search: ") + (m_filterActive ? m_filterBuffer : ""));
    searchText.setCharacterSize(m_fontSize - 1);
    searchText.setFillColor(sf::Color(150, 150, 150));
    searchText.setPosition(12, yStart + 30);
    window.draw(searchText);

    float yPos = yStart + 56;
    for (const auto& entity : m_entities) {
        if (entity.parentId == -1) {
            drawEntityTreeNode(window, entity, yPos);
        }
    }
}

void EditorGUI::drawEntityTreeNode(sf::RenderWindow& window, const EntityNode& node, float& yPos) {
    bool isSelected = (node.id == m_selectedEntityId);

    sf::RectangleShape itemBg(sf::Vector2f(m_hierarchyWidth, 20));
    itemBg.setPosition(0, yPos);
    itemBg.setFillColor(isSelected ? sf::Color(50, 80, 120) : sf::Color::Transparent);
    window.draw(itemBg);

    sf::Text entityText;
    entityText.setFont(m_font);
    std::string prefix = node.children.empty() ? "  " : (node.expanded ? "v " : "> ");
    entityText.setString(prefix + node.name);
    entityText.setCharacterSize(m_fontSize);
    entityText.setFillColor(isSelected ? sf::Color::White : sf::Color(200, 200, 200));
    entityText.setPosition(12, yPos + 2);
    window.draw(entityText);

    yPos += 22;

    if (node.expanded) {
        for (int childId : node.children) {
            auto it = std::find_if(m_entities.begin(), m_entities.end(),
                [childId](const EntityNode& e) { return e.id == childId; });
            if (it != m_entities.end()) {
                drawEntityTreeNode(window, *it, yPos);
            }
        }
    }
}

void EditorGUI::renderProperties(sf::RenderWindow& window) {
    sf::Vector2u winSize = window.getSize();
    float xStart = static_cast<float>(winSize.x) - m_propertiesWidth;
    float yStart = m_menuBarHeight + m_toolbarHeight;

    m_panelBg.setPosition(xStart, yStart);
    m_panelBg.setSize(sf::Vector2f(m_propertiesWidth, static_cast<float>(winSize.y) - yStart - m_assetBrowserHeight));
    m_panelBg.setFillColor(sf::Color(28, 28, 28));
    window.draw(m_panelBg);

    sf::Text headerText;
    headerText.setFont(m_font);
    headerText.setString(m_selectedEntityId >= 0 ? "Properties" : "Properties (No Selection)");
    headerText.setCharacterSize(m_fontSize + 1);
    headerText.setFillColor(sf::Color(255, 200, 100));
    headerText.setPosition(xStart + 8, yStart + 5);
    window.draw(headerText);

    if (m_selectedEntityId >= 0) {
        float yPos = yStart + 30;

        sf::Text nameLabel;
        nameLabel.setFont(m_font);
        nameLabel.setString("Entity ID: " + std::to_string(m_selectedEntityId));
        nameLabel.setCharacterSize(m_fontSize);
        nameLabel.setFillColor(sf::Color(180, 180, 180));
        nameLabel.setPosition(xStart + 8, yPos);
        window.draw(nameLabel);
        yPos += 22;

        sf::Text transformHeader;
        transformHeader.setFont(m_font);
        transformHeader.setString("Transform");
        transformHeader.setCharacterSize(m_fontSize);
        transformHeader.setFillColor(sf::Color(200, 200, 100));
        transformHeader.setPosition(xStart + 8, yPos);
        window.draw(transformHeader);
        yPos += 20;

        struct TransformField { std::string label; float values[3]; };
        std::vector<TransformField> fields = {
            {"Position", {0, 0, 0}},
            {"Rotation", {0, 0, 0}},
            {"Scale", {1, 1, 1}},
        };

        for (const auto& field : fields) {
            sf::Text fieldLabel;
            fieldLabel.setFont(m_font);
            fieldLabel.setString(field.label + ":");
            fieldLabel.setCharacterSize(m_fontSize - 1);
            fieldLabel.setFillColor(sf::Color(160, 160, 160));
            fieldLabel.setPosition(xStart + 16, yPos);
            window.draw(fieldLabel);

            for (int i = 0; i < 3; ++i) {
                sf::RectangleShape fieldBox(sf::Vector2f(55, 18));
                fieldBox.setPosition(xStart + 90 + i * 62, yPos);
                fieldBox.setFillColor(sf::Color(20, 20, 20));
                fieldBox.setOutlineThickness(1);
                fieldBox.setOutlineColor(sf::Color(60, 60, 60));
                window.draw(fieldBox);

                sf::Text valText;
                valText.setFont(m_font);
                valText.setString(std::to_string(static_cast<int>(field.values[i])));
                valText.setCharacterSize(m_fontSize - 2);
                valText.setFillColor(sf::Color(180, 200, 255));
                valText.setPosition(xStart + 93 + i * 62, yPos + 1);
                window.draw(valText);
            }
            yPos += 22;
        }

        for (const auto& comp : m_components) {
            drawComponentEditor(window, comp, yPos);
        }
    }
}

void EditorGUI::drawComponentEditor(sf::RenderWindow& window, const ComponentInfo& comp, float& yPos) {
    sf::Vector2u winSize = window.getSize();
    float xStart = static_cast<float>(winSize.x) - m_propertiesWidth;

    sf::RectangleShape compBg(sf::Vector2f(m_propertiesWidth - 4, 24));
    compBg.setPosition(xStart + 2, yPos);
    compBg.setFillColor(sf::Color(35, 35, 40));
    compBg.setOutlineThickness(1);
    compBg.setOutlineColor(sf::Color(50, 50, 55));
    window.draw(compBg);

    sf::Text compText;
    compText.setFont(m_font);
    compText.setString(comp.type);
    compText.setCharacterSize(m_fontSize);
    compText.setFillColor(comp.enabled ? sf::Color(220, 220, 220) : sf::Color(100, 100, 100));
    compText.setPosition(xStart + 10, yPos + 3);
    window.draw(compText);

    sf::Text summaryText;
    summaryText.setFont(m_font);
    summaryText.setString(comp.summary);
    summaryText.setCharacterSize(m_fontSize - 2);
    summaryText.setFillColor(sf::Color(130, 130, 130));
    summaryText.setPosition(xStart + 80, yPos + 4);
    window.draw(summaryText);

    yPos += 28;
}

void EditorGUI::renderColorPicker(sf::RenderWindow& window) {
    sf::Vector2u winSize = window.getSize();
    float pickerW = 280.0f, pickerH = 320.0f;
    float px = (static_cast<float>(winSize.x) - pickerW) / 2.0f;
    float py = (static_cast<float>(winSize.y) - pickerH) / 2.0f;

    sf::RectangleShape bg(sf::Vector2f(pickerW, pickerH));
    bg.setPosition(px, py);
    bg.setFillColor(sf::Color(40, 40, 40, 240));
    bg.setOutlineThickness(2);
    bg.setOutlineColor(sf::Color(100, 100, 100));
    window.draw(bg);

    sf::Text title;
    title.setFont(m_font);
    title.setString("Color Picker");
    title.setCharacterSize(m_fontSize + 2);
    title.setFillColor(sf::Color::White);
    title.setPosition(px + 10, py + 8);
    window.draw(title);

    sf::RectangleShape previewSwatch(sf::Vector2f(60, 30));
    previewSwatch.setPosition(px + 200, py + 8);
    previewSwatch.setFillColor(m_colorPicker.previewColor);
    window.draw(previewSwatch);

    float cy = py + 40;
    auto drawSlider = [&](const std::string& label, int& val, int minV, int maxV, sf::Color color) {
        sf::Text labelText;
        labelText.setFont(m_font);
        labelText.setString(label + ": " + std::to_string(val));
        labelText.setCharacterSize(m_fontSize - 1);
        labelText.setFillColor(sf::Color(200, 200, 200));
        labelText.setPosition(px + 10, cy);
        window.draw(labelText);

        sf::RectangleShape sliderBg(sf::Vector2f(180, 12));
        sliderBg.setPosition(px + 80, cy + 2);
        sliderBg.setFillColor(sf::Color(30, 30, 30));
        window.draw(sliderBg);

        float t = static_cast<float>(val - minV) / static_cast<float>(maxV - minV);
        sf::RectangleShape sliderFill(sf::Vector2f(t * 180, 12));
        sliderFill.setPosition(px + 80, cy + 2);
        sliderFill.setFillColor(color);
        window.draw(sliderFill);

        cy += 22;
    };

    drawSlider("R", m_colorPicker.r, 0, 255, sf::Color::Red);
    drawSlider("G", m_colorPicker.g, 0, 255, sf::Color::Green);
    drawSlider("B", m_colorPicker.b, 0, 255, sf::Color::Blue);
    drawSlider("A", m_colorPicker.a, 0, 255, sf::Color(180, 180, 180));

    m_colorPicker.previewColor = sf::Color(
        static_cast<sf::Uint8>(m_colorPicker.r),
        static_cast<sf::Uint8>(m_colorPicker.g),
        static_cast<sf::Uint8>(m_colorPicker.b),
        static_cast<sf::Uint8>(m_colorPicker.a)
    );

    sf::RectangleShape applyBtn(sf::Vector2f(80, 24));
    applyBtn.setPosition(px + 50, cy + 10);
    applyBtn.setFillColor(sf::Color(60, 120, 60));
    window.draw(applyBtn);
    sf::Text applyText;
    applyText.setFont(m_font);
    applyText.setString("Apply");
    applyText.setCharacterSize(m_fontSize);
    applyText.setFillColor(sf::Color::White);
    applyText.setPosition(px + 68, cy + 12);
    window.draw(applyText);
}

void EditorGUI::renderAssetPreviewPanel(sf::RenderWindow& window) {
    if (!m_hasAssetPreview) return;

    sf::Vector2u winSize = window.getSize();
    float panelW = 200.0f;
    float panelH = 180.0f;
    float px = static_cast<float>(winSize.x) - m_propertiesWidth - panelW - 10.0f;
    float py = m_menuBarHeight + m_toolbarHeight + 10.0f;

    m_panelBg.setPosition(px, py);
    m_panelBg.setSize(sf::Vector2f(panelW, panelH));
    m_panelBg.setFillColor(sf::Color(25, 25, 30, 220));
    window.draw(m_panelBg);

    sf::Text header;
    header.setFont(m_font);
    header.setString("Preview: " + m_assetPreview.name);
    header.setCharacterSize(m_fontSize - 1);
    header.setFillColor(sf::Color(255, 200, 100));
    header.setPosition(px + 5, py + 3);
    window.draw(header);

    sf::Text info;
    info.setFont(m_font);
    std::string infoStr = m_assetPreview.type + " | " +
        std::to_string(m_assetPreview.width) + "x" + std::to_string(m_assetPreview.height);
    info.setString(infoStr);
    info.setCharacterSize(m_fontSize - 2);
    info.setFillColor(sf::Color(150, 150, 150));
    info.setPosition(px + 5, py + 22);
    window.draw(info);

    if (m_assetPreview.textureLoaded) {
        sf::Sprite previewSprite(m_assetPreview.previewTexture);
        sf::FloatRect spriteBounds = previewSprite.getLocalBounds();
        float scale = std::min((panelW - 20) / spriteBounds.width,
                               (panelH - 50) / spriteBounds.height);
        previewSprite.setScale(scale, scale);
        previewSprite.setPosition(px + 10, py + 40);
        window.draw(previewSprite);
    } else {
        sf::Text noPreview;
        noPreview.setFont(m_font);
        noPreview.setString("No preview available");
        noPreview.setCharacterSize(m_fontSize - 1);
        noPreview.setFillColor(sf::Color(100, 100, 100));
        noPreview.setPosition(px + 30, py + panelH / 2 - 10);
        window.draw(noPreview);
    }
}

void EditorGUI::renderAnimationPreview(sf::RenderWindow& window) {
    sf::Vector2u winSize = window.getSize();
    float panelW = 250.0f;
    float panelH = 120.0f;
    float px = static_cast<float>(winSize.x) - m_propertiesWidth - panelW - 10.0f;
    float py = m_menuBarHeight + m_toolbarHeight + 200.0f;

    m_panelBg.setPosition(px, py);
    m_panelBg.setSize(sf::Vector2f(panelW, panelH));
    m_panelBg.setFillColor(sf::Color(25, 25, 30, 220));
    window.draw(m_panelBg);

    sf::Text header;
    header.setFont(m_font);
    header.setString("Animation: " + m_animationPreview.animationName);
    header.setCharacterSize(m_fontSize - 1);
    header.setFillColor(sf::Color(255, 200, 100));
    header.setPosition(px + 5, py + 3);
    window.draw(header);

    sf::Text info;
    info.setFont(m_font);
    std::string infoStr = "Frame " + std::to_string(m_animationPreview.currentFrame + 1) +
        "/" + std::to_string(m_animationPreview.totalFrames) +
        " | " + std::to_string(static_cast<int>(m_animationPreview.currentTime * 100) / 100.0f) + "s";
    info.setString(infoStr);
    info.setCharacterSize(m_fontSize - 2);
    info.setFillColor(sf::Color(150, 150, 150));
    info.setPosition(px + 5, py + 22);
    window.draw(info);

    sf::RectangleShape timeline(sf::Vector2f(panelW - 20, 8));
    timeline.setPosition(px + 10, py + 50);
    timeline.setFillColor(sf::Color(40, 40, 40));
    window.draw(timeline);

    float progress = m_animationPreview.duration > 0 ? m_animationPreview.currentTime / m_animationPreview.duration : 0;
    sf::RectangleShape progressFill(sf::Vector2f((panelW - 20) * progress, 8));
    progressFill.setPosition(px + 10, py + 50);
    progressFill.setFillColor(sf::Color(100, 200, 100));
    window.draw(progressFill);

    sf::Text btnLabel;
    btnLabel.setFont(m_font);
    btnLabel.setString(m_animationPreview.playing ? "Pause" : "Play");
    btnLabel.setCharacterSize(m_fontSize);
    btnLabel.setFillColor(sf::Color::White);
    btnLabel.setPosition(px + 10, py + 70);
    window.draw(btnLabel);

    std::string loopText = m_animationPreview.looping ? "Loop: On" : "Loop: Off";
    sf::Text loopLabel;
    loopLabel.setFont(m_font);
    loopLabel.setString(loopText);
    loopLabel.setCharacterSize(m_fontSize - 1);
    loopLabel.setFillColor(sf::Color(150, 150, 200));
    loopLabel.setPosition(px + 80, py + 70);
    window.draw(loopLabel);
}

void EditorGUI::renderSceneStatsPanel(sf::RenderWindow& window) {
    sf::Vector2u winSize = window.getSize();
    float panelW = 200.0f;
    float panelH = 160.0f;
    float px = static_cast<float>(winSize.x) - m_propertiesWidth - panelW - 10.0f;
    float py = m_menuBarHeight + m_toolbarHeight + 330.0f;

    m_panelBg.setPosition(px, py);
    m_panelBg.setSize(sf::Vector2f(panelW, panelH));
    m_panelBg.setFillColor(sf::Color(25, 25, 30, 220));
    window.draw(m_panelBg);

    sf::Text header;
    header.setFont(m_font);
    header.setString("Scene Stats");
    header.setCharacterSize(m_fontSize);
    header.setFillColor(sf::Color(255, 200, 100));
    header.setPosition(px + 5, py + 3);
    window.draw(header);

    auto drawStat = [&](const std::string& label, const std::string& value, float yOff) {
        sf::Text statText;
        statText.setFont(m_font);
        statText.setString(label + ": " + value);
        statText.setCharacterSize(m_fontSize - 1);
        statText.setFillColor(sf::Color(180, 180, 180));
        statText.setPosition(px + 8, py + yOff);
        window.draw(statText);
    };

    drawStat("Entities", std::to_string(m_sceneStats.totalEntities), 25);
    drawStat("Visible", std::to_string(m_sceneStats.visibleEntities), 43);
    drawStat("Draw Calls", std::to_string(m_sceneStats.drawCalls), 61);
    drawStat("Triangles", std::to_string(m_sceneStats.triangles), 79);
    drawStat("Vertices", std::to_string(m_sceneStats.vertices), 97);
    drawStat("Lights", std::to_string(m_sceneStats.lights), 115);
    drawStat("Cameras", std::to_string(m_sceneStats.cameras), 133);
}

void EditorGUI::renderConsolePanel(sf::RenderWindow& window) {
    if (!m_consolePanelVisible) return;

    sf::Vector2u winSize = window.getSize();
    float panelH = 100.0f;
    float py = static_cast<float>(winSize.y) - m_assetBrowserHeight - panelH - 5.0f;

    m_panelBg.setPosition(0, py);
    m_panelBg.setSize(sf::Vector2f(m_hierarchyWidth, panelH));
    m_panelBg.setFillColor(sf::Color(20, 20, 22, 220));
    window.draw(m_panelBg);

    sf::Text header;
    header.setFont(m_font);
    header.setString("Console Output");
    header.setCharacterSize(m_fontSize - 1);
    header.setFillColor(sf::Color(255, 200, 100));
    header.setPosition(5, py + 3);
    window.draw(header);

    sf::Text placeholder;
    placeholder.setFont(m_font);
    placeholder.setString("Console panel (hooked to Logger)");
    placeholder.setCharacterSize(m_fontSize - 2);
    placeholder.setFillColor(sf::Color(100, 100, 100));
    placeholder.setPosition(5, py + 25);
    window.draw(placeholder);
}

void EditorGUI::renderAssetBrowser(sf::RenderWindow& window) {
    sf::Vector2u winSize = window.getSize();
    float yStart = static_cast<float>(winSize.y) - m_assetBrowserHeight;

    m_panelBg.setPosition(0, yStart);
    m_panelBg.setSize(sf::Vector2f(static_cast<float>(winSize.x), m_assetBrowserHeight));
    m_panelBg.setFillColor(sf::Color(25, 25, 28));
    window.draw(m_panelBg);

    sf::Text headerText;
    headerText.setFont(m_font);
    headerText.setString("Asset Browser");
    headerText.setCharacterSize(m_fontSize + 1);
    headerText.setFillColor(sf::Color(255, 200, 100));
    headerText.setPosition(8, yStart + 5);
    window.draw(headerText);

    float yPos = yStart + 28;
    for (const auto& child : m_assetRoot.children) {
        drawAssetTreeNode(window, child, yPos);
    }
}

void EditorGUI::drawAssetTreeNode(sf::RenderWindow& window, const AssetEntry& entry, float& yPos) {
    sf::Text entryText;
    entryText.setFont(m_font);
    std::string icon = entry.isDirectory ? "[+] " : "    ";
    entryText.setString(icon + entry.name);
    entryText.setCharacterSize(m_fontSize);
    entryText.setFillColor(entry.isDirectory ? sf::Color(200, 200, 150) : sf::Color(180, 180, 180));
    entryText.setPosition(20, yPos);
    window.draw(entryText);
    yPos += 20;
}

void EditorGUI::setEntities(const std::vector<EntityNode>& entities) {
    m_entities = entities;
}

void EditorGUI::setComponents(int entityId, const std::vector<ComponentInfo>& components) {
    m_selectedEntityId = entityId;
    m_components = components;
}

void EditorGUI::setAssets(const AssetEntry& root) {
    m_assetRoot = root;
}

void EditorGUI::setSelectedEntity(int entityId) {
    m_selectedEntityId = entityId;
}

int EditorGUI::getSelectedEntity() const {
    return m_selectedEntityId;
}

void EditorGUI::setToolbarState(bool playing, bool paused) {
    m_playing = playing;
    m_paused = paused;
}

std::string EditorGUI::getSearchFilter() const {
    return m_searchFilter;
}

void EditorGUI::openColorPicker(const std::string& label, const sf::Color& initialColor) {
    m_colorPicker.open = true;
    m_colorPicker.currentColor = initialColor;
    m_colorPicker.previewColor = initialColor;
    m_colorPicker.r = initialColor.r;
    m_colorPicker.g = initialColor.g;
    m_colorPicker.b = initialColor.b;
    m_colorPicker.a = initialColor.a;
    m_colorPickerResultReady = false;
}

bool EditorGUI::isColorPickerOpen() const {
    return m_colorPicker.open;
}

bool EditorGUI::getColorPickerResult(sf::Color& outColor) const {
    if (m_colorPickerResultReady) {
        outColor = m_colorPickerResult;
        return true;
    }
    return false;
}

void EditorGUI::closeColorPicker() {
    m_colorPicker.open = false;
    m_colorPickerResult = m_colorPicker.previewColor;
    m_colorPickerResultReady = true;
}

void EditorGUI::setAssetPreview(const AssetPreviewInfo& preview) {
    m_assetPreview = preview;
    m_hasAssetPreview = true;
}

void EditorGUI::clearAssetPreview() {
    m_hasAssetPreview = false;
}

void EditorGUI::setAnimationPreview(const AnimationPreviewState& state) {
    m_animationPreview = state;
}

AnimationPreviewState& EditorGUI::getAnimationPreview() {
    return m_animationPreview;
}

void EditorGUI::updateAnimationPreview(float dt) {
    if (m_animationPreview.playing) {
        m_animationPreview.currentTime += dt;
    }
}

void EditorGUI::setSceneStats(const SceneStats& stats) {
    m_sceneStats = stats;
}

SceneStats EditorGUI::getSceneStats() const {
    return m_sceneStats;
}

bool EditorGUI::isConsolePanelVisible() const {
    return m_consolePanelVisible;
}

void EditorGUI::setConsolePanelVisible(bool visible) {
    m_consolePanelVisible = visible;
}

void EditorGUI::shutdown() {
    m_entities.clear();
    m_components.clear();
}

} // namespace editor
} // namespace tools
} // namespace nebula
