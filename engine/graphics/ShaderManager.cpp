#include "ShaderManager.h"
#include <sstream>

namespace nebula {
    namespace graphics {

        ShaderManager::ShaderManager()
            : m_hotReloadEnabled(false) {}

        ShaderManager::~ShaderManager() {
            releaseAll();
        }

        ShaderManager& ShaderManager::getInstance() {
            static ShaderManager instance;
            return instance;
        }

        std::shared_ptr<Shader> ShaderManager::loadShader(
            const std::string& name,
            const std::string& vertexPath,
            const std::string& fragmentPath)
        {
            auto it = m_shaders.find(name);
            if (it != m_shaders.end()) {
                if (it->second->isValid()) {
                    return it->second;
                }
            }

            auto shader = std::make_shared<Shader>(name);
            if (!shader->loadFromFile(vertexPath, fragmentPath)) {
                return nullptr;
            }

            shader->enableHotReload(m_hotReloadEnabled);
            m_shaders[name] = shader;
            return shader;
        }

        std::shared_ptr<Shader> ShaderManager::loadShaderFromMemory(
            const std::string& name,
            const std::string& vertexSource,
            const std::string& fragmentSource)
        {
            auto shader = std::make_shared<Shader>(name);
            std::string vSrc = injectDefines(vertexSource);
            std::string fSrc = injectDefines(fragmentSource);

            if (!shader->loadFromMemory(vSrc, fSrc)) {
                return nullptr;
            }

            m_shaders[name] = shader;
            return shader;
        }

        std::shared_ptr<Shader> ShaderManager::getShader(const std::string& name) const {
            auto it = m_shaders.find(name);
            if (it != m_shaders.end()) {
                return it->second;
            }
            return nullptr;
        }

        bool ShaderManager::unloadShader(const std::string& name) {
            auto it = m_shaders.find(name);
            if (it == m_shaders.end()) {
                return false;
            }
            m_shaders.erase(it);
            return true;
        }

        bool ShaderManager::hasShader(const std::string& name) const {
            return m_shaders.find(name) != m_shaders.end();
        }

        bool ShaderManager::reloadAll() {
            bool allSuccess = true;
            for (auto& pair : m_shaders) {
                if (!reloadShader(pair.first)) {
                    allSuccess = false;
                }
            }
            return allSuccess;
        }

        bool ShaderManager::reloadShader(const std::string& name) {
            auto it = m_shaders.find(name);
            if (it == m_shaders.end()) {
                return false;
            }

            auto& shader = it->second;
            if (!shader->getVertexPath().empty() && !shader->getFragmentPath().empty()) {
                return shader->loadFromFile(shader->getVertexPath(), shader->getFragmentPath());
            }

            return shader->loadFromMemory(
                shader->getLastCompileResult().vertexSource,
                shader->getLastCompileResult().fragmentSource
            );
        }

        void ShaderManager::addDefine(const std::string& define) {
            for (const auto& d : m_defines) {
                if (d == define) return;
            }
            m_defines.push_back(define);
        }

        void ShaderManager::removeDefine(const std::string& define) {
            auto it = std::find(m_defines.begin(), m_defines.end(), define);
            if (it != m_defines.end()) {
                m_defines.erase(it);
            }
        }

        bool ShaderManager::hasDefine(const std::string& define) const {
            return std::find(m_defines.begin(), m_defines.end(), define) != m_defines.end();
        }

        std::vector<std::string> ShaderManager::getDefines() const {
            return m_defines;
        }

        void ShaderManager::preloadDefaultShaders() {
            const std::string spriteVshader = R"(
                void main() {
                    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
                    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
                    gl_FrontColor = gl_Color;
                }
            )";

            const std::string spriteFshader = R"(
                uniform sampler2D texture;
                void main() {
                    gl_FragColor = gl_Color * texture2D(texture, gl_TexCoord[0].xy);
                }
            )";
            loadShaderFromMemory("sprite", spriteVshader, spriteFshader);

            const std::string textFshader = R"(
                uniform sampler2D texture;
                uniform vec4 color;
                void main() {
                    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
                    gl_FragColor = vec4(color.rgb, pixel.a * color.a);
                }
            )";
            loadShaderFromMemory("text", spriteVshader, textFshader);

            const std::string blurFshader = R"(
                uniform sampler2D texture;
                uniform vec2 blurRadius;
                uniform vec2 texelSize;
                void main() {
                    vec4 color = vec4(0.0);
                    float total = 0.0;
                    for (float x = -4.0; x <= 4.0; x += 1.0) {
                        for (float y = -4.0; y <= 4.0; y += 1.0) {
                            float weight = exp(-((x * x) / (2.0 * blurRadius.x * blurRadius.x) + (y * y) / (2.0 * blurRadius.y * blurRadius.y)));
                            vec2 offset = vec2(x * texelSize.x, y * texelSize.y);
                            color += texture2D(texture, gl_TexCoord[0].xy + offset) * weight;
                            total += weight;
                        }
                    }
                    gl_FragColor = color / total;
                }
            )";
            loadShaderFromMemory("blur", spriteVshader, blurFshader);

            const std::string glowFshader = R"(
                uniform sampler2D texture;
                uniform vec4 glowColor;
                uniform float intensity;
                void main() {
                    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
                    float alpha = pixel.a;
                    vec4 glow = glowColor * intensity * alpha;
                    gl_FragColor = pixel + glow * (1.0 - pixel.a);
                }
            )";
            loadShaderFromMemory("glow", spriteVshader, glowFshader);

            const std::string outlineFshader = R"(
                uniform sampler2D texture;
                uniform vec4 outlineColor;
                uniform vec2 texelSize;
                void main() {
                    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
                    if (pixel.a > 0.0) {
                        gl_FragColor = pixel;
                        return;
                    }
                    float alpha = 0.0;
                    for (float x = -1.0; x <= 1.0; x += 1.0) {
                        for (float y = -1.0; y <= 1.0; y += 1.0) {
                            if (x == 0.0 && y == 0.0) continue;
                            vec2 offset = vec2(x * texelSize.x, y * texelSize.y);
                            alpha += texture2D(texture, gl_TexCoord[0].xy + offset).a;
                        }
                    }
                    if (alpha > 0.0) {
                        gl_FragColor = outlineColor * min(alpha, 1.0);
                    } else {
                        gl_FragColor = vec4(0.0);
                    }
                }
            )";
            loadShaderFromMemory("outline", spriteVshader, outlineFshader);
        }

        void ShaderManager::enableHotReload(bool enable) {
            m_hotReloadEnabled = enable;
            for (auto& pair : m_shaders) {
                pair.second->enableHotReload(enable);
            }
        }

        bool ShaderManager::isHotReloadEnabled() const {
            return m_hotReloadEnabled;
        }

        void ShaderManager::updateHotReload() {
            if (!m_hotReloadEnabled) return;
            for (auto& pair : m_shaders) {
                pair.second->checkForChanges();
            }
        }

        std::vector<std::string> ShaderManager::getShaderList() const {
            std::vector<std::string> names;
            names.reserve(m_shaders.size());
            for (const auto& pair : m_shaders) {
                names.push_back(pair.first);
            }
            return names;
        }

        void ShaderManager::releaseAll() {
            m_shaders.clear();
        }

        std::string ShaderManager::injectDefines(const std::string& source) const {
            if (m_defines.empty()) return source;

            std::stringstream ss;
            for (const auto& define : m_defines) {
                ss << "#define " << define << "\n";
            }
            ss << source;
            return ss.str();
        }

    }
}
