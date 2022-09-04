#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <sstream>

namespace nebula {

    class XMLElement;

    class XMLDocument {
    public:
        std::unique_ptr<XMLElement> root;

        XMLElement* getRoot() const { return root.get(); }
    };

    class XMLElement {
    public:
        std::string name;
        std::unordered_map<std::string, std::string> attributes;
        std::vector<std::unique_ptr<XMLElement>> children;
        std::string text;

        XMLElement* parent = nullptr;

        XMLElement* getElement(const std::string& name) const;
        std::vector<XMLElement*> getElements(const std::string& name) const;

        std::string getAttribute(const std::string& name) const;
        int getAttributeAsInt(const std::string& name, int defaultValue = 0) const;
        float getAttributeAsFloat(const std::string& name, float defaultValue = 0.0f) const;
        bool getAttributeAsBool(const std::string& name, bool defaultValue = false) const;

        bool hasAttribute(const std::string& name) const;
        bool hasChild(const std::string& name) const;

        XMLElement* getChild(const std::string& name) const;
        std::vector<XMLElement*> getChildren(const std::string& name) const;

        std::string getText() const { return text; }
        XMLElement* firstChildElement() const;

    private:
        friend class XMLParser;
    };

    class XMLParser {
    public:
        XMLDocument parse(const std::string& xml);

    private:
        size_t m_pos = 0;
        std::string m_input;

        void skipWhitespace();
        void skipComment();
        void expect(char c);
        std::string parseName();
        std::string parseAttributeValue();
        std::unique_ptr<XMLElement> parseElement();
    };

    class XMLWriter {
    public:
        void writeStartDocument();
        void writeEndDocument();
        void writeStartElement(const std::string& name);
        void writeEndElement();
        void writeAttribute(const std::string& name, const std::string& value);
        void writeText(const std::string& text);

        std::string toString() const;
        void clear();

    private:
        std::ostringstream m_stream;
        std::vector<std::string> m_elementStack;
        bool m_hasContent = false;
    };

}
