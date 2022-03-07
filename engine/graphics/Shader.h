#pragma once

#include <SFML/Graphics/Shader.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <chrono>

namespace nebula {
    namespace graphics {

        struct ShaderCompileResult {
            bool success;
            std::string errorLog;
            std::string vertexSource;
            std::string fragmentSource;
        };

        class Shader {
        public:
            Shader();
            explicit Shader(const std::string& name);
            ~Shader();

            bool loadFromFile(const std::string& vertexPath, const std::string& fragmentPath);
            bool loadFromMemory(const std::string& vertexSource, const std::string& fragmentSource);
            bool loadFromSource(const std::string& vertexSource, const std::string& fragmentSource);

            void bind() const;
            static void unbind();

            void setUniform(const std::string& name, float value);
            void setUniform(const std::string& name, const sf::Vector2f& value);
            void setUniform(const std::string& name, const sf::Vector3f& value);
            void setUniform(const std::string& name, const sf::Glsl::Vec4& value);
            void setUniform(const std::string& name, const sf::Glsl::Mat3& value);
            void setUniform(const std::string& name, const sf::Glsl::Mat4& value);
            void setUniform(const std::string& name, int value);
            void setUniform(const std::string& name, const sf::Texture& texture);
            void setUniform(const std::string& name, const sf::Shader::CurrentTextureType& texture);

            void setUniformArray(const std::string& name, const float* values, std::size_t count);
            void setUniformArray(const std::string& name, const sf::Vector2f* values, std::size_t count);
            void setUniformArray(const std::string& name, const int* values, std::size_t count);

            void setUniformBlock(const std::string& name, unsigned int blockIndex);

            const std::string& getName() const;
            bool isValid() const;

            std::string getVertexPath() const;
            std::string getFragmentPath() const;

            ShaderCompileResult getLastCompileResult() const;

            void enableHotReload(bool enable);
            bool isHotReloadEnabled() const;
            bool checkForChanges();

            using ReloadCallback = std::function<void(const std::string& shaderName, bool success)>;
            void setReloadCallback(ReloadCallback callback);

            static bool isAvailable();
            static unsigned int getActiveShaderCount();

        private:
            std::string m_name;
            std::unique_ptr<sf::Shader> m_shader;

            std::string m_vertexPath;
            std::string m_fragmentPath;
            std::string m_vertexSource;
            std::string m_fragmentSource;

            ShaderCompileResult m_lastResult;

            bool m_hotReloadEnabled;
            std::chrono::system_clock::time_point m_lastFileCheck;
            ReloadCallback m_reloadCallback;

            std::unordered_map<std::string, sf::Glsl::Vec4> m_cachedUniforms;

            bool compileInternal();
            void storeSource(const std::string& vertexSource, const std::string& fragmentSource);

            static unsigned int s_activeShaderCount;
        };

    }
}
