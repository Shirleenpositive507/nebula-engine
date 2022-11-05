#include "EditorGUI.h"
#include <sstream>
#include <algorithm>
#include <cstring>

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
{
    std::memset(m_filterBuffer, 0, sizeof(m_filterBuffer));
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
}

void EditorGUI::render(sf::RenderWindow& window) {
    renderMenuBar(window);
    renderToolbar(window);
    renderSceneHierarchy(window);
    renderProperties(window);
    renderAssetBrowser(window);
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

    struct ToolButton {
        std::string label;
        bool active;
        std::function<void()> action;
    };

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

void EditorGUI::shutdown() {
    m_entities.clear();
    m_components.clear();
}

} // namespace editor
} // namespace tools
} // namespace nebula
