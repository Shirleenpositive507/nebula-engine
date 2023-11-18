#include "UIDropdown.h"
#include <algorithm>

namespace nebula {
namespace ui {

UIDropdown::UIDropdown()
    : UIWidget("dropdown")
    , m_selectedIndex(-1)
    , m_expanded(false)
    , m_maxVisibleItems(5)
    , m_itemHeight(24.f)
    , m_dropdownHeight(0.f)
{
    setSize(160.f, 28.f);
    m_dropdownPanel = std::make_shared<UIPanel>("dropdown_list");
}

UIDropdown::UIDropdown(const std::string& name)
    : UIWidget(name)
    , m_selectedIndex(-1)
    , m_expanded(false)
    , m_maxVisibleItems(5)
    , m_itemHeight(24.f)
    , m_dropdownHeight(0.f)
{
    setSize(160.f, 28.f);
    m_dropdownPanel = std::make_shared<UIPanel>(name + "_list");
}

void UIDropdown::addItem(const std::string& item) {
    m_items.push_back(item);
    rebuildDropdownList();
}

void UIDropdown::addItems(const std::vector<std::string>& items) {
    m_items.insert(m_items.end(), items.begin(), items.end());
    rebuildDropdownList();
}

void UIDropdown::removeItem(std::size_t index) {
    if (index < m_items.size()) {
        m_items.erase(m_items.begin() + static_cast<ptrdiff_t>(index));
        if (m_selectedIndex == static_cast<int>(index)) {
            m_selectedIndex = -1;
        }
        rebuildDropdownList();
    }
}

void UIDropdown::clearItems() {
    m_items.clear();
    m_selectedIndex = -1;
    m_expanded = false;
    rebuildDropdownList();
}

std::size_t UIDropdown::getItemCount() const {
    return m_items.size();
}

const std::string& UIDropdown::getItem(std::size_t index) const {
    return m_items[index];
}

void UIDropdown::setSelectedIndex(int index) {
    selectItem(index);
}

int UIDropdown::getSelectedIndex() const {
    return m_selectedIndex;
}

std::string UIDropdown::getSelectedItem() const {
    if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_items.size())) {
        return m_items[m_selectedIndex];
    }
    return "";
}

void UIDropdown::setExpanded(bool expanded) {
    m_expanded = expanded;
    if (m_expanded) {
        rebuildDropdownList();
    }
}

bool UIDropdown::isExpanded() const {
    return m_expanded;
}

void UIDropdown::setMaxVisibleItems(std::size_t count) {
    m_maxVisibleItems = count;
    rebuildDropdownList();
}

std::size_t UIDropdown::getMaxVisibleItems() const {
    return m_maxVisibleItems;
}

void UIDropdown::setItemHeight(float height) {
    m_itemHeight = height;
    rebuildDropdownList();
}

float UIDropdown::getItemHeight() const {
    return m_itemHeight;
}

void UIDropdown::setDropdownHeight(float height) {
    m_dropdownHeight = height;
}

float UIDropdown::getDropdownHeight() const {
    return m_dropdownHeight;
}

void UIDropdown::onRender(sf::RenderTarget& target, const sf::RenderStates& states) const {
    UIWidget::onRender(target, states);

    if (m_expanded && m_dropdownPanel) {
        m_dropdownPanel->onRender(target, states);
    }
}

void UIDropdown::onEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed) {
        sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
        if (getGlobalBounds().contains(mousePos)) {
            m_expanded = !m_expanded;
            if (m_expanded) rebuildDropdownList();
            return;
        }
        if (m_expanded) {
            for (std::size_t i = 0; i < m_itemBounds.size(); ++i) {
                if (m_itemBounds[i].contains(mousePos)) {
                    selectItem(static_cast<int>(i));
                    m_expanded = false;
                    return;
                }
            }
            m_expanded = false;
        }
    }
}

void UIDropdown::onUpdate(float dt) {
    UIWidget::onUpdate(dt);
    if (m_dropdownPanel) {
        m_dropdownPanel->onUpdate(dt);
    }
}

void UIDropdown::onLayout() {
    UIWidget::onLayout();
    rebuildDropdownList();
}

sf::FloatRect UIDropdown::getGlobalBounds() const {
    return UIWidget::getGlobalBounds();
}

void UIDropdown::rebuildDropdownList() {
    m_itemBounds.clear();
    if (!m_expanded || m_items.empty()) return;

    sf::FloatRect mainBounds = getGlobalBounds();
    std::size_t visibleCount = std::min(m_items.size(), m_maxVisibleItems);
    float totalHeight = static_cast<float>(visibleCount) * m_itemHeight;

    for (std::size_t i = 0; i < visibleCount; ++i) {
        sf::FloatRect itemRect(
            mainBounds.left,
            mainBounds.top + mainBounds.height + static_cast<float>(i) * m_itemHeight,
            mainBounds.width,
            m_itemHeight
        );
        m_itemBounds.push_back(itemRect);
    }
    m_dropdownHeight = totalHeight;
}

void UIDropdown::selectItem(int index) {
    if (index >= 0 && index < static_cast<int>(m_items.size())) {
        m_selectedIndex = index;
        if (onItemSelected) {
            onItemSelected(index, m_items[index]);
        }
    }
}

}
}

