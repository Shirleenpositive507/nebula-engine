#include "Editor.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace nebula {
namespace tools {
namespace editor {

Editor::Editor()
    : m_mode(EditorMode::Edit)
    , m_initialized(false)
    , m_selectedEntityId(-1)
    , m_nextEntityId(1)
    , m_cameraZoom(1.0f)
{
}

Editor::~Editor() {
    shutdown();
}

bool Editor::initialize(sf::RenderWindow& window) {
    m_viewport.init(800, 600);
    m_gui.init(window);
    m_debugOverlay.init();
    m_console.init();

    m_editorCamera.setSize(static_cast<sf::Vector2f>(window.getSize()));
    m_editorCamera.setCenter(0.0f, 0.0f);

    m_initialized = true;
    return true;
}

void Editor::update(float dt) {
    if (!m_initialized) return;

    m_console.update(dt);

    if (m_mode == EditorMode::Play) {
        updatePlayMode(dt);
    }

    m_viewport.update(dt);
    m_gui.update(dt);
    m_debugOverlay.update(dt);
}

void Editor::updatePlayMode(float dt) {
    if (m_mode == EditorMode::Pause) return;
}

void Editor::render(sf::RenderWindow& window) {
    if (!m_initialized) return;

    m_viewport.render({});
    m_gui.render(window);

    sf::Sprite viewportSprite = m_viewport.getRenderSprite();
    sf::Vector2u winSize = window.getSize();
    float guiLeft = 250.0f;
    float guiRight = 300.0f;
    float guiTop = 64.0f;
    float guiBottom = 150.0f;
    float vpX = guiLeft;
    float vpY = guiTop;
    float vpW = static_cast<float>(winSize.x) - guiLeft - guiRight;
    float vpH = static_cast<float>(winSize.y) - guiTop - guiBottom;
    viewportSprite.setPosition(vpX, vpY);
    viewportSprite.setScale(vpW / viewportSprite.getLocalBounds().width,
                            vpH / viewportSprite.getLocalBounds().height);
    window.draw(viewportSprite);

    m_debugOverlay.render(window);
    m_console.render(window);
}

void Editor::handleInput(const sf::Event& event) {
    if (!m_initialized) return;

    m_console.handleEvent(event);
    m_viewport.handleInput(event);
    handleShortcuts(event);

    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::F3) {
            m_debugOverlay.toggle();
        }
    }
}

void Editor::handleShortcuts(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        bool ctrl = sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::RControl);

        if (ctrl) {
            switch (event.key.code) {
                case sf::Keyboard::S:
                    if (m_currentScenePath.empty()) {
                        m_console.print("Save: No scene path set");
                    } else {
                        saveScene(m_currentScenePath);
                    }
                    break;
                case sf::Keyboard::Z:
                    undo();
                    break;
                case sf::Keyboard::Y:
                    redo();
                    break;
                case sf::Keyboard::D:
                    if (m_selectedEntityId >= 0) {
                        duplicateEntity(m_selectedEntityId);
                    }
                    break;
                case sf::Keyboard::N:
                    newScene();
                    break;
                default:
                    break;
            }
        } else {
            switch (event.key.code) {
                case sf::Keyboard::F5:
                    toggleMode();
                    break;
                case sf::Keyboard::Delete:
                    if (m_selectedEntityId >= 0) {
                        deleteEntity(m_selectedEntityId);
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

EditorMode Editor::getMode() const {
    return m_mode;
}

void Editor::setMode(EditorMode mode) {
    m_mode = mode;
}

void Editor::toggleMode() {
    switch (m_mode) {
        case EditorMode::Edit:
            m_mode = EditorMode::Play;
            m_console.print("Entering Play mode");
            break;
        case EditorMode::Play:
            m_mode = EditorMode::Edit;
            m_console.print("Exiting Play mode");
            break;
        case EditorMode::Pause:
            m_mode = EditorMode::Play;
            m_console.print("Resuming Play mode");
            break;
    }
}

void Editor::newScene() {
    m_selectedEntityId = -1;
    m_nextEntityId = 1;
    m_currentScenePath.clear();
    m_undoStack = {};
    m_redoStack = {};
    m_console.print("New scene created");
}

bool Editor::loadScene(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        m_console.printError("Failed to load scene: " + filename);
        return false;
    }
    m_currentScenePath = filename;
    m_console.print("Scene loaded: " + filename);
    return true;
}

bool Editor::saveScene(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        m_console.printError("Failed to save scene: " + filename);
        return false;
    }
    m_currentScenePath = filename;
    m_console.print("Scene saved: " + filename);
    return true;
}

void Editor::undo() {
    if (m_undoStack.empty()) {
        m_console.print("Nothing to undo");
        return;
    }
    UndoCommand cmd = m_undoStack.top();
    m_undoStack.pop();
    m_redoStack.push(cmd);
    m_console.print("Undo: " + cmd.type);
}

void Editor::redo() {
    if (m_redoStack.empty()) {
        m_console.print("Nothing to redo");
        return;
    }
    UndoCommand cmd = m_redoStack.top();
    m_redoStack.pop();
    m_undoStack.push(cmd);
    m_console.print("Redo: " + cmd.type);
}

void Editor::pushUndo(const UndoCommand& cmd) {
    m_undoStack.push(cmd);
    if (m_undoStack.size() > MAX_UNDO) {
        std::stack<UndoCommand> temp;
        std::vector<UndoCommand> all;
        while (!m_undoStack.empty()) {
            all.push_back(m_undoStack.top());
            m_undoStack.pop();
        }
        for (int i = static_cast<int>(all.size()) - 2; i >= 0; --i) {
            temp.push(all[i]);
        }
        m_undoStack = temp;
    }
    while (!m_redoStack.empty()) m_redoStack.pop();
}

void Editor::selectEntity(int entityId) {
    m_selectedEntityId = entityId;
    m_gui.setSelectedEntity(entityId);
}

int Editor::getSelectedEntity() const {
    return m_selectedEntityId;
}

void Editor::duplicateEntity(int entityId) {
    UndoCommand cmd;
    cmd.type = "duplicate_entity";
    cmd.data = std::to_string(entityId);
    pushUndo(cmd);

    int newId = m_nextEntityId++;
    m_console.print("Duplicated entity " + std::to_string(entityId) +
                    " as entity " + std::to_string(newId));
}

void Editor::deleteEntity(int entityId) {
    UndoCommand cmd;
    cmd.type = "delete_entity";
    cmd.data = std::to_string(entityId);
    pushUndo(cmd);

    m_selectedEntityId = -1;
    m_gui.setSelectedEntity(-1);
    m_console.print("Deleted entity " + std::to_string(entityId));
}

void Editor::setEditorCamera(const sf::Vector2f& pos, float zoom) {
    m_editorCamera.setCenter(pos);
    m_editorCamera.setSize(800.0f * zoom, 600.0f * zoom);
    m_cameraTarget = pos;
    m_cameraZoom = zoom;
}

sf::Vector2f Editor::getEditorCameraPos() const {
    return m_editorCamera.getCenter();
}

EditorViewport& Editor::getViewport() {
    return m_viewport;
}

EditorGUI& Editor::getGUI() {
    return m_gui;
}

debug::DebugOverlay& Editor::getDebugOverlay() {
    return m_debugOverlay;
}

debug::Console& Editor::getConsole() {
    return m_console;
}

void Editor::shutdown() {
    if (!m_initialized) return;

    m_viewport.shutdown();
    m_gui.shutdown();
    m_debugOverlay.shutdown();
    m_console.shutdown();

    while (!m_undoStack.empty()) m_undoStack.pop();
    while (!m_redoStack.empty()) m_redoStack.pop();

    m_initialized = false;
}

} // namespace editor
} // namespace tools
} // namespace nebula
