#include "Shader.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace nebula {
    namespace graphics {

        unsigned int Shader::s_activeShaderCount = 0;

        Shader::Shader()
            : m_name("unnamed")
            , m_shader(std::make_unique<sf::Shader>())
            , m_hotReloadEnabled(false)
            , m_lastFileCheck(std::chrono::system_clock::now()) {}

        Shader::Shader(const std::string& name)
            : m_name(name)
            , m_shader(std::make_unique<sf::Shader>())
            , m_hotReloadEnabled(false)
            , m_lastFileCheck(std::chrono::system_clock::now()) {}

        Shader::~Shader() {
            if (m_shader && m_shader->isAvailable()) {
                --s_activeShaderCount;
            }
        }

        bool Shader::loadFromFile(const std::string& vertexPath, const std::string& fragmentPath) {
            m_vertexPath = vertexPath;
            m_fragmentPath = fragmentPath;

            std::ifstream vFile(vertexPath);
            std::ifstream fFile(fragmentPath);

            if (!vFile.is_open() || !fFile.is_open()) {
                m_lastResult.success = false;
                m_lastResult.errorLog = "Failed to open shader file(s)";
                return false;
            }

            std::stringstream vStream, fStream;
            vStream << vFile.rdbuf();
            fStream << fFile.rdbuf();
            vFile.close();
            fFile.close();

            m_vertexSource = vStream.str();
            m_fragmentSource = fStream.str();

            return compileInternal();
        }

        bool Shader::loadFromMemory(const std::string& vertexSource, const std::string& fragmentSource) {
            storeSource(vertexSource, fragmentSource);
            return compileInternal();
        }

        bool Shader::loadFromSource(const std::string& vertexSource, const std::string& fragmentSource) {
            return loadFromMemory(vertexSource, fragmentSource);
        }

        void Shader::bind() const {
            if (m_shader) {
                sf::Shader::bind(m_shader.get());
            }
        }

        void Shader::unbind() {
            sf::Shader::bind(nullptr);
        }

        void Shader::setUniform(const std::string& name, float value) {
            if (m_shader) m_shader->setUniform(name, value);
        }

        void Shader::setUniform(const std::string& name, const sf::Vector2f& value) {
            if (m_shader) m_shader->setUniform(name, value);
        }

        void Shader::setUniform(const std::string& name, const sf::Vector3f& value) {
            if (m_shader) m_shader->setUniform(name, sf::Glsl::Vec3(value.x, value.y, value.z));
        }

        void Shader::setUniform(const std::string& name, const sf::Glsl::Vec4& value) {
            if (m_shader) m_shader->setUniform(name, value);
        }

        void Shader::setUniform(const std::string& name, const sf::Glsl::Mat3& value) {
            if (m_shader) m_shader->setUniform(name, value);
        }

        void Shader::setUniform(const std::string& name, const sf::Glsl::Mat4& value) {
            if (m_shader) m_shader->setUniform(name, value);
        }

        void Shader::setUniform(const std::string& name, int value) {
            if (m_shader) m_shader->setUniform(name, value);
        }

        void Shader::setUniform(const std::string& name, const sf::Texture& texture) {
            if (m_shader) m_shader->setUniform(name, texture);
        }

        void Shader::setUniform(const std::string& name, const sf::Shader::CurrentTextureType& texture) {
            if (m_shader) m_shader->setUniform(name, texture);
        }

        void Shader::setUniformArray(const std::string& name, const float* values, std::size_t count) {
            if (m_shader) m_shader->setUniformArray(name, values, count);
        }

        void Shader::setUniformArray(const std::string& name, const sf::Vector2f* values, std::size_t count) {
            if (m_shader) m_shader->setUniformArray(name, values, count);
        }

        void Shader::setUniformArray(const std::string& name, const int* values, std::size_t count) {
            if (m_shader) m_shader->setUniformArray(name, values, count);
        }

        void Shader::setUniformBlock(const std::string& name, unsigned int blockIndex) {
        }

        const std::string& Shader::getName() const {
            return m_name;
        }

        bool Shader::isValid() const {
            return m_shader && m_lastResult.success;
        }

        std::string Shader::getVertexPath() const {
            return m_vertexPath;
        }

        std::string Shader::getFragmentPath() const {
            return m_fragmentPath;
        }

        ShaderCompileResult Shader::getLastCompileResult() const {
            return m_lastResult;
        }

        void Shader::enableHotReload(bool enable) {
            m_hotReloadEnabled = enable;
            m_lastFileCheck = std::chrono::system_clock::now();
        }

        bool Shader::isHotReloadEnabled() const {
            return m_hotReloadEnabled;
        }

        bool Shader::checkForChanges() {
            if (!m_hotReloadEnabled || m_vertexPath.empty() || m_fragmentPath.empty()) {
                return false;
            }

            auto now = std::chrono::system_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastFileCheck).count();
            if (elapsed < 500) {
                return false;
            }
            m_lastFileCheck = now;

            std::ifstream vFile(m_vertexPath);
            std::ifstream fFile(m_fragmentPath);
            if (!vFile.is_open() || !fFile.is_open()) {
                return false;
            }

            std::stringstream vStream, fStream;
            vStream << vFile.rdbuf();
            fStream << fFile.rdbuf();

            std::string newVertexSource = vStream.str();
            std::string newFragmentSource = fStream.str();

            if (newVertexSource != m_vertexSource || newFragmentSource != m_fragmentSource) {
                m_vertexSource = newVertexSource;
                m_fragmentSource = newFragmentSource;
                bool success = compileInternal();
                if (m_reloadCallback) {
                    m_reloadCallback(m_name, success);
                }
                return true;
            }

            return false;
        }

        void Shader::setReloadCallback(ReloadCallback callback) {
            m_reloadCallback = callback;
        }

        bool Shader::isAvailable() {
            return sf::Shader::isAvailable();
        }

        unsigned int Shader::getActiveShaderCount() {
            return s_activeShaderCount;
        }

        bool Shader::compileInternal() {
            if (!sf::Shader::isAvailable()) {
                m_lastResult.success = false;
                m_lastResult.errorLog = "Shaders are not supported on this system";
                return false;
            }

            auto newShader = std::make_unique<sf::Shader>();
            bool loaded = false;

            if (!m_vertexPath.empty() && !m_fragmentPath.empty()) {
                loaded = newShader->loadFromFile(m_vertexPath, m_fragmentPath);
            } else {
                if (m_vertexSource.find("void main") != std::string::npos &&
                    m_fragmentSource.find("void main") != std::string::npos) {
                    loaded = newShader->loadFromMemory(m_vertexSource, m_fragmentSource);
                } else {
                    loaded = newShader->loadFromMemory(m_fragmentSource, sf::Shader::Fragment);
                }
            }

            m_lastResult.success = loaded;
            m_lastResult.errorLog = loaded ? "" : "Failed to load shader";
            m_lastResult.vertexSource = m_vertexSource;
            m_lastResult.fragmentSource = m_fragmentSource;

            if (loaded) {
                if (m_shader) --s_activeShaderCount;
                m_shader = std::move(newShader);
                ++s_activeShaderCount;
            }

            return loaded;
        }

        void Shader::storeSource(const std::string& vertexSource, const std::string& fragmentSource) {
            m_vertexSource = vertexSource;
            m_fragmentSource = fragmentSource;
            m_vertexPath.clear();
            m_fragmentPath.clear();
        }

    }
}
