#pragma once
#include "Shader.h"
#include <memory>
#include <unordered_map>

namespace nebula {
    class ShaderLibrary {
    public:
        static void init();
        static void shutdown();
        static std::shared_ptr<Shader> getDefaultSprite();
        static std::shared_ptr<Shader> getBlur();
        static std::shared_ptr<Shader> getOutline();
        static std::shared_ptr<Shader> getVignette();
        static std::shared_ptr<Shader> getChromaticAberration();
        static std::shared_ptr<Shader> getBloom();
        static std::shared_ptr<Shader> getHDRTone();
        static std::shared_ptr<Shader> getFlash();
        static std::shared_ptr<Shader> getGrayscale();
        static std::shared_ptr<Shader> getCustom(const std::string& name);
        static void registerCustom(const std::string& name, std::shared_ptr<Shader> shader);
    private:
        static std::unordered_map<std::string, std::shared_ptr<Shader>> s_shaders;
    };
}
