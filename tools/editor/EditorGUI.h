#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <functional>

namespace nebula {
namespace tools {
namespace editor {

struct EntityNode {
    int id;
    std::string name;
    int parentId;
    std::vector<int> children;
    bool expanded;
    bool selected;
};

struct ComponentInfo {
    std::string type;
    std::string summary;
    bool enabled;
};

struct AssetEntry {
    std::string name;
    std::string path;
    std::string type;
    bool isDirectory;
    std::vector<AssetEntry> children;
};

class EditorGUI {
public:
    EditorGUI();
    ~EditorGUI();

    void init(sf::RenderWindow& window);
    void update(float dt);
    void render(sf::RenderWindow& window);
    void shutdown();

    void setEntities(const std::vector<EntityNode>& entities);
    void setComponents(int entityId, const std::vector<ComponentInfo>& components);
    void setAssets(const AssetEntry& root);

    void setSelectedEntity(int entityId);
    int getSelectedEntity() const;

    void setToolbarState(bool playing, bool paused);

    std::string getSearchFilter() const;

private:
    void renderSceneHierarchy(sf::RenderWindow& window);
    void renderProperties(sf::RenderWindow& window);
    void renderAssetBrowser(sf::RenderWindow& window);
    void renderToolbar(sf::RenderWindow& window);
    void renderMenuBar(sf::RenderWindow& window);

    void drawEntityTreeNode(sf::RenderWindow& window, const EntityNode& node, float& yPos);
    void drawComponentEditor(sf::RenderWindow& window, const ComponentInfo& comp, float& yPos);
    void drawAssetTreeNode(sf::RenderWindow& window, const AssetEntry& entry, float& yPos);

    sf::Font m_font;
    unsigned int m_fontSize;

    std::vector<EntityNode> m_entities;
    std::vector<ComponentInfo> m_components;
    AssetEntry m_assetRoot;

    int m_selectedEntityId;
    bool m_playing;
    bool m_paused;

    std::string m_searchFilter;
    bool m_filterActive;
    char m_filterBuffer[256];
    int m_filterCursor;

    // Layout state
    float m_hierarchyWidth;
    float m_propertiesWidth;
    float m_assetBrowserHeight;
    float m_toolbarHeight;
    float m_menuBarHeight;

    sf::RectangleShape m_panelBg;
    sf::RectangleShape m_divider;
};

} // namespace editor
} // namespace tools
} // namespace nebula
