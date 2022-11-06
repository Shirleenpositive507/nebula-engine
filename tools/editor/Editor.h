#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <stack>
#include <memory>
#include <functional>

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

private:
    void handleShortcuts(const sf::Event& event);
    void updatePlayMode(float dt);

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
};

} // namespace editor
} // namespace tools
} // namespace nebula
