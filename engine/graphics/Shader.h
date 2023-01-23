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
#include <cstdint>

namespace nebula {
    namespace graphics {

        enum class ShaderType {
            Vertex,
            Fragment,
            Geometry,
            TessellationControl,
            TessellationEvaluation,
            Compute
        };

        enum class ShaderStage {
            None = 0,
            Vertex = 1 << 0,
            Fragment = 1 << 1,
            Geometry = 1 << 2,
            TessellationControl = 1 << 3,
            TessellationEvaluation = 1 << 4,
            Compute = 1 << 5
        };

        struct ShaderUniformBlock {
            std::string name;
            unsigned int binding;
            std::size_t size;
            std::vector<unsigned char> data;

            ShaderUniformBlock() : binding(0), size(0) {}
        };

        struct ShaderStorageBlock {
            std::string name;
            unsigned int binding;
            std::size_t size;

            ShaderStorageBlock() : binding(0), size(0) {}
        };

        struct ShaderCompileResult {
            bool success;
            std::string errorLog;
            std::string vertexSource;
            std::string fragmentSource;
            std::string geometrySource;
            std::string tessControlSource;
            std::string tessEvalSource;
            std::string computeSource;
        };

        class Shader {
        public:
            Shader();
            explicit Shader(const std::string& name);
            ~Shader();

            bool loadFromFile(const std::string& vertexPath, const std::string& fragmentPath);
            bool loadFromFile(const std::string& vertexPath, const std::string& geometryPath, const std::string& fragmentPath);
            bool loadFromFile(const std::string& vertexPath, const std::string& tessControlPath,
                              const std::string& tessEvalPath, const std::string& geometryPath, const std::string& fragmentPath);
            bool loadFromFile(const std::string& computePath);

            bool loadFromMemory(const std::string& vertexSource, const std::string& fragmentSource);
            bool loadFromMemory(const std::string& vertexSource, const std::string& geometrySource, const std::string& fragmentSource);
            bool loadFromMemory(const std::string& vertexSource, const std::string& tessControlSource,
                                const std::string& tessEvalSource, const std::string& geometrySource, const std::string& fragmentSource);
            bool loadFromMemory(const std::string& computeSource);

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
            void setUniformBlockBinding(const std::string& name, unsigned int binding);
            bool hasUniformBlock(const std::string& name) const;

            void setShaderStorageBlock(const std::string& name, unsigned int binding);
            bool hasShaderStorageBlock(const std::string& name) const;

            void dispatchCompute(unsigned int groupX, unsigned int groupY, unsigned int groupZ);
            void dispatchComputeIndirect(unsigned int indirectBuffer);

            ShaderStage getActiveStages() const;
            bool hasStage(ShaderStage stage) const;

            const std::string& getGeometrySource() const;
            const std::string& getTessControlSource() const;
            const std::string& getTessEvalSource() const;
            const std::string& getComputeSource() const;

            bool isComputeShader() const;

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
            std::string m_geometryPath;
            std::string m_tessControlPath;
            std::string m_tessEvalPath;
            std::string m_fragmentPath;
            std::string m_computePath;
            std::string m_vertexSource;
            std::string m_geometrySource;
            std::string m_tessControlSource;
            std::string m_tessEvalSource;
            std::string m_fragmentSource;
            std::string m_computeSource;

            ShaderStage m_activeStages;
            ShaderCompileResult m_lastResult;

            bool m_hotReloadEnabled;
            std::chrono::system_clock::time_point m_lastFileCheck;
            ReloadCallback m_reloadCallback;

            std::unordered_map<std::string, sf::Glsl::Vec4> m_cachedUniforms;
            std::unordered_map<std::string, ShaderUniformBlock> m_uniformBlocks;
            std::unordered_map<std::string, ShaderStorageBlock> m_storageBlocks;

            bool compileInternal();
            void storeSource(const std::string& vertexSource, const std::string& fragmentSource);

            static unsigned int s_activeShaderCount;
        };

    }
}
