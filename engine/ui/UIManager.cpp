#include "UIManager.h"
#include <algorithm>

namespace nebula {
    namespace ui {

        UIManager::UIManager()
            : m_debugMode(false)
            , m_screenSpace(true)
            , m_dpiScale(1.f)
            , m_screenReaderEnabled(false)
            , m_tooltipDelay(0.5f) {
            m_root = std::make_shared<UIWidget>("__root__");
            m_root->setSize(1920.f, 1080.f);
            m_globalStyle = UIStyle::Dark();

            m_debugRect.setFillColor(sf::Color::Transparent);
            m_debugRect.setOutlineThickness(1.f);
            m_debugRect.setOutlineColor(sf::Color(0, 255, 0, 128));

            m_uiCamera = sf::View(sf::FloatRect(0.f, 0.f, 1920.f, 1080.f));

            m_describeWidget = [this](std::shared_ptr<UIWidget> w) {
                return defaultDescribeWidget(w);
            };
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
            if (isModalActive()) {
                auto topModal = getTopModal();
                if (topModal && topModal->visible && topModal->containsPoint(point)) {
                    std::vector<std::shared_ptr<UIWidget>> stack = {topModal};
                    while (!stack.empty()) {
                        auto current = stack.back();
                        stack.pop_back();
                        if (current->visible && current->containsPoint(point)) {
                            for (auto it = current->m_children.rbegin(); it != current->m_children.rend(); ++it) {
                                stack.push_back(*it);
                            }
                            return current;
                        }
                    }
                }
                return nullptr;
            }

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
                if (m_screenReaderEnabled) {
                    announce(m_describeWidget(m_focusedWidget));
                }
                if (onFocusChanged) onFocusChanged();
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
            if (isModalActive()) {
                handleModalInput(event);
                return;
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2f mousePos(
                    static_cast<float>(event.mouseButton.x) / m_dpiScale,
                    static_cast<float>(event.mouseButton.y) / m_dpiScale
                );
                auto clicked = findWidget(mousePos);
                if (auto dropdown = std::dynamic_pointer_cast<UIDropdown>(clicked)) {
                    setFocusedWidget(clicked);
                    return;
                }
                if (auto scrollbar = std::dynamic_pointer_cast<UIScrollbar>(clicked)) {
                    setFocusedWidget(clicked);
                    return;
                }
            }

            if (event.type == sf::Event::MouseButtonPressed || event.type == sf::Event::MouseMoved) {
                sf::Vector2f mousePos(
                    static_cast<float>(event.mouseButton.x) / m_dpiScale,
                    static_cast<float>(event.mouseButton.y) / m_dpiScale
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

            if (m_screenReaderEnabled && m_currentAnnouncement.elapsed < m_currentAnnouncement.duration) {
                m_currentAnnouncement.elapsed += dt;
            }

            if (isModalActive()) {
                auto topModal = getTopModal();
                if (topModal) {
                    topModal->onUpdate(dt);
                    topModal->onLayout();
                }
            }
        }

        void UIManager::render(sf::RenderTarget& target) {
            target.setView(m_uiCamera);

            sf::RenderStates states = sf::RenderStates::Default;
            states.transform.scale(m_dpiScale, m_dpiScale);

            m_root->onRender(target, states);

            if (isModalActive()) {
                renderModalOverlay(target);
                auto topModal = getTopModal();
                if (topModal) {
                    topModal->onRender(target, states);
                }
            }

            if (m_debugMode) {
                renderWidgetBounds(target, m_root);
            }

            target.setView(target.getDefaultView());
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
            if (event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::Right) {
                navigateDirection(event.key.code);
                return;
            }

            auto focusableWidgets = collectFocusableWidgets();
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

        void UIManager::pushModal(std::shared_ptr<UIWidget> modal) {
            if (modal) {
                m_modalStack.push(modal);
                modal->show();
                setFocusedWidget(modal);
                if (onModalPushed) onModalPushed();
            }
        }

        void UIManager::popModal() {
            if (!m_modalStack.empty()) {
                auto top = m_modalStack.top();
                top->hide();
                m_modalStack.pop();
                clearFocus();
                if (!m_modalStack.empty()) {
                    setFocusedWidget(m_modalStack.top());
                }
                if (onModalPopped) onModalPopped();
            }
        }

        std::shared_ptr<UIWidget> UIManager::getTopModal() const {
            if (m_modalStack.empty()) return nullptr;
            return m_modalStack.top();
        }

        bool UIManager::isModalActive() const {
            return !m_modalStack.empty();
        }

        void UIManager::setUICamera(const sf::View& view) {
            m_uiCamera = view;
        }

        sf::View UIManager::getUICamera() const {
            return m_uiCamera;
        }

        void UIManager::setScreenSpace(bool screenSpace) {
            m_screenSpace = screenSpace;
        }

        bool UIManager::isScreenSpace() const {
            return m_screenSpace;
        }

        void UIManager::setDPIScale(float scale) {
            m_dpiScale = std::max(0.25f, scale);
        }

        float UIManager::getDPIScale() const {
            return m_dpiScale;
        }

        void UIManager::setTooltipDelay(float delay) {
            m_tooltipDelay = std::max(0.0f, delay);
        }

        float UIManager::getTooltipDelay() const {
            return m_tooltipDelay;
        }

        void UIManager::setInputRemapping(const InputRemap& remap) {
            m_inputRemap = remap;
        }

        const InputRemap& UIManager::getInputRemapping() const {
            return m_inputRemap;
        }

        void UIManager::setTabOrder(const std::vector<std::string>& order) {
            m_tabOrder = order;
        }

        void UIManager::focusNext(bool forward) {
            auto focusableWidgets = collectFocusableWidgets();
            if (focusableWidgets.empty()) return;

            int currentIndex = -1;
            if (m_focusedWidget) {
                auto it = std::find(focusableWidgets.begin(), focusableWidgets.end(), m_focusedWidget);
                if (it != focusableWidgets.end()) {
                    currentIndex = static_cast<int>(it - focusableWidgets.begin());
                }
            }

            int size = static_cast<int>(focusableWidgets.size());
            int nextIndex = forward ? (currentIndex + 1) % size : (currentIndex - 1 + size) % size;
            setFocusedWidget(focusableWidgets[nextIndex]);
        }

        void UIManager::focusPrevious() {
            focusNext(false);
        }

        void UIManager::navigateDirection(sf::Keyboard::Key key) {
            auto focusableWidgets = collectFocusableWidgets();
            if (focusableWidgets.empty() || !m_focusedWidget) return;

            sf::FloatRect currentBounds = m_focusedWidget->getGlobalBounds();
            sf::Vector2f currentCenter(
                currentBounds.left + currentBounds.width / 2.f,
                currentBounds.top + currentBounds.height / 2.f
            );

            std::shared_ptr<UIWidget> bestMatch;
            float bestDistance = std::numeric_limits<float>::max();

            for (auto& w : focusableWidgets) {
                if (w == m_focusedWidget) continue;
                sf::FloatRect bounds = w->getGlobalBounds();
                sf::Vector2f center(
                    bounds.left + bounds.width / 2.f,
                    bounds.top + bounds.height / 2.f
                );
                sf::Vector2f delta = center - currentCenter;

                bool validDirection = false;
                if (key == sf::Keyboard::Left && delta.x < 0) validDirection = true;
                if (key == sf::Keyboard::Right && delta.x > 0) validDirection = true;

                if (validDirection) {
                    float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);
                    if (dist < bestDistance) {
                        bestDistance = dist;
                        bestMatch = w;
                    }
                }
            }

            if (bestMatch) {
                setFocusedWidget(bestMatch);
            }
        }

        void UIManager::announce(const std::string& text, float duration) {
            m_currentAnnouncement = ScreenReaderAnnouncement(text, duration);
        }

        std::string UIManager::getCurrentAnnouncement() const {
            if (m_currentAnnouncement.elapsed < m_currentAnnouncement.duration) {
                return m_currentAnnouncement.text;
            }
            return std::string();
        }

        void UIManager::enableScreenReader(bool enabled) {
            m_screenReaderEnabled = enabled;
        }

        bool UIManager::isScreenReaderEnabled() const {
            return m_screenReaderEnabled;
        }

        void UIManager::setScreenReaderDescribeWidget(
            std::function<std::string(std::shared_ptr<UIWidget>)> describeFunc) {
            m_describeWidget = std::move(describeFunc);
        }

        std::vector<std::shared_ptr<UIWidget>> UIManager::collectFocusableWidgets() const {
            std::vector<std::shared_ptr<UIWidget>> result;

            if (!m_tabOrder.empty()) {
                for (auto& name : m_tabOrder) {
                    auto it = m_widgetMap.find(name);
                    if (it != m_widgetMap.end() && it->second->focusable &&
                        it->second->visible && it->second->enabled) {
                        result.push_back(it->second);
                    }
                }
                return result;
            }

            std::function<void(std::shared_ptr<UIWidget>)> collect;
            collect = [&](std::shared_ptr<UIWidget> w) {
                if (w->focusable && w->visible && w->enabled) {
                    result.push_back(w);
                }
                for (auto& child : w->m_children) {
                    collect(child);
                }
            };
            collect(m_root);
            return result;
        }

        std::string UIManager::defaultDescribeWidget(std::shared_ptr<UIWidget> widget) const {
            if (!widget) return std::string();
            std::string desc;
            if (!widget->name.empty() && widget->name != "__root__") {
                desc = widget->name;
            }
            if (!widget->tooltip.empty()) {
                if (!desc.empty()) desc += ": ";
                desc += widget->tooltip;
            }
            if (desc.empty()) {
                desc = "Widget";
            }
            return desc;
        }

        void UIManager::handleModalInput(const sf::Event& event) {
            auto topModal = getTopModal();
            if (!topModal) return;

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    popModal();
                    return;
                }
                if (event.key.code == sf::Keyboard::Tab) {
                    handleNavigation(event);
                    return;
                }
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2f mousePos(
                    static_cast<float>(event.mouseButton.x) / m_dpiScale,
                    static_cast<float>(event.mouseButton.y) / m_dpiScale
                );
                if (!topModal->containsPoint(mousePos)) {
                    return;
                }
            }

            topModal->onEvent(event);
        }

        void UIManager::renderModalOverlay(sf::RenderTarget& target) {
            sf::RectangleShape overlay;
            overlay.setSize(sf::Vector2f(1920.f, 1080.f));
            overlay.setFillColor(sf::Color(0, 0, 0, 120));
            target.draw(overlay);
        }

    }
}
