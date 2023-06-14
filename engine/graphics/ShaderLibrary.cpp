#include "ShaderLibrary.h"

namespace nebula {
    std::unordered_map<std::string, std::shared_ptr<Shader>> ShaderLibrary::s_shaders;

    void ShaderLibrary::init() {
        auto defaultVert = Shader::loadVertex("resources/shaders/default_sprite.vert");
        auto defaultFrag = Shader::loadFragment("resources/shaders/default_sprite.frag");
        s_shaders["default_sprite"] = std::make_shared<Shader>(defaultVert, defaultFrag);

        auto blurFrag = Shader::loadFragment("resources/shaders/gaussian_blur.frag");
        s_shaders["gaussian_blur"] = std::make_shared<Shader>(defaultVert, blurFrag);

        auto outlineFrag = Shader::loadFragment("resources/shaders/outline.frag");
        s_shaders["outline"] = std::make_shared<Shader>(defaultVert, outlineFrag);

        auto vignetteFrag = Shader::loadFragment("resources/shaders/vignette.frag");
        s_shaders["vignette"] = std::make_shared<Shader>(defaultVert, vignetteFrag);

        auto chromaFrag = Shader::loadFragment("resources/shaders/chromatic_aberration.frag");
        s_shaders["chromatic_aberration"] = std::make_shared<Shader>(defaultVert, chromaFrag);

        auto bloomFrag = Shader::loadFragment("resources/shaders/bloom_blur.frag");
        s_shaders["bloom"] = std::make_shared<Shader>(defaultVert, bloomFrag);

        auto hdrFrag = Shader::loadFragment("resources/shaders/hdr_tone.frag");
        s_shaders["hdr_tone"] = std::make_shared<Shader>(defaultVert, hdrFrag);

        auto flashFrag = Shader::loadFragment("resources/shaders/flash.frag");
        s_shaders["flash"] = std::make_shared<Shader>(defaultVert, flashFrag);

        auto grayFrag = Shader::loadFragment("resources/shaders/grayscale.frag");
        s_shaders["grayscale"] = std::make_shared<Shader>(defaultVert, grayFrag);
    }

    void ShaderLibrary::shutdown() {
        s_shaders.clear();
    }

    std::shared_ptr<Shader> ShaderLibrary::getDefaultSprite() {
        return s_shaders["default_sprite"];
    }

    std::shared_ptr<Shader> ShaderLibrary::getBlur() {
        return s_shaders["gaussian_blur"];
    }

    std::shared_ptr<Shader> ShaderLibrary::getOutline() {
        return s_shaders["outline"];
    }

    std::shared_ptr<Shader> ShaderLibrary::getVignette() {
        return s_shaders["vignette"];
    }

    std::shared_ptr<Shader> ShaderLibrary::getChromaticAberration() {
        return s_shaders["chromatic_aberration"];
    }

    std::shared_ptr<Shader> ShaderLibrary::getBloom() {
        return s_shaders["bloom"];
    }

    std::shared_ptr<Shader> ShaderLibrary::getHDRTone() {
        return s_shaders["hdr_tone"];
    }

    std::shared_ptr<Shader> ShaderLibrary::getFlash() {
        return s_shaders["flash"];
    }

    std::shared_ptr<Shader> ShaderLibrary::getGrayscale() {
        return s_shaders["grayscale"];
    }

    std::shared_ptr<Shader> ShaderLibrary::getCustom(const std::string& name) {
        auto it = s_shaders.find(name);
        if (it != s_shaders.end()) return it->second;
        return nullptr;
    }

    void ShaderLibrary::registerCustom(const std::string& name, std::shared_ptr<Shader> shader) {
        s_shaders[name] = shader;
    }
}
