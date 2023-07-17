#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <functional>
#include <fstream>

namespace nebula {
namespace tools {
namespace debug {

enum class LogLevel {
    Info,
    Warning,
    Error,
    Debug
};

struct LogEntry {
    std::string text;
    LogLevel level;
    uint64_t timestamp;
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

struct OutputFilter {
    bool showInfo;
    bool showWarning;
    bool showError;
    bool showDebug;
    std::string searchText;
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
    void printDebug(const std::string& text);
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

    void saveHistory(const std::string& filename);
    void loadHistory(const std::string& filename);

    void setOutputFilter(const OutputFilter& filter);
    OutputFilter getOutputFilter() const;

    void startOutputToFile(const std::string& filename);
    void stopOutputToFile();

    void updateAutocompleteSuggestions();
    std::vector<std::string> getAutocompleteSuggestions(const std::string& prefix) const;

    void setMultilineInput(bool enable);
    bool isMultilineInputEnabled() const;

private:
    void processInput();
    void updateAutocomplete();
    void navigateHistory(int direction);
    void renderOutput(sf::RenderWindow& window);
    void renderInput(sf::RenderWindow& window);
    void renderFilterBar(sf::RenderWindow& window);
    void renderAutocompleteDropdown(sf::RenderWindow& window);
    void writeOutputToFile(const LogEntry& entry);
    bool passesFilter(const LogEntry& entry) const;

    bool m_visible;
    sf::Keyboard::Key m_toggleKey;

    sf::Font m_font;
    unsigned int m_fontSize;

    sf::Color m_bgColor;
    sf::Color m_textColor;
    sf::Color m_warningColor;
    sf::Color m_errorColor;
    sf::Color m_debugColor;

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

    std::string m_historyFilePath;
    OutputFilter m_filter;

    std::ofstream m_outputFileStream;
    bool m_outputToFile;

    bool m_multilineInput;
    std::vector<std::string> m_inputLines;
};

} // namespace debug
} // namespace tools
} // namespace nebula
