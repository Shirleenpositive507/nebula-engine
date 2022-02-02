#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <sstream>

namespace nebula {

    class Settings {
    public:
        static Settings& instance();

        void loadDefaults();
        void loadFromFile(const std::string& path);
        void saveToFile(const std::string& path);

        void setGraphicsQuality(int quality);
        int getGraphicsQuality() const;

        void setMasterVolume(float volume);
        float getMasterVolume() const;

        void setMusicVolume(float volume);
        float getMusicVolume() const;

        void setSFXVolume(float volume);
        float getSFXVolume() const;

        void setResolution(int width, int height);
        void getResolution(int& width, int& height) const;
        int getWidth() const { return m_width; }
        int getHeight() const { return m_height; }

        void setFullscreen(bool fs);
        bool getFullscreen() const;

        void setVSync(bool vsync);
        bool getVSync() const;

        void setAntialiasingLevel(int level);
        int getAntialiasingLevel() const;

        void setShadowQuality(int quality);
        int getShadowQuality() const;

        void setTextureQuality(int quality);
        int getTextureQuality() const;

        void setLanguage(const std::string& lang);
        std::string getLanguage() const;

        void setShowFPS(bool show);
        bool getShowFPS() const;

        void setBrightness(float brightness);
        float getBrightness() const;

        void setGamma(float gamma);
        float getGamma() const;

        void resetToDefaults();

    private:
        Settings() = default;
        Settings(const Settings&) = delete;
        Settings& operator=(const Settings&) = delete;

        int m_graphicsQuality = 2;
        int m_shadowQuality = 1;
        int m_textureQuality = 2;
        float m_masterVolume = 1.0f;
        float m_musicVolume = 0.8f;
        float m_sfxVolume = 1.0f;
        int m_width = 1280;
        int m_height = 720;
        bool m_fullscreen = false;
        bool m_vsync = true;
        int m_antialiasing = 0;
        std::string m_language = "en";
        bool m_showFPS = true;
        float m_brightness = 1.0f;
        float m_gamma = 1.0f;
    };

}
