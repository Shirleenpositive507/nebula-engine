#include "Console.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cmath>

namespace nebula {
namespace tools {
namespace debug {

Console::Console()
    : m_visible(false)
    , m_toggleKey(sf::Keyboard::Tilde)
    , m_fontSize(14)
    , m_bgColor(0, 0, 0, 200)
    , m_textColor(220, 220, 220)
    , m_warningColor(255, 200, 0)
    , m_errorColor(255, 60, 60)
    , m_historyIndex(-1)
    , m_autocompleteIndex(-1)
    , m_scrollOffset(0.0f)
    , m_consoleSize(800.0f, 300.0f)
    , m_ctrlPressed(false)
{
}

Console::~Console() {
    shutdown();
}

Console& Console::getInstance() {
    static Console instance;
    return instance;
}

void Console::init() {
    if (!m_font.loadFromFile("resources/fonts/consolas.ttf")) {
        if (!m_font.loadFromFile("resources/fonts/DejaVuSansMono.ttf")) {
            m_font.loadFromFile("C:/Windows/Fonts/consola.ttf");
        }
    }

    m_background.setSize(m_consoleSize);
    m_background.setFillColor(m_bgColor);

    registerBuiltinCommands();

    print("Nebula Engine Developer Console");
    print("Type 'help' for available commands.");
}

void Console::update(float dt) {
    if (!m_visible) return;
}

void Console::render(sf::RenderWindow& window) {
    if (!m_visible) return;

    sf::Vector2u winSize = window.getSize();
    sf::Vector2f consolePos(0, static_cast<float>(winSize.y) - m_consoleSize.y);

    m_background.setPosition(consolePos);
    m_background.setSize(sf::Vector2f(static_cast<float>(winSize.x), m_consoleSize.y));
    window.draw(m_background);

    renderOutput(window);
    renderInput(window);
}

void Console::renderOutput(sf::RenderWindow& window) {
    sf::Vector2u winSize = window.getSize();
    float startY = static_cast<float>(winSize.y) - m_consoleSize.y + 5.0f;
    float outputHeight = m_consoleSize.y - m_fontSize - 20.0f;

    size_t startLine = static_cast<size_t>(std::max(0.0f, m_scrollOffset));
    float yPos = startY;

    size_t visibleLines = static_cast<size_t>(outputHeight / (m_fontSize + 2));
    size_t count = std::min(visibleLines, m_outputBuffer.size() - startLine);

    for (size_t i = 0; i < count; ++i) {
        size_t idx = startLine + i;
        if (idx >= m_outputBuffer.size()) break;

        const auto& entry = m_outputBuffer[idx];

        sf::Text text;
        text.setFont(m_font);
        text.setString(entry.text);
        text.setCharacterSize(m_fontSize);

        switch (entry.level) {
            case LogLevel::Info:    text.setFillColor(m_textColor); break;
            case LogLevel::Warning: text.setFillColor(m_warningColor); break;
            case LogLevel::Error:   text.setFillColor(m_errorColor); break;
        }

        text.setPosition(5.0f, yPos);
        window.draw(text);
        yPos += m_fontSize + 2;
    }
}

void Console::renderInput(sf::RenderWindow& window) {
    sf::Vector2u winSize = window.getSize();
    float inputY = static_cast<float>(winSize.y) - m_fontSize - 8.0f;

    sf::Text promptText;
    promptText.setFont(m_font);
    promptText.setString("> " + m_inputText + "_");
    promptText.setCharacterSize(m_fontSize);
    promptText.setFillColor(sf::Color::Green);
    promptText.setPosition(5.0f, inputY);
    window.draw(promptText);
}

void Console::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == m_toggleKey) {
            toggle();
            return;
        }

        if (!m_visible) return;

        if (event.key.code == sf::Keyboard::Return) {
            processInput();
        }
        else if (event.key.code == sf::Keyboard::Up) {
            navigateHistory(-1);
        }
        else if (event.key.code == sf::Keyboard::Down) {
            navigateHistory(1);
        }
        else if (event.key.code == sf::Keyboard::Tab) {
            autocomplete();
        }
        else if (event.key.code == sf::Keyboard::LControl ||
                 event.key.code == sf::Keyboard::RControl) {
            m_ctrlPressed = true;
        }
        else if (event.key.code == sf::Keyboard::L && m_ctrlPressed) {
            clearOutput();
        }
    }
    else if (event.type == sf::Event::KeyReleased) {
        if (event.key.code == sf::Keyboard::LControl ||
            event.key.code == sf::Keyboard::RControl) {
            m_ctrlPressed = false;
        }
    }
    else if (event.type == sf::Event::TextEntered && m_visible) {
        if (event.text.unicode == 8) {
            if (!m_inputText.empty()) {
                m_inputText.pop_back();
            }
        }
        else if (event.text.unicode >= 32 && event.text.unicode < 127) {
            m_inputText += static_cast<char>(event.text.unicode);
        }
    }
    else if (event.type == sf::Event::MouseWheelScrolled && m_visible) {
        if (event.mouseWheelScroll.delta > 0) {
            m_scrollOffset = std::max(0.0f, m_scrollOffset - 1.0f);
        } else {
            float maxScroll = std::max(0.0f, static_cast<float>(m_outputBuffer.size()) - 15.0f);
            m_scrollOffset = std::min(maxScroll, m_scrollOffset + 1.0f);
        }
    }
}

void Console::processInput() {
    if (m_inputText.empty()) return;

    m_commandHistory.push_back(m_inputText);
    if (m_commandHistory.size() > MAX_HISTORY) {
        m_commandHistory.pop_front();
    }
    m_historyIndex = -1;

    print("> " + m_inputText);
    executeCommand(m_inputText);
    m_inputText.clear();
}

void Console::executeCommand(const std::string& cmd) {
    std::istringstream iss(cmd);
    std::string commandName;
    iss >> commandName;

    if (commandName.empty()) return;

    std::vector<std::string> args;
    std::string arg;
    while (iss >> arg) {
        args.push_back(arg);
    }

    auto it = m_commands.find(commandName);
    if (it != m_commands.end()) {
        it->second.callback(args);
    } else {
        printError("Unknown command: " + commandName);
    }
}

void Console::registerCommand(const std::string& name,
                               std::function<void(const std::vector<std::string>&)> callback,
                               const std::string& help) {
    m_commands[name] = {name, std::move(callback), help};
}

void Console::unregisterCommand(const std::string& name) {
    m_commands.erase(name);
}

void Console::registerVariable(const std::string& name, void* variable,
                               const std::string& type, const std::string& help) {
    m_variables[name] = {name, variable, type, help};
}

void Console::unregisterVariable(const std::string& name) {
    m_variables.erase(name);
}

void Console::print(const std::string& text) {
    m_outputBuffer.push_back({text, LogLevel::Info});
    if (m_outputBuffer.size() > MAX_OUTPUT) {
        m_outputBuffer.erase(m_outputBuffer.begin());
    }
}

void Console::printWarning(const std::string& text) {
    m_outputBuffer.push_back({text, LogLevel::Warning});
    if (m_outputBuffer.size() > MAX_OUTPUT) {
        m_outputBuffer.erase(m_outputBuffer.begin());
    }
}

void Console::printError(const std::string& text) {
    m_outputBuffer.push_back({text, LogLevel::Error});
    if (m_outputBuffer.size() > MAX_OUTPUT) {
        m_outputBuffer.erase(m_outputBuffer.begin());
    }
}

void Console::clearOutput() {
    m_outputBuffer.clear();
}

void Console::show() {
    m_visible = true;
}

void Console::hide() {
    m_visible = false;
}

void Console::toggle() {
    m_visible = !m_visible;
    if (m_visible) {
        m_scrollOffset = std::max(0.0f, static_cast<float>(m_outputBuffer.size()) - 15.0f);
    }
}

bool Console::isVisible() const {
    return m_visible;
}

void Console::setFont(const sf::Font& font) {
    m_font = font;
}

void Console::setFontSize(unsigned int size) {
    m_fontSize = size;
}

void Console::setColors(const sf::Color& bg, const sf::Color& text,
                        const sf::Color& warning, const sf::Color& error) {
    m_bgColor = bg;
    m_textColor = text;
    m_warningColor = warning;
    m_errorColor = error;
    m_background.setFillColor(bg);
}

void Console::navigateHistory(int direction) {
    if (m_commandHistory.empty()) return;

    m_historyIndex += direction;
    if (m_historyIndex < 0) {
        m_historyIndex = -1;
        m_inputText.clear();
        return;
    }

    size_t idx = static_cast<size_t>(m_historyIndex);
    size_t histSize = m_commandHistory.size();

    if (idx >= histSize) {
        m_historyIndex = static_cast<int>(histSize) - 1;
        idx = histSize - 1;
    }

    m_inputText = m_commandHistory[histSize - 1 - idx];
}

void Console::autocomplete() {
    if (m_inputText.empty()) return;

    std::vector<std::string> matches;
    for (const auto& cmd : m_commands) {
        if (cmd.first.find(m_inputText) == 0) {
            matches.push_back(cmd.first);
        }
    }

    if (matches.empty()) return;

    if (matches.size() == 1) {
        m_inputText = matches[0] + " ";
        return;
    }

    m_autocompleteIndex = (m_autocompleteIndex + 1) % static_cast<int>(matches.size());
    m_autocompleteMatches = matches;

    print("Suggestions: ");
    std::string suggestions;
    for (size_t i = 0; i < matches.size(); ++i) {
        suggestions += matches[i];
        if (i < matches.size() - 1) suggestions += ", ";
    }
    print(suggestions);

    m_inputText = matches[m_autocompleteIndex];
}

void Console::registerBuiltinCommands() {
    registerCommand("help", [this](const std::vector<std::string>&) {
        print("Available commands:");
        for (const auto& cmd : m_commands) {
            std::string helpText = "  " + cmd.first;
            if (!cmd.second.help.empty()) {
                helpText += " - " + cmd.second.help;
            }
            print(helpText);
        }
        if (!m_variables.empty()) {
            print("Variables:");
            for (const auto& var : m_variables) {
                std::string varText = "  " + var.first + " (" + var.second.type + ")";
                if (!var.second.help.empty()) {
                    varText += " - " + var.second.help;
                }
                print(varText);
            }
        }
    }, "Show this help message");

    registerCommand("clear", [this](const std::vector<std::string>&) {
        clearOutput();
    }, "Clear console output");

    registerCommand("exec", [this](const std::vector<std::string>& args) {
        if (args.empty()) {
            printError("Usage: exec <filename>");
            return;
        }
        print("Executing script: " + args[0] + " (not implemented)");
    }, "Execute a script file");

    registerCommand("echo", [this](const std::vector<std::string>& args) {
        std::string message;
        for (const auto& a : args) {
            if (!message.empty()) message += " ";
            message += a;
        }
        print(message);
    }, "Print text to console");

    registerCommand("quit", [this](const std::vector<std::string>&) {
        print("Quit command received");
    }, "Quit the application");

    registerCommand("fps", [this](const std::vector<std::string>& args) {
        if (!args.empty()) {
            print("FPS command with args: " + args[0]);
        } else {
            print("FPS display toggled");
        }
    }, "Toggle or set FPS display");

    registerCommand("vsync", [this](const std::vector<std::string>& args) {
        if (args.empty()) {
            print("Usage: vsync <on|off>");
            return;
        }
        print("Vsync: " + args[0]);
    }, "Toggle vsync on/off");

    registerCommand("teleport", [this](const std::vector<std::string>& args) {
        if (args.size() < 3) {
            printError("Usage: teleport <x> <y> <z>");
            return;
        }
        print("Teleporting to (" + args[0] + ", " + args[1] + ", " + args[2] + ")");
    }, "Teleport to coordinates");

    registerCommand("spawn", [this](const std::vector<std::string>& args) {
        if (args.empty()) {
            printError("Usage: spawn <entity_type>");
            return;
        }
        print("Spawning entity: " + args[0]);
    }, "Spawn an entity");

    registerCommand("set", [this](const std::vector<std::string>& args) {
        if (args.size() < 2) {
            printError("Usage: set <variable> <value>");
            return;
        }
        print("Set " + args[0] + " = " + args[1]);
    }, "Set a variable value");

    registerCommand("get", [this](const std::vector<std::string>& args) {
        if (args.empty()) {
            printError("Usage: get <variable>");
            return;
        }
        print("Get " + args[0]);
    }, "Get a variable value");
}

void Console::shutdown() {
    m_commands.clear();
    m_variables.clear();
    m_outputBuffer.clear();
    m_commandHistory.clear();
}

} // namespace debug
} // namespace tools
} // namespace nebula
