#include "UIManager.h"
#include <algorithm>

namespace nebula {
    namespace ui {

        UIManager::UIManager()
            : m_debugMode(false) {
            m_root = std::make_shared<UIWidget>("__root__");
            m_root->setSize(1920.f, 1080.f);
            m_globalStyle = UIStyle::Dark();

            m_debugRect.setFillColor(sf::Color::Transparent);
            m_debugRect.setOutlineThickness(1.f);
            m_debugRect.setOutlineColor(sf::Color(0, 255, 0, 128));
        }

        UIManager& UIManager::getInstance() {
            static UIManager instance;
            return instance;
        }

        void UIManager::addWidget(std::shared_ptr<UIWidget> widget) {
            if (widget) {
                m_root->addChild(widget);
                buildWidgetMap(m_root);
            }
        }

        void UIManager::removeWidget(std::shared_ptr<UIWidget> widget) {
            if (widget) {
                m_root->removeChild(widget);
                for (auto it = m_widgetMap.begin(); it != m_widgetMap.end(); ) {
                    if (it->second == widget) {
                        it = m_widgetMap.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
        }

        std::shared_ptr<UIWidget> UIManager::getWidget(const std::string& name) {
            auto it = m_widgetMap.find(name);
            if (it != m_widgetMap.end()) {
                return it->second;
            }
            return nullptr;
        }

        std::shared_ptr<UIWidget> UIManager::findWidget(const sf::Vector2f& point) {
            std::vector<std::shared_ptr<UIWidget>> stack = {m_root};
            std::shared_ptr<UIWidget> result = nullptr;

            while (!stack.empty()) {
                auto current = stack.back();
                stack.pop_back();

                if (current->visible && current->containsPoint(point)) {
                    result = current;
                    for (auto it = current->m_children.rbegin(); it != current->m_children.rend(); ++it) {
                        stack.push_back(*it);
                    }
                }
            }
            return result;
        }

        void UIManager::setFocusedWidget(std::shared_ptr<UIWidget> widget) {
            if (m_focusedWidget && m_focusedWidget != widget) {
                m_focusedWidget->m_focused = false;
            }
            m_focusedWidget = widget;
            if (m_focusedWidget) {
                m_focusedWidget->m_focused = true;
                m_focusedWidget->bringToFront();
            }
        }

        std::shared_ptr<UIWidget> UIManager::getFocusedWidget() const {
            return m_focusedWidget;
        }

        void UIManager::clearFocus() {
            if (m_focusedWidget) {
                m_focusedWidget->m_focused = false;
                m_focusedWidget.reset();
            }
        }

        void UIManager::processEvent(const sf::Event& event) {
            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2f mousePos(
                    static_cast<float>(event.mouseButton.x),
                    static_cast<float>(event.mouseButton.y)
                );
                auto clicked = findWidget(mousePos);
                if (clicked && clicked->focusable) {
                    setFocusedWidget(clicked);
                } else if (!clicked) {
                    clearFocus();
                }
            }

            if (event.type == sf::Event::KeyPressed &&
                (event.key.code == sf::Keyboard::Tab || event.key.code == sf::Keyboard::Up ||
                 event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::Left ||
                 event.key.code == sf::Keyboard::Right)) {
                handleNavigation(event);
            }

            m_root->onEvent(event);
        }

        void UIManager::update(float dt) {
            m_root->onUpdate(dt);
            m_root->onLayout();
        }

        void UIManager::render(sf::RenderTarget& target) {
            m_root->onRender(target, sf::RenderStates::Default);

            if (m_debugMode) {
                renderWidgetBounds(target, m_root);
            }
        }

        void UIManager::setGlobalStyle(const UIStyle& style) {
            m_globalStyle = style;
        }

        UIStyle UIManager::getGlobalStyle() const {
            return m_globalStyle;
        }

        void UIManager::setDebugMode(bool debug) {
            m_debugMode = debug;
        }

        bool UIManager::getDebugMode() const {
            return m_debugMode;
        }

        void UIManager::buildWidgetMap(std::shared_ptr<UIWidget> widget) {
            if (!widget) return;
            if (!widget->name.empty()) {
                m_widgetMap[widget->name] = widget;
            }
            if (!widget->id.empty()) {
                m_widgetMap[widget->id] = widget;
            }
            for (auto& child : widget->m_children) {
                buildWidgetMap(child);
            }
        }

        void UIManager::renderWidgetBounds(sf::RenderTarget& target, std::shared_ptr<UIWidget> widget) {
            if (!widget->visible) return;

            sf::FloatRect bounds = widget->getGlobalBounds();
            m_debugRect.setPosition(bounds.left, bounds.top);
            m_debugRect.setSize(sf::Vector2f(bounds.width, bounds.height));
            target.draw(m_debugRect);

            for (auto& child : widget->m_children) {
                renderWidgetBounds(target, child);
            }
        }

        void UIManager::handleNavigation(const sf::Event& event) {
            std::vector<std::shared_ptr<UIWidget>> focusableWidgets;
            auto collectFocusable = [&](std::shared_ptr<UIWidget> w, auto& self_ref) -> void {
                if (w->focusable && w->visible && w->enabled) {
                    focusableWidgets.push_back(w);
                }
                for (auto& child : w->m_children) {
                    self_ref(child, self_ref);
                }
            };
            collectFocusable(m_root, collectFocusable);

            if (focusableWidgets.empty()) return;

            int currentIndex = -1;
            if (m_focusedWidget) {
                auto it = std::find(focusableWidgets.begin(), focusableWidgets.end(), m_focusedWidget);
                if (it != focusableWidgets.end()) {
                    currentIndex = static_cast<int>(it - focusableWidgets.begin());
                }
            }

            int nextIndex = currentIndex;
            if (event.key.code == sf::Keyboard::Tab || event.key.code == sf::Keyboard::Down) {
                nextIndex = (currentIndex + 1) % static_cast<int>(focusableWidgets.size());
            } else if (event.key.code == sf::Keyboard::Up) {
                nextIndex = (currentIndex - 1 + static_cast<int>(focusableWidgets.size())) % static_cast<int>(focusableWidgets.size());
            }

            if (nextIndex >= 0 && nextIndex < static_cast<int>(focusableWidgets.size())) {
                setFocusedWidget(focusableWidgets[nextIndex]);
            }
        }

    }
}
