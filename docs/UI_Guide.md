# UI System Guide

## Widgets

The UI system provides a complete widget toolkit:

```cpp
#include <Nebula/UI.h>

auto panel = nebula::ui::Panel::create({400, 300, 200, 150});
panel->setStyle({
    .backgroundColor = {0.2, 0.2, 0.2, 0.9},
    .borderColor = {1, 1, 1, 0.5},
    .borderWidth = 2,
    .cornerRadius = 8
});

auto button = nebula::ui::Button::create("Click Me");
button->setPosition({50, 30});
button->onClick = []() {
    // Handle click
};

auto label = nebula::ui::Label::create("Hello, World!");
label->setFontSize(18);
label->setColor({1, 1, 1, 1});

panel->addChild(button);
panel->addChild(label);
```

## Layouts

Automatic layout management:

```cpp
// Horizontal layout
auto hLayout = nebula::ui::HorizontalLayout::create();
hLayout->setSpacing(10);
hLayout->addWidget(button1);
hLayout->addWidget(button2);
hLayout->addWidget(button3);

// Vertical layout
auto vLayout = nebula::ui::VerticalLayout::create();
vLayout->setPadding({10, 10, 10, 10});

// Grid layout
auto grid = nebula::ui::GridLayout::create(3, 3);
grid->setCellSize({100, 50});
```

## Styling

Customize widget appearance with Style objects:

```cpp
nebula::ui::Style buttonStyle;
buttonStyle.backgroundColor = {0.3, 0.6, 0.9, 1};
buttonStyle.hoverColor = {0.4, 0.7, 1.0, 1};
buttonStyle.pressedColor = {0.2, 0.4, 0.7, 1};
buttonStyle.textColor = {1, 1, 1, 1};
buttonStyle.fontSize = 16;
buttonStyle.borderWidth = 1;
buttonStyle.borderColor = {1, 1, 1, 0.3};
buttonStyle.cornerRadius = 4;

button->setStyle(buttonStyle);
```

## Event Handling

```cpp
// Mouse events
button->onClick = []() { /* ... */ };
button->onHover = []() { /* ... */ };
button->onPress = []() { /* ... */ };
button->onRelease = []() { /* ... */ };

// Keyboard events
textInput->onTextEntered = [](char c) { /* ... */ };
textInput->onKeyPressed = [](Key key) { /* ... */ };

// Drag events
slider->onValueChanged = [](float value) { /* ... */ };
```

## Custom Widgets

Extend the Widget base class to create custom UI elements:

```cpp
class HealthBar : public nebula::ui::Widget {
public:
    HealthBar() {
        setSize({200, 20});
    }

    void setValue(float percent) {
        healthPercent = percent;
    }

    void onRender(nebula::gfx::Renderer& renderer) override {
        // Background
        renderer.drawRect(getPosition(), getSize(), {0.2, 0.2, 0.2, 1});

        // Health fill
        vec2 fillSize = {getSize().x * healthPercent, getSize().y};
        renderer.drawRect(getPosition(), fillSize, {0.2, 0.8, 0.2, 1});
    }

private:
    float healthPercent = 1.0f;
};
```
