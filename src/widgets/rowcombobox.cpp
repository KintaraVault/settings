// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "widgets/colors.h"
#include "widgets/rowcombobox.h"

#include <cmath>

#include "ui/color_provider.h"
#include "ui/icon.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/menu.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/metrics.h"

using namespace cutefish::ui;

namespace settings {

static constexpr double kChevron = 20;
// Fits a plain row, so a choice does not make its row taller than a switch does.
static constexpr double kHeight = 26;
static constexpr double kItemIcon = 22;

RowComboBox::RowComboBox(std::vector<std::string> items)
{
    m_icon = addChild<ThemeIcon>(std::string(), kItemIcon);
    m_icon->setVisible(false);
    m_label = addChild<Label>();
    m_label->setVerticallyCentered(true);
    setItems(std::move(items));
}

RowComboBox::~RowComboBox() = default;

void RowComboBox::setItems(std::vector<std::string> items)
{
    std::vector<Item> list;
    for (std::string& text : items)
        list.push_back({std::move(text), {}});
    setItems(std::move(list));
}

void RowComboBox::setItems(std::vector<Item> items)
{
    m_items = std::move(items);
    if (m_current >= count())
        m_current = count() - 1;
    if (m_current < 0 && !m_items.empty())
        m_current = 0;
    update();
}

void RowComboBox::setCurrentIndex(int index)
{
    if (index < -1 || index >= count() || index == m_current)
        return;
    m_current = index;
    update();
}

std::string RowComboBox::currentText() const
{
    return m_current >= 0 ? m_items[size_t(m_current)].text : std::string();
}

void RowComboBox::setDisplayText(std::optional<std::string> text)
{
    m_displayText = std::move(text);
    update();
}

void RowComboBox::update()
{
    m_label->setText(m_displayText.value_or(currentText()));
    const std::string icon = m_current >= 0 ? m_items[size_t(m_current)].icon : std::string();
    m_icon->setIconName(icon);
    m_icon->setVisible(!icon.empty());
    requestLayout();
    requestPaint();
}

double RowComboBox::trailingOverhang() const
{
    return 0;
}

SizeF RowComboBox::sizeHint() const
{
    const Theme& theme = Theme::current();
    const double iconWidth = m_icon->isVisible() ? kItemIcon + theme.mediumSpacing : 0;
    const double width = theme.largeSpacing + iconWidth + std::ceil(m_label->sizeHint().width)
            + theme.smallSpacing + kChevron;
    // FishUI's text field pads its text by smallSpacing above and below.
    const double height = std::max(kHeight, m_label->sizeHint().height + 6);
    return {width, height};
}

void RowComboBox::layout()
{
    const Theme& theme = Theme::current();
    const double width = geometry().width;
    const double height = geometry().height;
    double x = theme.largeSpacing;
    if (m_icon->isVisible()) {
        m_icon->setGeometry({x, (height - kItemIcon) / 2, kItemIcon, kItemIcon});
        x += kItemIcon + theme.mediumSpacing;
    }
    const double right = width - theme.smallSpacing - kChevron;
    m_label->setGeometry({x, 0, std::max(0.0, right - x), height});
    const bool enabled = isEnabled();
    m_label->setColor(enabled ? theme.text : theme.disabledText);
    m_icon->setOpacity(enabled ? 1 : 0.6);
}

void RowComboBox::paint(Painter& painter)
{
    const Theme& theme = Theme::current();
    const bool open = m_menu && m_menu->isOpen();
    if (isEnabled() && (m_hovered || open))
        painter.drawRoundedRect(localRect(), {7, 0, themeCorners()},
                                color(open ? SettingsColor::RowComboBoxOpen : SettingsColor::RowComboBoxHovered));
    const double x = geometry().width - kChevron / 2;
    const double y = geometry().height / 2;
    const Color ink = (isEnabled() ? theme.text : theme.disabledText).withAlpha(isEnabled() ? 0.88 : 0.43);
    painter.drawSmoothRectangle({x - 10, y - 10, 20, 20}, 10, theme.text.withAlpha(isEnabled() ? 0.075 : 0.045));
    for (double sign : {-1., 1.}) {
        painter.drawLine({x - 3, y + sign * 1.3}, {x, y + sign * 4.5}, ink, 1.25);
        painter.drawLine({x, y + sign * 4.5}, {x + 3, y + sign * 1.3}, ink, 1.25);
    }
}

// The chevron's colour and fade follow the state, set in layout().
void RowComboBox::enabledChanged()
{
    requestLayout();
    requestPaint();
}

void RowComboBox::pointerEnter()
{
    m_hovered = true;
    requestPaint();
}

void RowComboBox::pointerLeave()
{
    m_hovered = false;
    requestPaint();
}

void RowComboBox::pointerPress(const PointerEvent& event)
{
    if (event.button != MouseButton::Left)
        return;
    setFocus(FocusReason::Mouse);
    toggleMenu(false);
}

bool RowComboBox::keyPress(const KeyEvent& event)
{
    switch (event.key) {
    case key::Down:
    case key::Up:
    case key::Space:
    case key::Return:
    case key::KeypadEnter:
        toggleMenu(true);
        return true;
    default:
        return false;
    }
}

// FishUI's ComboBoxMenu: a drop-down 3 px under the box, opened on the
// checked row.
void RowComboBox::toggleMenu(bool fromKeyboard)
{
    if (m_menu && m_menu->isOpen()) {
        m_menu->close();
        return;
    }
    if (m_items.empty())
        return;
    m_menu = std::make_unique<Menu>();
    m_menu->setDropDown(true);
    for (size_t i = 0; i < m_items.size(); ++i) {
        const size_t index = m_menu->addItem(m_items[i].text, [this, i] {
            m_current = int(i);
            update();
            if (onActivated)
                onActivated(int(i));
        });
        m_menu->setItemChecked(index, int(i) == m_current);
        if (!m_items[i].icon.empty())
            m_menu->setItemIcon(index, m_items[i].icon);
        if (m_fontPreview)
            m_menu->setItemFontFamily(index, m_items[i].text);
    }
    m_menu->setMinimumWidth(geometry().width);
    m_menu->onClosed = [this] { requestPaint(); };
    m_menu->popupFor(this, 3, std::nullopt, fromKeyboard);
    requestPaint();
}

} // namespace settings
