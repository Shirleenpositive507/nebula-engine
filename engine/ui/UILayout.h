#pragma once

#include "UIWidget.h"
#include <memory>
#include <vector>

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

        class UILayout {
        public:
            UILayout();
            virtual ~UILayout() = default;

            virtual void apply(std::vector<std::shared_ptr<UIWidget>>& widgets, const sf::Vector2f& containerSize);
            virtual void recalculate();

            void setSpacing(float spacing);
            void setPadding(float padding);
            void fitToContent();

            float spacing;
            float padding;
            float margin;

            Anchor anchorH;
            Anchor anchorV;

        protected:
            std::vector<std::shared_ptr<UIWidget>> m_widgets;
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
        };

    }
}
