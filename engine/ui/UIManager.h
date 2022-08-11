#pragma once

#include "UIWidget.h"
#include "UIStyle.h"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <memory>
#include <vector>
#include <unordered_map>

namespace nebula {
    namespace ui {

        class UIManager {
        public:
            static UIManager& getInstance();

            void addWidget(std::shared_ptr<UIWidget> widget);
            void removeWidget(std::shared_ptr<UIWidget> widget);
            std::shared_ptr<UIWidget> getWidget(const std::string& name);
            std::shared_ptr<UIWidget> findWidget(const sf::Vector2f& point);

            void setFocusedWidget(std::shared_ptr<UIWidget> widget);
            std::shared_ptr<UIWidget> getFocusedWidget() const;
            void clearFocus();

            void processEvent(const sf::Event& event);
            void update(float dt);
            void render(sf::RenderTarget& target);

            void setGlobalStyle(const UIStyle& style);
            UIStyle getGlobalStyle() const;

            void setDebugMode(bool debug);
            bool getDebugMode() const;

        private:
            UIManager();
            ~UIManager() = default;
            UIManager(const UIManager&) = delete;
            UIManager& operator=(const UIManager&) = delete;

            std::shared_ptr<UIWidget> m_root;
            std::shared_ptr<UIWidget> m_focusedWidget;
            std::unordered_map<std::string, std::shared_ptr<UIWidget>> m_widgetMap;
            UIStyle m_globalStyle;

            bool m_debugMode;
            sf::RectangleShape m_debugRect;

            void buildWidgetMap(std::shared_ptr<UIWidget> widget);
            void renderWidgetBounds(sf::RenderTarget& target, std::shared_ptr<UIWidget> widget);
            void handleNavigation(const sf::Event& event);
        };

    }
}
