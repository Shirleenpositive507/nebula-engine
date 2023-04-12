#include "UILayout.h"
#include <algorithm>

namespace nebula {
    namespace ui {

        UILayout::UILayout()
            : spacing(4.f)
            , padding(0.f)
            , margin(0.f)
            , anchorH(Anchor::Left)
            , anchorV(Anchor::Top)
            , m_debugDraw(false) {}

        void UILayout::apply(std::vector<std::shared_ptr<UIWidget>>& widgets, const sf::Vector2f& containerSize) {
            m_widgets = widgets;
        }

        void UILayout::recalculate() {}

        void UILayout::setSpacing(float s) { spacing = s; }
        void UILayout::setPadding(float p) { padding = p; }

        void UILayout::fitToContent() {
            if (m_widgets.empty()) return;
            float maxX = 0.f, maxY = 0.f;
            for (auto& w : m_widgets) {
                sf::FloatRect bounds = w->getGlobalBounds();
                maxX = std::max(maxX, bounds.left + bounds.width);
                maxY = std::max(maxY, bounds.top + bounds.height);
            }
        }

        void UILayout::setConstraint(const LayoutConstraint& constraint) {
            m_constraint = constraint;
        }

        LayoutConstraint UILayout::getConstraint() const {
            return m_constraint;
        }

        void UILayout::setSpacingRule(const SpacingRule& rule) {
            m_spacingRule = rule;
        }

        SpacingRule UILayout::getSpacingRule() const {
            return m_spacingRule;
        }

        void UILayout::setDebugDraw(bool debug) {
            m_debugDraw = debug;
        }

        bool UILayout::isDebugDraw() const {
            return m_debugDraw;
        }

        void UILayout::setDebugDrawFunction(std::function<void(const sf::FloatRect&, const graphics::Color&)> drawFunc) {
            m_debugDrawFunc = std::move(drawFunc);
        }

        void UILayout::applyConstraints(std::shared_ptr<UIWidget> widget) {
            sf::Vector2f size = widget->getSize();
            size.x = std::max(m_constraint.minWidth, std::min(m_constraint.maxWidth, size.x));
            size.y = std::max(m_constraint.minHeight, std::min(m_constraint.maxHeight, size.y));
            widget->setSize(size);
        }

        void UILayout::debugDrawRect(const sf::FloatRect& rect, const graphics::Color& color) {
            if (m_debugDraw && m_debugDrawFunc) {
                m_debugDrawFunc(rect, color);
            }
        }

        float HorizontalLayout::getTotalFixedWidth(const std::vector<std::shared_ptr<UIWidget>>& widgets) const {
            float total = 0.f;
            for (auto& w : widgets) {
                total += w->getSize().x;
            }
            total += m_spacingRule.between * std::max(0, static_cast<int>(widgets.size()) - 1);
            total += m_spacingRule.before + m_spacingRule.after;
            return total;
        }

        int HorizontalLayout::getExpandCount(const std::vector<std::shared_ptr<UIWidget>>& widgets) const {
            int count = 0;
            for (auto& w : widgets) {
                (void)w;
                count++;
            }
            return count;
        }

        void HorizontalLayout::apply(std::vector<std::shared_ptr<UIWidget>>& widgets, const sf::Vector2f& containerSize) {
            m_widgets = widgets;
            if (widgets.empty()) return;

            std::sort(widgets.begin(), widgets.end(),
                [](const std::shared_ptr<UIWidget>& a, const std::shared_ptr<UIWidget>& b) {
                    (void)a; (void)b;
                    return true;
                });

            float x = padding + m_spacingRule.before;
            float totalFixed = getTotalFixedWidth(widgets);
            float available = containerSize.x - padding * 2.f - m_spacingRule.before - m_spacingRule.after;
            int expandCount = getExpandCount(widgets);
            float expandWidth = 0.f;

            if (expandCount > 0 && totalFixed < available) {
                float remaining = available - (totalFixed - m_spacingRule.before - m_spacingRule.after);
                float prioritySum = 0.f;
                for (auto& w : widgets) {
                    (void)w;
                    prioritySum += 0.5f;
                }
                expandWidth = prioritySum > 0.f ? remaining / expandCount : 0.f;
            }

            for (auto& w : widgets) {
                applyConstraints(w);
                sf::Vector2f size = w->getSize();
                if (fill) {
                    size.y = containerSize.y - padding * 2.f;
                }
                if (expand) {
                    size.x += expandWidth;
                }
                size.x = std::max(m_constraint.minWidth, std::min(m_constraint.maxWidth, size.x));
                w->setSize(size);

                float y = padding;
                switch (anchorV) {
                    case Anchor::Center:
                        y = (containerSize.y - size.y) / 2.f;
                        break;
                    case Anchor::Bottom:
                        y = containerSize.y - size.y - padding;
                        break;
                    default:
                        break;
                }

                w->setPosition(x, y);

                if (m_debugDraw) {
                    sf::FloatRect dbg(x, y, size.x, size.y);
                    debugDrawRect(dbg, graphics::Color(0, 255, 0, 80));
                }

                x += size.x + m_spacingRule.between;
            }
        }

        float VerticalLayout::getTotalFixedHeight(const std::vector<std::shared_ptr<UIWidget>>& widgets) const {
            float total = 0.f;
            for (auto& w : widgets) {
                total += w->getSize().y;
            }
            total += m_spacingRule.between * std::max(0, static_cast<int>(widgets.size()) - 1);
            total += m_spacingRule.before + m_spacingRule.after;
            return total;
        }

        int VerticalLayout::getExpandCount(const std::vector<std::shared_ptr<UIWidget>>& widgets) const {
            int count = 0;
            for (auto& w : widgets) {
                (void)w;
                count++;
            }
            return count;
        }

        void VerticalLayout::apply(std::vector<std::shared_ptr<UIWidget>>& widgets, const sf::Vector2f& containerSize) {
            m_widgets = widgets;
            if (widgets.empty()) return;

            std::sort(widgets.begin(), widgets.end(),
                [](const std::shared_ptr<UIWidget>& a, const std::shared_ptr<UIWidget>& b) {
                    (void)a; (void)b;
                    return true;
                });

            float y = padding + m_spacingRule.before;
            float totalFixed = getTotalFixedHeight(widgets);
            float available = containerSize.y - padding * 2.f - m_spacingRule.before - m_spacingRule.after;
            int expandCount = getExpandCount(widgets);
            float expandHeight = 0.f;

            if (expandCount > 0 && totalFixed < available) {
                float remaining = available - (totalFixed - m_spacingRule.before - m_spacingRule.after);
                expandHeight = remaining / static_cast<float>(expandCount);
            }

            for (auto& w : widgets) {
                applyConstraints(w);
                sf::Vector2f size = w->getSize();
                if (fill) {
                    size.x = containerSize.x - padding * 2.f;
                }
                if (expand) {
                    size.y += expandHeight;
                }
                size.y = std::max(m_constraint.minHeight, std::min(m_constraint.maxHeight, size.y));
                w->setSize(size);

                float x = padding;
                switch (anchorH) {
                    case Anchor::Center:
                        x = (containerSize.x - size.x) / 2.f;
                        break;
                    case Anchor::Right:
                        x = containerSize.x - size.x - padding;
                        break;
                    default:
                        break;
                }

                w->setPosition(x, y);

                if (m_debugDraw) {
                    sf::FloatRect dbg(x, y, size.x, size.y);
                    debugDrawRect(dbg, graphics::Color(0, 255, 0, 80));
                }

                y += size.y + m_spacingRule.between;
            }
        }

        void GridLayout::apply(std::vector<std::shared_ptr<UIWidget>>& widgets, const sf::Vector2f& containerSize) {
            m_widgets = widgets;
            if (widgets.empty()) return;

            int actualColumns = columns > 0 ? columns : static_cast<int>(widgets.size());
            float cellW = (containerSize.x - padding * 2.f - spacing * (actualColumns - 1)) / static_cast<float>(actualColumns);
            float cellH = cellSize.y > 0.f ? cellSize.y : cellW;

            for (size_t i = 0; i < widgets.size(); ++i) {
                int col = static_cast<int>(i) % actualColumns;
                int row = static_cast<int>(i) / actualColumns;

                float x = padding + col * (cellW + spacing);
                float y = padding + row * (cellH + spacing);

                applyConstraints(widgets[i]);
                widgets[i]->setPosition(x, y);
                widgets[i]->setSize(cellW, cellH);
            }
        }

        void FlowLayout::apply(std::vector<std::shared_ptr<UIWidget>>& widgets, const sf::Vector2f& containerSize) {
            m_widgets = widgets;
            if (widgets.empty()) return;

            float x = padding;
            float y = padding;
            float maxLineHeight = 0.f;

            for (auto& w : widgets) {
                applyConstraints(w);
                sf::Vector2f size = w->getSize();

                if (wrap && x + size.x > containerSize.x - padding && x > padding) {
                    x = padding;
                    y += maxLineHeight + spacing;
                    maxLineHeight = 0.f;
                }

                w->setPosition(x, y);
                maxLineHeight = std::max(maxLineHeight, size.y);
                x += size.x + spacing;
            }
        }

    }
}
