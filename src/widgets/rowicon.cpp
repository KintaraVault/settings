// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "widgets/rowicon.h"

#include "ui/icon.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "widgets/picture.h"

using namespace cutefish::ui;

namespace settings {

RowIcon::RowIcon(std::string glyph, Color tint)
{
    m_picture = addChild<Picture>();
    m_glyph = addChild<Icon>();
    m_glyph->setInkCentered(true);
    set(std::move(glyph), tint);
}

void RowIcon::set(std::string glyph, Color tint)
{
    m_tint = tint;
    const bool picture = glyph.ends_with(".svg");
    m_picture->setVisible(picture);
    m_glyph->setVisible(!picture);
    // The sidebar's own glyphs are white already; the toolkit's need tinting.
    if (picture) {
        m_picture->setPath(dataPath("images/sidebar/dark/" + glyph));
        m_picture->setTint(Color::rgb(0xFFFFFF));
    } else {
        m_glyph->setIconName(std::move(glyph));
        m_glyph->setColor(Color::rgb(0xFFFFFF));
    }
    requestPaint();
}

void RowIcon::layout()
{
    const double side = std::round(std::min(geometry().width, geometry().height) * 0.67);
    const RectF glyph{(geometry().width - side) / 2, (geometry().height - side) / 2, side, side};
    m_glyph->setSize(side);
    m_picture->setGeometry(glyph);
    m_glyph->setGeometry(glyph);
}

void RowIcon::paint(Painter& painter)
{
    const RectF rect = localRect();
    const bool dark = Theme::current().darkMode;
    const auto top = dark ? m_tint.darker(1.35) : m_tint.lighter(1.14);
    const auto bottom = dark ? m_tint.darker(1.75) : m_tint;
    const double radius = std::min(rect.width, rect.height) * 0.24;
    painter.fillRoundedRect(rect, radius, LinearGradient::vertical(rect, top, bottom));
    painter.drawRoundedRect(rect, {radius}, Color{0, 0, 0, 0}, 0.5, Color::rgb(0xffffff).withAlpha(0.24));
}

} // namespace settings
