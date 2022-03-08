#pragma once

#include "Shader.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

namespace nebula {
    namespace graphics {

        class ShaderManager {
        public:
            static ShaderManager& getInstance();

            std::shared_ptr<Shader> loadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);
            std::shared_ptr<Shader> loadShaderFromMemory(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource);
            std::shared_ptr<Shader> getShader(const std::string& name) const;
            bool unloadShader(const std::string& name);
            bool hasShader(const std::string& name) const;

            bool reloadAll();
            bool reloadShader(const std::string& name);

            void addDefine(const std::string& define);
            void removeDefine(const std::string& define);
            bool hasDefine(const std::string& define) const;
            std::vector<std::string> getDefines() const;

            void preloadDefaultShaders();

            void enableHotReload(bool enable);
            bool isHotReloadEnabled() const;
            void updateHotReload();

            std::vector<std::string> getShaderList() const;

            void releaseAll();

        private:
            ShaderManager();
            ~ShaderManager();
            ShaderManager(const ShaderManager&) = delete;
            ShaderManager& operator=(const ShaderManager&) = delete;

            std::string injectDefines(const std::string& source) const;

            std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
            std::vector<std::string> m_defines;
            bool m_hotReloadEnabled;
        };

    }
}
