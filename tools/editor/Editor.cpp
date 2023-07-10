#include "Editor.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <iomanip>

namespace nebula {
namespace tools {
namespace editor {

Editor::Editor()
    : m_mode(EditorMode::Edit)
    , m_initialized(false)
    , m_selectedEntityId(-1)
    , m_nextEntityId(1)
    , m_cameraZoom(1.0f)
    , m_nextDocId(1)
{
    m_settings.vsyncEnabled = true;
    m_settings.showGrid = true;
    m_settings.showRulers = true;
    m_settings.snapToGrid = false;
    m_settings.snapSize = 32.0f;
    m_settings.fontSize = 13;
    m_settings.theme = "dark";
    m_settings.backgroundColor = sf::Color(60, 60, 60);
    m_settings.gridColor = sf::Color(80, 80, 80, 100);
    m_settings.cameraPanSpeed = 1.0f;
    m_settings.cameraZoomSpeed = 1.1f;
    m_settings.autoSave = true;
    m_settings.autoSaveInterval = 300.0f;
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

    m_layout.hierarchyWidth = 250.0f;
    m_layout.propertiesWidth = 300.0f;
    m_layout.assetBrowserHeight = 150.0f;
    m_layout.consoleHeight = 200.0f;

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
    float guiLeft = m_layout.hierarchyWidth;
    float guiRight = m_layout.propertiesWidth;
    float guiTop = 64.0f;
    float guiBottom = m_layout.assetBrowserHeight;
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
        bool shift = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ||
                     sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);
        bool alt = sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt) ||
                   sf::Keyboard::isKeyPressed(sf::Keyboard::RAlt);

        for (const auto& sc : m_customShortcuts) {
            if (sc.key == event.key.code && sc.ctrl == ctrl &&
                sc.shift == shift && sc.alt == alt) {
                m_console.print("Shortcut: " + sc.action);
                return;
            }
        }

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
    m_project.lastScenePath = filename;
    m_console.print("Scene loaded: " + filename);

    auto it = std::find(m_project.recentScenes.begin(), m_project.recentScenes.end(), filename);
    if (it != m_project.recentScenes.end()) {
        m_project.recentScenes.erase(it);
    }
    m_project.recentScenes.insert(m_project.recentScenes.begin(), filename);
    if (m_project.recentScenes.size() > 10) {
        m_project.recentScenes.resize(10);
    }

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

int Editor::createProject(const std::string& name, const std::string& path) {
    m_project.name = name;
    m_project.version = "1.0";
    m_project.author = "User";
    m_project.lastScenePath.clear();
    m_project.recentScenes.clear();
    m_project.openScenes.clear();
    m_project.activeSceneIndex = -1;
    m_project.createdAt = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count()
    );
    m_project.lastModifiedAt = m_project.createdAt;
    m_projectFilePath = path + "/" + name + ".nebula";
    m_console.print("Project created: " + name);
    return 0;
}

bool Editor::loadProject(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        m_console.printError("Failed to load project: " + filename);
        return false;
    }
    deserializeProject(file);
    m_projectFilePath = filename;
    m_console.print("Project loaded: " + m_project.name);
    return true;
}

bool Editor::saveProject() {
    if (m_projectFilePath.empty()) return false;
    std::ofstream file(m_projectFilePath);
    if (!file.is_open()) {
        m_console.printError("Failed to save project");
        return false;
    }
    serializeProject(file);
    m_console.print("Project saved: " + m_project.name);
    return true;
}

bool Editor::saveProjectAs(const std::string& filename) {
    m_projectFilePath = filename;
    return saveProject();
}

EditorProject& Editor::getProject() {
    return m_project;
}

const EditorProject& Editor::getProject() const {
    return m_project;
}

void Editor::loadSettings(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return;
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key;
        if (std::getline(iss, key, '=')) {
            std::string value;
            if (std::getline(iss, value)) {
                if (key == "vsync") m_settings.vsyncEnabled = (value == "true");
                else if (key == "showGrid") m_settings.showGrid = (value == "true");
                else if (key == "showRulers") m_settings.showRulers = (value == "true");
                else if (key == "snapToGrid") m_settings.snapToGrid = (value == "true");
                else if (key == "snapSize") m_settings.snapSize = std::stof(value);
                else if (key == "fontSize") m_settings.fontSize = std::stoul(value);
                else if (key == "theme") m_settings.theme = value;
                else if (key == "autoSave") m_settings.autoSave = (value == "true");
            }
        }
    }
    m_settingsFilePath = filename;
}

void Editor::saveSettings(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) return;
    file << "vsync=" << (m_settings.vsyncEnabled ? "true" : "false") << "\n";
    file << "showGrid=" << (m_settings.showGrid ? "true" : "false") << "\n";
    file << "showRulers=" << (m_settings.showRulers ? "true" : "false") << "\n";
    file << "snapToGrid=" << (m_settings.snapToGrid ? "true" : "false") << "\n";
    file << "snapSize=" << m_settings.snapSize << "\n";
    file << "fontSize=" << m_settings.fontSize << "\n";
    file << "theme=" << m_settings.theme << "\n";
    file << "autoSave=" << (m_settings.autoSave ? "true" : "false") << "\n";
    m_settingsFilePath = filename;
}

EditorSettings& Editor::getSettings() {
    return m_settings;
}

const EditorSettings& Editor::getSettings() const {
    return m_settings;
}

void Editor::loadLayout(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return;
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key;
        if (std::getline(iss, key, '=')) {
            std::string value;
            if (std::getline(iss, value)) {
                if (key == "hierarchyWidth") m_layout.hierarchyWidth = std::stof(value);
                else if (key == "propertiesWidth") m_layout.propertiesWidth = std::stof(value);
                else if (key == "assetBrowserHeight") m_layout.assetBrowserHeight = std::stof(value);
                else if (key == "consoleHeight") m_layout.consoleHeight = std::stof(value);
            }
        }
    }
    m_layoutFilePath = filename;
}

void Editor::saveLayout(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) return;
    file << "hierarchyWidth=" << m_layout.hierarchyWidth << "\n";
    file << "propertiesWidth=" << m_layout.propertiesWidth << "\n";
    file << "assetBrowserHeight=" << m_layout.assetBrowserHeight << "\n";
    file << "consoleHeight=" << m_layout.consoleHeight << "\n";
    m_layoutFilePath = filename;
}

EditorLayout& Editor::getLayout() {
    return m_layout;
}

const EditorLayout& Editor::getLayout() const {
    return m_layout;
}

void Editor::setCustomShortcut(const std::string& action, sf::Keyboard::Key key,
                                bool ctrl, bool shift, bool alt) {
    for (auto& sc : m_customShortcuts) {
        if (sc.action == action) {
            sc.key = key;
            sc.ctrl = ctrl;
            sc.shift = shift;
            sc.alt = alt;
            return;
        }
    }
    m_customShortcuts.push_back({key, ctrl, shift, alt, action});
}

void Editor::removeCustomShortcut(const std::string& action) {
    m_customShortcuts.erase(
        std::remove_if(m_customShortcuts.begin(), m_customShortcuts.end(),
            [&action](const EditorShortcut& s) { return s.action == action; }),
        m_customShortcuts.end()
    );
}

const std::vector<EditorShortcut>& Editor::getCustomShortcuts() const {
    return m_customShortcuts;
}

int Editor::openSceneDocument(const std::string& name) {
    SceneDocument doc;
    doc.id = m_nextDocId++;
    doc.name = name;
    doc.modified = false;
    doc.active = true;
    m_openDocuments.push_back(doc);
    return doc.id;
}

void Editor::closeSceneDocument(int docId) {
    m_openDocuments.erase(
        std::remove_if(m_openDocuments.begin(), m_openDocuments.end(),
            [docId](const SceneDocument& d) { return d.id == docId; }),
        m_openDocuments.end()
    );
}

void Editor::setActiveSceneDocument(int docId) {
    for (auto& doc : m_openDocuments) {
        doc.active = (doc.id == docId);
    }
}

SceneDocument* Editor::getActiveSceneDocument() {
    for (auto& doc : m_openDocuments) {
        if (doc.active) return &doc;
    }
    return m_openDocuments.empty() ? nullptr : &m_openDocuments[0];
}

std::vector<SceneDocument>& Editor::getOpenDocuments() {
    return m_openDocuments;
}

int Editor::getDocumentCount() const {
    return static_cast<int>(m_openDocuments.size());
}

void Editor::setCustomShortcutFile(const std::string& filename) {
    m_shortcutFilePath = filename;
}

void Editor::serializeProject(std::ofstream& file) {
    file << "[Project]\n";
    file << "name=" << m_project.name << "\n";
    file << "version=" << m_project.version << "\n";
    file << "author=" << m_project.author << "\n";
    file << "lastScene=" << m_project.lastScenePath << "\n";
    file << "createdAt=" << m_project.createdAt << "\n";
    file << "lastModified=" << m_project.lastModifiedAt << "\n";
    file << "[RecentScenes]\n";
    for (const auto& scene : m_project.recentScenes) {
        file << "scene=" << scene << "\n";
    }
    file << "[OpenScenes]\n";
    for (const auto& scene : m_project.openScenes) {
        file << "scene=" << scene << "\n";
    }
}

void Editor::deserializeProject(std::ifstream& file) {
    std::string line;
    while (std::getline(file, line)) {
        if (line == "[Project]") continue;
        if (line == "[RecentScenes]") break;
        std::istringstream iss(line);
        std::string key;
        if (std::getline(iss, key, '=')) {
            std::string value;
            if (std::getline(iss, value)) {
                if (key == "name") m_project.name = value;
                else if (key == "version") m_project.version = value;
                else if (key == "author") m_project.author = value;
                else if (key == "lastScene") m_project.lastScenePath = value;
                else if (key == "createdAt") m_project.createdAt = std::stoull(value);
                else if (key == "lastModified") m_project.lastModifiedAt = std::stoull(value);
            }
        }
    }
    while (std::getline(file, line)) {
        if (line == "[OpenScenes]") break;
        std::string value = line.substr(line.find('=') + 1);
        m_project.recentScenes.push_back(value);
    }
    while (std::getline(file, line)) {
        std::string value = line.substr(line.find('=') + 1);
        m_project.openScenes.push_back(value);
    }
}

void Editor::shutdown() {
    if (!m_initialized) return;

    if (!m_projectFilePath.empty()) {
        saveProject();
    }
    if (!m_settingsFilePath.empty()) {
        saveSettings(m_settingsFilePath);
    }
    if (!m_layoutFilePath.empty()) {
        saveLayout(m_layoutFilePath);
    }

    m_viewport.shutdown();
    m_gui.shutdown();
    m_debugOverlay.shutdown();
    m_console.shutdown();

    while (!m_undoStack.empty()) m_undoStack.pop();
    while (!m_redoStack.empty()) m_redoStack.pop();

    m_openDocuments.clear();
    m_customShortcuts.clear();

    m_initialized = false;
}

} // namespace editor
} // namespace tools
} // namespace nebula
