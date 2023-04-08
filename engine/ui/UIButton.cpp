#include "UIButton.h"
#include <algorithm>
#include <cmath>

namespace nebula {
    namespace ui {

        std::unordered_map<std::string, std::weak_ptr<UIButton>> UIButton::s_groupSelection;

        UIButton::UIButton()
            : UIWidget("Button")
            , m_state(ButtonState::Normal)
            , m_targetState(ButtonState::Normal)
            , m_cornerRadius(4.f)
            , m_autoSize(true)
            , m_hoverLerp(0.f)
            , m_scalePulse(0.f)
            , m_toggleMode(false)
            , m_toggled(false)
            , m_radioExclusive(true)
            , m_shortcutKey(sf::Keyboard::Unknown)
            , m_shortcutCtrl(false)
            , m_shortcutAlt(false)
            , m_shortcutShift(false)
            , m_autoRepeat(false)
            , m_autoRepeatDelay(0.5f)
            , m_autoRepeatRate(0.05f)
            , m_repeatTimer(0.f)
            , m_repeatFired(false)
            , m_iconSize(16.f, 16.f) {
            m_size.x = 120.f;
            m_size.y = 32.f;
            focusable = true;

            m_label = std::make_shared<UILabel>();
            m_label->setAutoSize(false);
            addChild(m_label);

            for (int i = 0; i < 4; ++i) {
                m_colors[i].bg = graphics::Color(200, 200, 200);
                m_colors[i].text = graphics::Color(0, 0, 0);
                m_colors[i].border = graphics::Color(150, 150, 150);
            }

            m_colors[0].bg = graphics::Color(60, 60, 60);
            m_colors[0].text = graphics::Color(220, 220, 220);
            m_colors[0].border = graphics::Color(80, 80, 80);

            m_colors[1].bg = graphics::Color(80, 80, 80);
            m_colors[1].text = graphics::Color(240, 240, 240);
            m_colors[1].border = graphics::Color(100, 100, 100);

            m_colors[2].bg = graphics::Color(40, 40, 40);
            m_colors[2].text = graphics::Color(200, 200, 200);
            m_colors[2].border = graphics::Color(60, 60, 60);

            m_colors[3].bg = graphics::Color(100, 100, 100);
            m_colors[3].text = graphics::Color(140, 140, 140);
            m_colors[3].border = graphics::Color(100, 100, 100);

            updateAppearance();
        }

        UIButton::UIButton(const std::string& text)
            : UIButton() {
            setText(text);
        }

        void UIButton::setText(const std::string& text) {
            m_label->setText(text);
            if (m_autoSize) {
                onLayout();
            }
        }

        void UIButton::setFont(sf::Font& font) {
            m_label->setFont(font);
        }

        void UIButton::setBackgroundColor(ButtonState state, const graphics::Color& color) {
            m_colors[static_cast<int>(state)].bg = color;
            if (m_state == state) updateAppearance();
        }

        void UIButton::setTextColor(ButtonState state, const graphics::Color& color) {
            m_colors[static_cast<int>(state)].text = color;
            if (m_state == state) updateAppearance();
        }

        void UIButton::setBorderColor(ButtonState state, const graphics::Color& color) {
            m_colors[static_cast<int>(state)].border = color;
            if (m_state == state) updateAppearance();
        }

        void UIButton::setTexture(ButtonState state, const std::shared_ptr<sf::Texture>& texture) {
            m_colors[static_cast<int>(state)].texture = texture;
            if (m_state == state) updateAppearance();
        }

        void UIButton::setCornerRadius(float radius) {
            m_cornerRadius = radius;
        }

        void UIButton::setAutoSize(bool autoSize) {
            m_autoSize = autoSize;
        }

        void UIButton::setToggleMode(bool toggle) {
            m_toggleMode = toggle;
        }

        bool UIButton::isToggleMode() const {
            return m_toggleMode;
        }

        void UIButton::setToggled(bool toggled) {
            m_toggled = toggled;
            if (m_toggleMode) {
                if (toggled && !m_radioGroup.empty() && m_radioExclusive) {
                    setGroupSelection(m_radioGroup, shared_from_this());
                }
                updateAppearance();
            }
        }

        bool UIButton::isToggled() const {
            return m_toggled;
        }

        void UIButton::setRadioGroup(const std::string& group) {
            m_radioGroup = group;
        }

        std::string UIButton::getRadioGroup() const {
            return m_radioGroup;
        }

        void UIButton::setRadioGroupExclusive(bool exclusive) {
            m_radioExclusive = exclusive;
        }

        void UIButton::setIcon(const std::string& iconPath) {
            auto tex = std::make_shared<sf::Texture>();
            if (tex->loadFromFile(iconPath)) {
                m_iconTexture = tex;
                m_iconSprite.setTexture(*m_iconTexture);
                m_iconSprite.setScale(
                    m_iconSize.x / static_cast<float>(tex->getSize().x),
                    m_iconSize.y / static_cast<float>(tex->getSize().y)
                );
            }
        }

        void UIButton::setIcon(const std::shared_ptr<sf::Texture>& iconTexture) {
            m_iconTexture = iconTexture;
            if (iconTexture) {
                m_iconSprite.setTexture(*iconTexture);
                m_iconSprite.setScale(
                    m_iconSize.x / static_cast<float>(iconTexture->getSize().x),
                    m_iconSize.y / static_cast<float>(iconTexture->getSize().y)
                );
            }
        }

        void UIButton::setIconSize(float width, float height) {
            m_iconSize = sf::Vector2f(width, height);
            if (m_iconTexture) {
                m_iconSprite.setScale(
                    m_iconSize.x / static_cast<float>(m_iconTexture->getSize().x),
                    m_iconSize.y / static_cast<float>(m_iconTexture->getSize().y)
                );
            }
        }

        void UIButton::setShortcutKey(sf::Keyboard::Key key, bool ctrl, bool alt, bool shift) {
            m_shortcutKey = key;
            m_shortcutCtrl = ctrl;
            m_shortcutAlt = alt;
            m_shortcutShift = shift;
        }

        sf::Keyboard::Key UIButton::getShortcutKey() const {
            return m_shortcutKey;
        }

        bool UIButton::matchesShortcut(const sf::Event& event) const {
            if (event.type != sf::Event::KeyPressed) return false;
            if (m_shortcutKey == sf::Keyboard::Unknown) return false;
            if (event.key.code != m_shortcutKey) return false;
            if (m_shortcutCtrl != (event.key.control)) return false;
            if (m_shortcutAlt != (event.key.alt)) return false;
            if (m_shortcutShift != (event.key.shift)) return false;
            return true;
        }

        void UIButton::setAutoRepeat(bool repeat) {
            m_autoRepeat = repeat;
            m_repeatTimer = 0.f;
            m_repeatFired = false;
        }

        bool UIButton::isAutoRepeat() const {
            return m_autoRepeat;
        }

        void UIButton::setAutoRepeatDelay(float delay) {
            m_autoRepeatDelay = delay;
        }

        void UIButton::setAutoRepeatRate(float rate) {
            m_autoRepeatRate = rate;
        }

        std::shared_ptr<UIButton> UIButton::getSelectedInGroup(const std::string& group) {
            auto it = s_groupSelection.find(group);
            if (it != s_groupSelection.end()) {
                return it->second.lock();
            }
            return nullptr;
        }

        void UIButton::setGroupSelection(const std::string& group, std::shared_ptr<UIButton> button) {
            s_groupSelection[group] = button;
        }

        graphics::Color UIButton::lerpColor(const graphics::Color& a, const graphics::Color& b, float t) const {
            return graphics::Color::lerp(a, b, t);
        }

        void UIButton::updateAppearance() {
            const auto& colors = m_colors[static_cast<int>(m_state)];

            graphics::Color currentBg = colors.bg;
            if (m_hoverLerp > 0.f && m_hoverLerp < 1.f) {
                const auto& fromColors = m_colors[static_cast<int>(ButtonState::Normal)];
                const auto& toColors = m_colors[static_cast<int>(ButtonState::Hovered)];
                currentBg = lerpColor(fromColors.bg, toColors.bg, m_hoverLerp);
            }

            if (m_toggleMode && m_toggled) {
                currentBg = lerpColor(currentBg, graphics::Color(100, 180, 100), 0.5f);
            }

            m_background.setFillColor(currentBg.toSFML());
            m_background.setOutlineColor(colors.border.toSFML());
            m_background.setOutlineThickness(style.borderSize);
            m_background.setSize(m_size);

            if (colors.texture) {
                m_background.setTexture(colors.texture.get());
            } else {
                m_background.setTexture(nullptr);
            }

            updateLabelStyle();
        }

        void UIButton::updateLabelStyle() {
            const auto& colors = m_colors[static_cast<int>(m_state)];
            m_label->setColor(colors.text);
            m_label->setCharacterSize(style.fontSize);
            m_label->setAlignment(TextAlignment::Center);
            m_label->setPosition(0.f, 0.f);
            m_label->setSize(m_size.x, m_size.y);
        }

        void UIButton::onRender(sf::RenderTarget& target, sf::RenderStates states) {
            if (!visible) return;

            states.transform.translate(m_position);

            float scale = 1.f + m_scalePulse * 0.05f;
            if (scale != 1.f) {
                sf::Vector2f center(m_size.x / 2.f, m_size.y / 2.f);
                states.transform.translate(center);
                states.transform.scale(scale, scale);
                states.transform.translate(-center);
            }

            target.draw(m_background, states);

            if (m_iconTexture) {
                float iconX = style.padding;
                float iconY = (m_size.y - m_iconSize.y) / 2.f;
                m_iconSprite.setPosition(iconX, iconY);
                target.draw(m_iconSprite, states);
            }

            UIWidget::onRender(target, states);
        }

        void UIButton::onUpdate(float dt) {
            UIWidget::onUpdate(dt);

            if (m_state != m_targetState) {
                if (m_targetState == ButtonState::Hovered) {
                    m_hoverLerp = std::min(1.f, m_hoverLerp + dt * 8.f);
                    if (m_hoverLerp >= 1.f) {
                        m_state = m_targetState;
                        m_hoverLerp = 0.f;
                    }
                } else {
                    m_hoverLerp = std::max(0.f, m_hoverLerp - dt * 8.f);
                    if (m_hoverLerp <= 0.f) {
                        m_state = m_targetState;
                    }
                }
                updateAppearance();
            }

            if (m_scalePulse > 0.f) {
                m_scalePulse = std::max(0.f, m_scalePulse - dt * 6.f);
            }

            if (m_autoRepeat && m_pressed) {
                handleAutoRepeat(dt);
            }
        }

        void UIButton::onLayout() {
            if (m_autoSize && m_label) {
                sf::FloatRect textBounds = m_label->getTextBounds();
                float iconOffset = m_iconTexture ? m_iconSize.x + 4.f : 0.f;
                m_size.x = textBounds.width + style.padding * 2.f + 16.f + iconOffset;
                m_size.y = textBounds.height + style.padding * 2.f + 8.f;
            }
            m_background.setSize(m_size);
            if (m_label) {
                float iconOffset = m_iconTexture ? m_iconSize.x + 4.f : 0.f;
                m_label->setPosition(iconOffset, 0.f);
                m_label->setSize(m_size.x - iconOffset, m_size.y);
            }
            UIWidget::onLayout();
        }

        bool UIButton::onEvent(const sf::Event& event) {
            if (!enabled || !visible) return false;

            if (matchesShortcut(event)) {
                if (m_toggleMode) {
                    m_toggled = !m_toggled;
                    if (!m_radioGroup.empty() && m_radioExclusive && m_toggled) {
                        setGroupSelection(m_radioGroup, shared_from_this());
                    }
                    updateAppearance();
                    if (onToggled) onToggled();
                }
                if (onClick) onClick();
                if (onClicked) onClicked();
                return true;
            }

            if (event.type == sf::Event::MouseMoved) {
                sf::Vector2f mousePos(static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y));
                bool hovered = containsPoint(mousePos);
                if (hovered && m_targetState != ButtonState::Pressed) {
                    m_targetState = ButtonState::Hovered;
                    m_hovered = true;
                    if (onHover) onHover();
                    if (onHovered) onHovered();
                } else if (!hovered && m_targetState != ButtonState::Pressed) {
                    m_targetState = ButtonState::Normal;
                    m_hovered = false;
                }
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
                if (containsPoint(mousePos)) {
                    m_targetState = ButtonState::Pressed;
                    m_pressed = true;
                    m_scalePulse = 1.f;
                    m_repeatTimer = 0.f;
                    m_repeatFired = false;
                    if (onPressed) onPressed();
                    return true;
                }
            }

            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                if (m_pressed) {
                    m_pressed = false;
                    sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
                    if (containsPoint(mousePos)) {
                        m_targetState = ButtonState::Hovered;

                        if (m_toggleMode) {
                            m_toggled = !m_toggled;
                            if (!m_radioGroup.empty() && m_radioExclusive && m_toggled) {
                                setGroupSelection(m_radioGroup, shared_from_this());
                            }
                            updateAppearance();
                            if (onToggled) onToggled();
                        }

                        if (onClick) onClick();
                        if (onClicked) onClicked();
                    } else {
                        m_targetState = ButtonState::Normal;
                    }
                    if (onRelease) onRelease();
                    if (onReleased) onReleased();
                    return true;
                }
            }

            return UIWidget::onEvent(event);
        }

        void UIButton::handleAutoRepeat(float dt) {
            m_repeatTimer += dt;
            if (!m_repeatFired) {
                if (m_repeatTimer >= m_autoRepeatDelay) {
                    m_repeatFired = true;
                    m_repeatTimer = 0.f;
                    if (onClick) onClick();
                    if (onClicked) onClicked();
                }
            } else {
                if (m_repeatTimer >= m_autoRepeatRate) {
                    m_repeatTimer = 0.f;
                    if (onClick) onClick();
                    if (onClicked) onClicked();
                }
            }
        }

    }
}
