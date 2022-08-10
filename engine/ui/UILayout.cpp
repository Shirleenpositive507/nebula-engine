#include "UILayout.h"
#include <algorithm>

namespace nebula {
    namespace ui {

        UILayout::UILayout()
            : spacing(4.f)
            , padding(0.f)
            , margin(0.f)
            , anchorH(Anchor::Left)
            , anchorV(Anchor::Top) {}

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

        float HorizontalLayout::getTotalFixedWidth(const std::vector<std::shared_ptr<UIWidget>>& widgets) const {
            float total = 0.f;
            for (auto& w : widgets) {
                total += w->getSize().x;
            }
            total += spacing * std::max(0, static_cast<int>(widgets.size()) - 1);
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

            float x = padding;
            float totalFixed = getTotalFixedWidth(widgets);
            float available = containerSize.x - padding * 2.f;
            int expandCount = getExpandCount(widgets);
            float expandWidth = 0.f;
            if (expandCount > 0 && totalFixed < available) {
                expandWidth = (available - totalFixed) / static_cast<float>(expandCount);
            }

            for (auto& w : widgets) {
                sf::Vector2f size = w->getSize();
                if (fill) {
                    size.y = containerSize.y - padding * 2.f;
                }
                if (expand) {
                    size.x += expandWidth;
                }
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
                x += size.x + spacing;
            }
        }

        float VerticalLayout::getTotalFixedHeight(const std::vector<std::shared_ptr<UIWidget>>& widgets) const {
            float total = 0.f;
            for (auto& w : widgets) {
                total += w->getSize().y;
            }
            total += spacing * std::max(0, static_cast<int>(widgets.size()) - 1);
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

            float y = padding;
            float totalFixed = getTotalFixedHeight(widgets);
            float available = containerSize.y - padding * 2.f;
            int expandCount = getExpandCount(widgets);
            float expandHeight = 0.f;
            if (expandCount > 0 && totalFixed < available) {
                expandHeight = (available - totalFixed) / static_cast<float>(expandCount);
            }

            for (auto& w : widgets) {
                sf::Vector2f size = w->getSize();
                if (fill) {
                    size.x = containerSize.x - padding * 2.f;
                }
                if (expand) {
                    size.y += expandHeight;
                }
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
                y += size.y + spacing;
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
                sf::Vector2f size = w->getSize();

                if (x + size.x > containerSize.x - padding && x > padding) {
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
