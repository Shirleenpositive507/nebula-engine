#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <functional>

namespace nebula {
namespace tools {
namespace debug {

enum class LogLevel {
    Info,
    Warning,
    Error
};

struct LogEntry {
    std::string text;
    LogLevel level;
};

struct ConsoleCommand {
    std::string name;
    std::function<void(const std::vector<std::string>&)> callback;
    std::string help;
};

struct ConsoleVariable {
    std::string name;
    void* variable;
    std::string type;
    std::string help;
};

class Console {
public:
    Console();
    ~Console();

    void init();
    void update(float dt);
    void render(sf::RenderWindow& window);
    void handleEvent(const sf::Event& event);
    void shutdown();

    void executeCommand(const std::string& cmd);
    void registerCommand(const std::string& name,
                         std::function<void(const std::vector<std::string>&)> callback,
                         const std::string& help = "");
    void unregisterCommand(const std::string& name);
    void registerVariable(const std::string& name, void* variable,
                          const std::string& type, const std::string& help = "");
    void unregisterVariable(const std::string& name);

    void print(const std::string& text);
    void printWarning(const std::string& text);
    void printError(const std::string& text);
    void clearOutput();

    void show();
    void hide();
    void toggle();

    bool isVisible() const;
    void setFont(const sf::Font& font);
    void setFontSize(unsigned int size);
    void setColors(const sf::Color& bg, const sf::Color& text,
                   const sf::Color& warning, const sf::Color& error);

    void registerBuiltinCommands();

    static Console& getInstance();

private:
    void processInput();
    void autocomplete();
    void navigateHistory(int direction);
    void renderOutput(sf::RenderWindow& window);
    void renderInput(sf::RenderWindow& window);

    bool m_visible;
    sf::Keyboard::Key m_toggleKey;

    sf::Font m_font;
    unsigned int m_fontSize;

    sf::Color m_bgColor;
    sf::Color m_textColor;
    sf::Color m_warningColor;
    sf::Color m_errorColor;

    std::string m_inputText;
    std::vector<LogEntry> m_outputBuffer;
    std::deque<std::string> m_commandHistory;
    int m_historyIndex;
    static constexpr size_t MAX_HISTORY = 100;
    static constexpr size_t MAX_OUTPUT = 500;

    std::unordered_map<std::string, ConsoleCommand> m_commands;
    std::unordered_map<std::string, ConsoleVariable> m_variables;

    std::vector<std::string> m_autocompleteMatches;
    int m_autocompleteIndex;

    float m_scrollOffset;
    sf::Vector2f m_consoleSize;
    sf::RectangleShape m_background;
    bool m_ctrlPressed;
};

} // namespace debug
} // namespace tools
} // namespace nebula
