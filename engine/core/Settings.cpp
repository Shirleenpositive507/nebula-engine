#include "Settings.h"
#include "Logger.h"
#include "Config.h"
#include <algorithm>

namespace nebula {

    Settings& Settings::instance() {
        static Settings inst;
        return inst;
    }

    void Settings::loadDefaults() {
        m_graphicsQuality = 2;
        m_shadowQuality = 1;
        m_textureQuality = 2;
        m_masterVolume = 1.0f;
        m_musicVolume = 0.8f;
        m_sfxVolume = 1.0f;
        m_width = 1280;
        m_height = 720;
        m_fullscreen = false;
        m_vsync = true;
        m_antialiasing = 0;
        m_language = "en";
        m_showFPS = true;
        m_brightness = 1.0f;
        m_gamma = 1.0f;

        NEBULA_INFO("Settings loaded defaults");
    }

    void Settings::loadFromFile(const std::string& path) {
        loadDefaults();

        Config& config = Config::instance();
        if (!config.load(path)) {
            NEBULA_WARN("Could not load settings from file, using defaults");
            return;
        }

        m_graphicsQuality = config.getInt("video.graphics_quality", m_graphicsQuality);
        m_shadowQuality = config.getInt("video.shadow_quality", m_shadowQuality);
        m_textureQuality = config.getInt("video.texture_quality", m_textureQuality);
        m_width = config.getInt("video.width", m_width);
        m_height = config.getInt("video.height", m_height);
        m_fullscreen = config.getBool("video.fullscreen", m_fullscreen);
        m_vsync = config.getBool("video.vsync", m_vsync);
        m_antialiasing = config.getInt("video.antialiasing", m_antialiasing);
        m_brightness = config.getFloat("video.brightness", m_brightness);
        m_gamma = config.getFloat("video.gamma", m_gamma);

        m_masterVolume = config.getFloat("audio.master_volume", m_masterVolume);
        m_musicVolume = config.getFloat("audio.music_volume", m_musicVolume);
        m_sfxVolume = config.getFloat("audio.sfx_volume", m_sfxVolume);

        m_language = config.getString("general.language", m_language);
        m_showFPS = config.getBool("general.show_fps", m_showFPS);

        m_graphicsQuality = std::clamp(m_graphicsQuality, 0, 3);
        m_masterVolume = std::clamp(m_masterVolume, 0.0f, 1.0f);
        m_musicVolume = std::clamp(m_musicVolume, 0.0f, 1.0f);
        m_sfxVolume = std::clamp(m_sfxVolume, 0.0f, 1.0f);
        m_brightness = std::clamp(m_brightness, 0.1f, 2.0f);
        m_gamma = std::clamp(m_gamma, 0.1f, 3.0f);
        m_antialiasing = std::clamp(m_antialiasing, 0, 16);

        NEBULA_INFO("Settings loaded from: " + path);
    }

    void Settings::saveToFile(const std::string& path) {
        Config& config = Config::instance();

        config.set("video.graphics_quality", m_graphicsQuality);
        config.set("video.shadow_quality", m_shadowQuality);
        config.set("video.texture_quality", m_textureQuality);
        config.set("video.width", m_width);
        config.set("video.height", m_height);
        config.set("video.fullscreen", m_fullscreen);
        config.set("video.vsync", m_vsync);
        config.set("video.antialiasing", m_antialiasing);
        config.set("video.brightness", m_brightness);
        config.set("video.gamma", m_gamma);

        config.set("audio.master_volume", m_masterVolume);
        config.set("audio.music_volume", m_musicVolume);
        config.set("audio.sfx_volume", m_sfxVolume);

        config.set("general.language", m_language);
        config.set("general.show_fps", m_showFPS);

        if (config.save(path)) {
            NEBULA_INFO("Settings saved to: " + path);
        }
    }

    void Settings::setGraphicsQuality(int quality) {
        m_graphicsQuality = std::clamp(quality, 0, 3);
    }

    int Settings::getGraphicsQuality() const {
        return m_graphicsQuality;
    }

    void Settings::setMasterVolume(float volume) {
        m_masterVolume = std::clamp(volume, 0.0f, 1.0f);
    }

    float Settings::getMasterVolume() const {
        return m_masterVolume;
    }

    void Settings::setMusicVolume(float volume) {
        m_musicVolume = std::clamp(volume, 0.0f, 1.0f);
    }

    float Settings::getMusicVolume() const {
        return m_musicVolume;
    }

    void Settings::setSFXVolume(float volume) {
        m_sfxVolume = std::clamp(volume, 0.0f, 1.0f);
    }

    float Settings::getSFXVolume() const {
        return m_sfxVolume;
    }

    void Settings::setResolution(int width, int height) {
        if (width > 0 && height > 0) {
            m_width = width;
            m_height = height;
        }
    }

    void Settings::getResolution(int& width, int& height) const {
        width = m_width;
        height = m_height;
    }

    void Settings::setFullscreen(bool fs) {
        m_fullscreen = fs;
    }

    bool Settings::getFullscreen() const {
        return m_fullscreen;
    }

    void Settings::setVSync(bool vsync) {
        m_vsync = vsync;
    }

    bool Settings::getVSync() const {
        return m_vsync;
    }

    void Settings::setAntialiasingLevel(int level) {
        m_antialiasing = std::clamp(level, 0, 16);
    }

    int Settings::getAntialiasingLevel() const {
        return m_antialiasing;
    }

    void Settings::setShadowQuality(int quality) {
        m_shadowQuality = std::clamp(quality, 0, 3);
    }

    int Settings::getShadowQuality() const {
        return m_shadowQuality;
    }

    void Settings::setTextureQuality(int quality) {
        m_textureQuality = std::clamp(quality, 0, 3);
    }

    int Settings::getTextureQuality() const {
        return m_textureQuality;
    }

    void Settings::setLanguage(const std::string& lang) {
        m_language = lang;
    }

    std::string Settings::getLanguage() const {
        return m_language;
    }

    void Settings::setShowFPS(bool show) {
        m_showFPS = show;
    }

    bool Settings::getShowFPS() const {
        return m_showFPS;
    }

    void Settings::setBrightness(float brightness) {
        m_brightness = std::clamp(brightness, 0.1f, 2.0f);
    }

    float Settings::getBrightness() const {
        return m_brightness;
    }

    void Settings::setGamma(float gamma) {
        m_gamma = std::clamp(gamma, 0.1f, 3.0f);
    }

    float Settings::getGamma() const {
        return m_gamma;
    }

    void Settings::resetToDefaults() {
        loadDefaults();
        NEBULA_INFO("Settings reset to defaults");
    }

}
