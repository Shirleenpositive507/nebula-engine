#include "XMLParser.h"
#include <cctype>
#include <stack>
#include <stdexcept>

namespace nebula {

    XMLElement* XMLElement::getElement(const std::string& name) const {
        for (auto& child : children) {
            if (child->name == name) return child.get();
        }
        return nullptr;
    }

    std::vector<XMLElement*> XMLElement::getElements(const std::string& name) const {
        std::vector<XMLElement*> result;
        for (auto& child : children) {
            if (child->name == name) result.push_back(child.get());
        }
        return result;
    }

    std::string XMLElement::getAttribute(const std::string& name) const {
        auto it = attributes.find(name);
        if (it != attributes.end()) return it->second;
        return std::string();
    }

    int XMLElement::getAttributeAsInt(const std::string& name, int defaultValue) const {
        std::string val = getAttribute(name);
        if (val.empty()) return defaultValue;
        return std::stoi(val);
    }

    float XMLElement::getAttributeAsFloat(const std::string& name, float defaultValue) const {
        std::string val = getAttribute(name);
        if (val.empty()) return defaultValue;
        return std::stof(val);
    }

    bool XMLElement::getAttributeAsBool(const std::string& name, bool defaultValue) const {
        std::string val = getAttribute(name);
        if (val.empty()) return defaultValue;
        std::string lower;
        for (char c : val) lower += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return lower == "true" || lower == "1" || lower == "yes";
    }

    bool XMLElement::hasAttribute(const std::string& name) const {
        return attributes.find(name) != attributes.end();
    }

    bool XMLElement::hasChild(const std::string& name) const {
        return getElement(name) != nullptr;
    }

    XMLElement* XMLElement::getChild(const std::string& name) const {
        return getElement(name);
    }

    std::vector<XMLElement*> XMLElement::getChildren(const std::string& name) const {
        return getElements(name);
    }

    XMLElement* XMLElement::firstChildElement() const {
        if (children.empty()) return nullptr;
        return children.front().get();
    }

    void XMLParser::skipWhitespace() {
        while (m_pos < m_input.size() && std::isspace(static_cast<unsigned char>(m_input[m_pos])))
            ++m_pos;
    }

    void XMLParser::skipComment() {
        if (m_input.substr(m_pos, 4) == "<!--") {
            size_t end = m_input.find("-->", m_pos);
            if (end != std::string::npos) {
                m_pos = end + 3;
            }
        }
    }

    void XMLParser::expect(char c) {
        if (m_pos >= m_input.size() || m_input[m_pos] != c) {
            throw std::runtime_error(std::string("XML: expected '") + c + "'");
        }
        ++m_pos;
    }

    std::string XMLParser::parseName() {
        size_t start = m_pos;
        while (m_pos < m_input.size() &&
               (std::isalnum(static_cast<unsigned char>(m_input[m_pos])) ||
                m_input[m_pos] == '_' || m_input[m_pos] == '-' || m_input[m_pos] == ':'))
            ++m_pos;
        if (m_pos == start) throw std::runtime_error("XML: expected name");
        return m_input.substr(start, m_pos - start);
    }

    std::string XMLParser::parseAttributeValue() {
        expect('"');
        size_t start = m_pos;
        while (m_pos < m_input.size() && m_input[m_pos] != '"')
            ++m_pos;
        std::string value = m_input.substr(start, m_pos - start);
        expect('"');
        return value;
    }

    std::unique_ptr<XMLElement> XMLParser::parseElement() {
        auto element = std::make_unique<XMLElement>();

        skipWhitespace();
        skipComment();
        expect('<');

        if (m_pos < m_input.size() && m_input[m_pos] == '/') {
            return nullptr;
        }

        element->name = parseName();

        skipWhitespace();
        while (m_pos < m_input.size() && m_input[m_pos] != '>' && !(m_pos + 1 < m_input.size() && m_input[m_pos] == '/' && m_input[m_pos + 1] == '>')) {
            skipWhitespace();
            if (m_input[m_pos] == '>' || (m_input[m_pos] == '/' && m_pos + 1 < m_input.size() && m_input[m_pos + 1] == '>'))
                break;
            std::string attrName = parseName();
            skipWhitespace();
            expect('=');
            skipWhitespace();
            std::string attrValue = parseAttributeValue();
            element->attributes[attrName] = attrValue;
            skipWhitespace();
        }

        bool selfClosing = false;
        if (m_input[m_pos] == '/') {
            selfClosing = true;
            ++m_pos;
        }
        expect('>');

        if (selfClosing) return element;

        while (m_pos < m_input.size()) {
            skipWhitespace();
            skipComment();

            if (m_pos >= m_input.size()) break;

            if (m_input[m_pos] == '<') {
                if (m_pos + 1 < m_input.size() && m_input[m_pos + 1] == '/') {
                    m_pos += 2;
                    std::string closingName = parseName();
                    skipWhitespace();
                    expect('>');
                    if (closingName != element->name) {
                        throw std::runtime_error("XML: mismatched closing tag: " + closingName + " expected " + element->name);
                    }
                    return element;
                } else {
                    auto child = parseElement();
                    if (child) {
                        child->parent = element.get();
                        element->children.push_back(std::move(child));
                    }
                }
            } else {
                size_t start = m_pos;
                while (m_pos < m_input.size() && m_input[m_pos] != '<')
                    ++m_pos;
                element->text += m_input.substr(start, m_pos - start);
            }
        }

        return element;
    }

    XMLDocument XMLParser::parse(const std::string& xml) {
        m_input = xml;
        m_pos = 0;

        static_cast<void>(skipWhitespace());
        static_cast<void>(skipComment());

        XMLDocument doc;
        doc.root = parseElement();
        if (!doc.root) throw std::runtime_error("XML: failed to parse root element");
        return doc;
    }

    void XMLWriter::writeStartDocument() {
        m_stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        m_hasContent = true;
    }

    void XMLWriter::writeEndDocument() {
        while (!m_elementStack.empty()) {
            writeEndElement();
        }
    }

    void XMLWriter::writeStartElement(const std::string& name) {
        std::string indent(m_elementStack.size() * 2, ' ');
        m_stream << indent << "<" << name;
        m_elementStack.push_back(name);
        m_hasContent = false;
    }

    void XMLWriter::writeEndElement() {
        if (m_elementStack.empty()) return;
        std::string name = m_elementStack.back();
        m_elementStack.pop_back();

        if (!m_hasContent) {
            m_stream << "/>\n";
            m_hasContent = true;
            return;
        }

        std::string indent(m_elementStack.size() * 2, ' ');
        m_stream << indent << "</" << name << ">\n";
    }

    void XMLWriter::writeAttribute(const std::string& name, const std::string& value) {
        m_stream << " " << name << "=\"" << value << "\"";
    }

    void XMLWriter::writeText(const std::string& text) {
        std::string escaped;
        for (char c : text) {
            switch (c) {
                case '<': escaped += "&lt;"; break;
                case '>': escaped += "&gt;"; break;
                case '&': escaped += "&amp;"; break;
                case '"': escaped += "&quot;"; break;
                default:  escaped += c; break;
            }
        }
        m_stream << escaped;
        m_hasContent = true;
    }

    std::string XMLWriter::toString() const {
        return m_stream.str();
    }

    void XMLWriter::clear() {
        m_stream.str("");
        m_stream.clear();
        m_elementStack.clear();
        m_hasContent = false;
    }

}
