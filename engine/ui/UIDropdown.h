#pragma once

#include "UIWidget.h"
#include "UIButton.h"
#include "UILabel.h"
#include "UIPanel.h"
#include <vector>
#include <functional>

namespace nebula {
namespace ui {

class UIDropdown : public UIWidget {
public:
    UIDropdown();
    explicit UIDropdown(const std::string& name);

    void addItem(const std::string& item);
    void addItems(const std::vector<std::string>& items);
    void removeItem(std::size_t index);
    void clearItems();
    std::size_t getItemCount() const;
    const std::string& getItem(std::size_t index) const;

    void setSelectedIndex(int index);
    int getSelectedIndex() const;
    std::string getSelectedItem() const;

    void setExpanded(bool expanded);
    bool isExpanded() const;

    void setMaxVisibleItems(std::size_t count);
    std::size_t getMaxVisibleItems() const;

    void setItemHeight(float height);
    float getItemHeight() const;

    void setDropdownHeight(float height);
    float getDropdownHeight() const;

    std::function<void(int index, const std::string& item)> onItemSelected;

    void onRender(sf::RenderTarget& target, const sf::RenderStates& states) const override;
    void onEvent(const sf::Event& event) override;
    void onUpdate(float dt) override;
    void onLayout() override;
    sf::FloatRect getGlobalBounds() const override;

private:
    void rebuildDropdownList();
    void selectItem(int index);

    std::vector<std::string> m_items;
    int m_selectedIndex;
    bool m_expanded;
    std::size_t m_maxVisibleItems;
    float m_itemHeight;
    float m_dropdownHeight;
    sf::RectangleShape m_dropdownBg;
    sf::RectangleShape m_arrowShape;
    std::vector<sf::FloatRect> m_itemBounds;
    std::shared_ptr<UIPanel> m_dropdownPanel;
};

}
}

