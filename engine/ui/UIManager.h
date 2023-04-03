#pragma once

#include "UIWidget.h"
#include "UIStyle.h"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/View.hpp>
#include <memory>
#include <vector>
#include <stack>
#include <unordered_map>
#include <functional>

namespace nebula {
    namespace ui {

        struct InputRemap {
            sf::Keyboard::Key tabNext;
            sf::Keyboard::Key tabPrev;
            sf::Keyboard::Key up;
            sf::Keyboard::Key down;
            sf::Keyboard::Key left;
            sf::Keyboard::Key right;
            sf::Keyboard::Key confirm;
            sf::Keyboard::Key cancel;

            InputRemap()
                : tabNext(sf::Keyboard::Tab)
                , tabPrev(sf::Keyboard::LShift)
                , up(sf::Keyboard::Up)
                , down(sf::Keyboard::Down)
                , left(sf::Keyboard::Left)
                , right(sf::Keyboard::Right)
                , confirm(sf::Keyboard::Enter)
                , cancel(sf::Keyboard::Escape) {}
        };

        struct ScreenReaderAnnouncement {
            std::string text;
            float duration;
            float elapsed;

            ScreenReaderAnnouncement() : duration(3.f), elapsed(0.f) {}
            explicit ScreenReaderAnnouncement(const std::string& t, float d = 3.f)
                : text(t), duration(d), elapsed(0.f) {}
        };

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

            void pushModal(std::shared_ptr<UIWidget> modal);
            void popModal();
            std::shared_ptr<UIWidget> getTopModal() const;
            bool isModalActive() const;

            void setUICamera(const sf::View& view);
            sf::View getUICamera() const;
            void setScreenSpace(bool screenSpace);
            bool isScreenSpace() const;

            void setDPIScale(float scale);
            float getDPIScale() const;

            void setInputRemapping(const InputRemap& remap);
            const InputRemap& getInputRemapping() const;

            void setTabOrder(const std::vector<std::string>& order);
            void focusNext(bool forward = true);
            void focusPrevious();
            void navigateDirection(sf::Keyboard::Key key);

            void announce(const std::string& text, float duration = 3.f);
            std::string getCurrentAnnouncement() const;

            void enableScreenReader(bool enabled);
            bool isScreenReaderEnabled() const;

            void setScreenReaderDescribeWidget(std::function<std::string(std::shared_ptr<UIWidget>)> describeFunc);

            Callback onModalPushed;
            Callback onModalPopped;
            Callback onFocusChanged;

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

            std::stack<std::shared_ptr<UIWidget>> m_modalStack;

            sf::View m_uiCamera;
            bool m_screenSpace;
            float m_dpiScale;

            InputRemap m_inputRemap;
            std::vector<std::string> m_tabOrder;

            bool m_screenReaderEnabled;
            std::function<std::string(std::shared_ptr<UIWidget>)> m_describeWidget;
            ScreenReaderAnnouncement m_currentAnnouncement;

            void buildWidgetMap(std::shared_ptr<UIWidget> widget);
            void renderWidgetBounds(sf::RenderTarget& target, std::shared_ptr<UIWidget> widget);
            void handleNavigation(const sf::Event& event);
            void handleModalInput(const sf::Event& event);
            void renderModalOverlay(sf::RenderTarget& target);
            std::vector<std::shared_ptr<UIWidget>> collectFocusableWidgets() const;
            std::string defaultDescribeWidget(std::shared_ptr<UIWidget> widget) const;
        };

    }
}
