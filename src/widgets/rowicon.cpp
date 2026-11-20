// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "widgets/rowicon.h"

#include <unordered_set>

#include "base/file.h"
#include "ui/icon.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "widgets/picture.h"

using namespace cutefish::ui;

namespace settings {

namespace {
appearance_style::IconLayers badgeLayers(const Theme& theme, Color tint)
{
    const auto& style = Theme::appearanceStyle();
    if (style.icons == IconStyle::Clear) {
        const bool dark = appearance_style::dark(style, theme.darkMode);
        const double opacity = style.reduceTransparency ? 1 : .93;
        return {Color::rgb(dark ? 0x484848 : 0x818181).withAlpha(opacity),
                Color::rgb(dark ? 0x232323 : 0x727272).withAlpha(opacity),
                Color::rgb(0xffffff), Color::rgb(0xffffff).withAlpha(dark ? .26 : .45)};
    }
    if (style.icons == IconStyle::Dark)
        return {Color::rgb(0x262628), Color::rgb(0x101012),
                appearance_style::blend(tint, Color::rgb(0xffffff), .13), Color::rgb(0xffffff).withAlpha(.23)};
    return appearance_style::layers(style, theme.darkMode, tint, theme.highlight);
}
}

RowIcon::RowIcon(std::string glyph, Color tint)
{
    setInkCentered(true);
    setColor(Color::rgb(0xFFFFFF));
    set(std::move(glyph), tint);
}

void RowIcon::set(std::string glyph, Color tint)
{
    m_tint = tint;
    if (glyph.ends_with(".svg")) {
        const std::string file = dataPath("images/sidebar/dark/" + glyph);
        glyph = "settings-row:" + file;
        static std::unordered_set<std::string> registered;
        if (!registered.contains(glyph)) {
            if (auto svg = cutefish::base::file::read(file)) {
                Icon::registerGlyph(glyph, std::move(*svg));
                registered.insert(glyph);
            }
        }
    }
    setIconName(std::move(glyph));
    requestPaint();
}

void RowIcon::layout()
{
    setSize(std::min(geometry().width, geometry().height) * 0.67);
    setColor(badgeLayers(Theme::current(), m_tint).ink);
}

void RowIcon::paint(Painter& painter)
{
    const RectF rect = localRect();
    const bool dark = Theme::current().darkMode;
    const auto& style = Theme::appearanceStyle();
    if (style.icons != IconStyle::Default) {
        const auto layers = badgeLayers(Theme::current(), m_tint);
        const double radius = std::min(rect.width, rect.height) * 0.24;
        painter.fillRoundedRect(rect, radius, LinearGradient::vertical(rect, layers.top, layers.bottom));
        painter.drawRoundedRect(rect, {radius}, Color{0, 0, 0, 0}, 0.6, layers.rim);
        paintInSnappedBounds(painter);
        return;
    }
    const auto top = dark ? m_tint.darker(1.35) : m_tint.lighter(1.14);
    const auto bottom = dark ? m_tint.darker(1.75) : m_tint;
    const double radius = std::min(rect.width, rect.height) * 0.24;
    painter.fillRoundedRect(rect, radius, LinearGradient::vertical(rect, top, bottom));
    painter.drawRoundedRect(rect, {radius}, Color{0, 0, 0, 0}, 0.5, Color::rgb(0xffffff).withAlpha(0.24));
    paintInSnappedBounds(painter);
}

} // namespace settings
