#pragma once

#include "UIWidget.h"
#include <memory>
#include <vector>
#include <functional>

namespace nebula {
    namespace ui {

        enum class Anchor {
            Top,
            Bottom,
            Left,
            Right,
            Center,
            Fill
        };

        struct LayoutConstraint {
            float minWidth;
            float maxWidth;
            float minHeight;
            float maxHeight;
            float priority;

            LayoutConstraint()
                : minWidth(0.f), maxWidth(10000.f)
                , minHeight(0.f), maxHeight(10000.f)
                , priority(0.5f) {}
        };

        struct SpacingRule {
            float before;
            float after;
            float between;

            SpacingRule() : before(0.f), after(0.f), between(4.f) {}
        };

        class UILayout {
        public:
            UILayout();
            virtual ~UILayout() = default;

            virtual void apply(std::vector<std::shared_ptr<UIWidget>>& widgets, const sf::Vector2f& containerSize);
            virtual void recalculate();

            void setSpacing(float spacing);
            void setPadding(float padding);
            void fitToContent();

            void setConstraint(const LayoutConstraint& constraint);
            LayoutConstraint getConstraint() const;

            void setSpacingRule(const SpacingRule& rule);
            SpacingRule getSpacingRule() const;

            void setDebugDraw(bool debug);
            bool isDebugDraw() const;
            void setDebugDrawFunction(std::function<void(const sf::FloatRect&, const graphics::Color&)> drawFunc);

            float spacing;
            float padding;
            float margin;

            Anchor anchorH;
            Anchor anchorV;

        protected:
            std::vector<std::shared_ptr<UIWidget>> m_widgets;
            LayoutConstraint m_constraint;
            SpacingRule m_spacingRule;
            bool m_debugDraw;
            std::function<void(const sf::FloatRect&, const graphics::Color&)> m_debugDrawFunc;

            void applyConstraints(std::shared_ptr<UIWidget> widget);
            void debugDrawRect(const sf::FloatRect& rect, const graphics::Color& color);
        };

        class HorizontalLayout : public UILayout {
        public:
            void apply(std::vector<std::shared_ptr<UIWidget>>& widgets, const sf::Vector2f& containerSize) override;

            bool expand;
            bool fill;

        private:
            float getTotalFixedWidth(const std::vector<std::shared_ptr<UIWidget>>& widgets) const;
            int getExpandCount(const std::vector<std::shared_ptr<UIWidget>>& widgets) const;
        };

        class VerticalLayout : public UILayout {
        public:
            void apply(std::vector<std::shared_ptr<UIWidget>>& widgets, const sf::Vector2f& containerSize) override;

            bool expand;
            bool fill;

        private:
            float getTotalFixedHeight(const std::vector<std::shared_ptr<UIWidget>>& widgets) const;
            int getExpandCount(const std::vector<std::shared_ptr<UIWidget>>& widgets) const;
        };

        class GridLayout : public UILayout {
        public:
            void apply(std::vector<std::shared_ptr<UIWidget>>& widgets, const sf::Vector2f& containerSize) override;

            int columns;
            int rows;
            sf::Vector2f cellSize;
        };

        class FlowLayout : public UILayout {
        public:
            void apply(std::vector<std::shared_ptr<UIWidget>>& widgets, const sf::Vector2f& containerSize) override;

            bool wrap;
            sf::Vector2f preferredItemSize;
        };

    }
}
