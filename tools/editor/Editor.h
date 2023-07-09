#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <stack>
#include <memory>
#include <functional>
#include <unordered_map>
#include <fstream>

#include "EditorViewport.h"
#include "EditorGUI.h"
#include "../debug/DebugOverlay.h"
#include "../debug/Console.h"

namespace nebula {
namespace tools {
namespace editor {

enum class EditorMode {
    Edit,
    Play,
    Pause
};

struct UndoCommand {
    std::string type;
    std::string data;
};

struct EditorProject {
    std::string name;
    std::string version;
    std::string author;
    std::string lastScenePath;
    std::vector<std::string> recentScenes;
    std::vector<std::string> openScenes;
    int activeSceneIndex;
    uint64_t createdAt;
    uint64_t lastModifiedAt;
};

struct EditorLayout {
    std::unordered_map<std::string, sf::FloatRect> windowRects;
    std::unordered_map<std::string, bool> windowVisibility;
    std::unordered_map<std::string, std::string> dockTargets;
    std::vector<std::string> tabOrder;
    float hierarchyWidth;
    float propertiesWidth;
    float assetBrowserHeight;
    float consoleHeight;
};

struct EditorSettings {
    bool vsyncEnabled;
    bool showGrid;
    bool showRulers;
    bool snapToGrid;
    float snapSize;
    unsigned int fontSize;
    std::string theme;
    sf::Color backgroundColor;
    sf::Color gridColor;
    float cameraPanSpeed;
    float cameraZoomSpeed;
    std::string defaultScenePath;
    std::string defaultProjectPath;
    bool autoSave;
    float autoSaveInterval;
};

struct EditorShortcut {
    sf::Keyboard::Key key;
    bool ctrl;
    bool shift;
    bool alt;
    std::string action;
};

struct SceneDocument {
    int id;
    std::string name;
    std::string filePath;
    bool modified;
    bool active;
};

class Editor {
public:
    Editor();
    ~Editor();

    bool initialize(sf::RenderWindow& window);
    void update(float dt);
    void render(sf::RenderWindow& window);
    void handleInput(const sf::Event& event);
    void shutdown();

    EditorMode getMode() const;
    void setMode(EditorMode mode);
    void toggleMode();

    void newScene();
    bool loadScene(const std::string& filename);
    bool saveScene(const std::string& filename);

    void undo();
    void redo();
    void pushUndo(const UndoCommand& cmd);

    void selectEntity(int entityId);
    int getSelectedEntity() const;
    void duplicateEntity(int entityId);
    void deleteEntity(int entityId);

    void setEditorCamera(const sf::Vector2f& pos, float zoom);
    sf::Vector2f getEditorCameraPos() const;

    EditorViewport& getViewport();
    EditorGUI& getGUI();
    debug::DebugOverlay& getDebugOverlay();
    debug::Console& getConsole();

    int createProject(const std::string& name, const std::string& path);
    bool loadProject(const std::string& filename);
    bool saveProject();
    bool saveProjectAs(const std::string& filename);
    EditorProject& getProject();
    const EditorProject& getProject() const;

    void loadSettings(const std::string& filename);
    void saveSettings(const std::string& filename);
    EditorSettings& getSettings();
    const EditorSettings& getSettings() const;

    void loadLayout(const std::string& filename);
    void saveLayout(const std::string& filename);
    EditorLayout& getLayout();
    const EditorLayout& getLayout() const;

    void setCustomShortcut(const std::string& action, sf::Keyboard::Key key,
                           bool ctrl, bool shift, bool alt);
    void removeCustomShortcut(const std::string& action);
    const std::vector<EditorShortcut>& getCustomShortcuts() const;

    int openSceneDocument(const std::string& name);
    void closeSceneDocument(int docId);
    void setActiveSceneDocument(int docId);
    SceneDocument* getActiveSceneDocument();
    std::vector<SceneDocument>& getOpenDocuments();
    int getDocumentCount() const;

    void setCustomShortcutFile(const std::string& filename);

private:
    void handleShortcuts(const sf::Event& event);
    void updatePlayMode(float dt);
    void autoSaveTimer();
    void serializeProject(std::ofstream& file);
    void deserializeProject(std::ifstream& file);

    EditorMode m_mode;
    bool m_initialized;

    EditorViewport m_viewport;
    EditorGUI m_gui;
    debug::DebugOverlay m_debugOverlay;
    debug::Console m_console;

    std::stack<UndoCommand> m_undoStack;
    std::stack<UndoCommand> m_redoStack;
    static constexpr size_t MAX_UNDO = 100;

    std::string m_currentScenePath;
    int m_selectedEntityId;
    int m_nextEntityId;

    sf::View m_editorCamera;
    sf::Vector2f m_cameraTarget;
    float m_cameraZoom;

    sf::Clock m_stepClock;

    EditorProject m_project;
    EditorLayout m_layout;
    EditorSettings m_settings;
    std::vector<EditorShortcut> m_customShortcuts;
    std::vector<SceneDocument> m_openDocuments;
    int m_nextDocId;
    std::string m_projectFilePath;
    std::string m_settingsFilePath;
    std::string m_layoutFilePath;
    std::string m_shortcutFilePath;
};

} // namespace editor
} // namespace tools
} // namespace nebula
