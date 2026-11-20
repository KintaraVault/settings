// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <string_view>

// Spacing tokens for every page, on a 4 px grid. Pages use the semantic
// names; retune the look here rather than at the use sites.
namespace settings::metrics {

// Scale.
inline constexpr double xxs = 2;
inline constexpr double xs = 4;
inline constexpr double s = 8;
inline constexpr double m = 12;
inline constexpr double l = 16;
inline constexpr double xl = 24;
inline constexpr double xxl = 32;

inline constexpr double headerTitlePointSize = 11.25;

// Page: the scroll area's margins and the rhythm between its sections.
inline constexpr double pageMargin = 20;
// Below the floating window header, which already has its own air.
inline constexpr double pageTopMargin = 10;
inline constexpr double sidebarTopMargin = s;
inline constexpr double sectionSpacing = 10;
// Between a section's card and its label or footnote: tight, so the text
// reads as belonging to that card and not to the neighbouring section.
inline constexpr double sectionLabelSpacing = 9;

// Card. Section labels and row separators share its horizontal padding so
// every text edge on a page lines up.
inline constexpr double cardHorizontalPadding = 10;
inline constexpr double cardVerticalPadding = 10;
inline constexpr double cardSpacing = 10;
inline constexpr double cardRadius = 12;

// Row inside a card.
inline constexpr double rowHeight = 38;
// For a row holding a slider or another control taller than a switch: the
// extra height keeps the control from crowding the row's edges.
inline constexpr double controlRowHeight = 42;
// For a row with a second line of text below its label.
inline constexpr double twoLineRowHeight = 48;
inline constexpr double rowSpacing = 10;

// A full-width button standing alone between cards.
inline constexpr double buttonHeight = 26;

// Content: controls placed side by side, and text stacked tightly.
inline constexpr double controlSpacing = s;
inline constexpr double textSpacing = xxs;

// Selection tiles and grids of choices inside a card.
inline constexpr double tileSpacing = xl;

// Two-column label/control forms.
inline constexpr double formColumnSpacing = xl;
inline constexpr double formRowSpacing = l;

// Sidebar.
// The default width, which is also the widest a drag makes the sidebar.
inline constexpr double sidebarWidth = 232;
inline constexpr double sidebarMinimumWidth = 180;
inline constexpr double sidebarMargin = 10;
inline constexpr double sidebarItemHeight = 32;
inline constexpr double sidebarItemPadding = 9;
inline constexpr double sidebarItemSpacing = 0;
inline constexpr double sidebarCategorySpacing = 12;

// Dialogs.
inline constexpr double dialogPadding = l;
inline constexpr double dialogSpacing = l;
inline constexpr double dialogButtonSpacing = s;
inline constexpr double dialogWindowButtonsMargin = 0;

// Font sizes are kept in points, as the Qt version gave them.
constexpr double points(double pointSize)
{
    return pointSize * 4 / 3;
}

// A row's last glyph takes this margin so its drawing, not its box, ends
// where a switch does. Negative: the box reaches past the content edge.
double trailingGlyphMargin(std::string_view name, double boxWidth, double padding = 0);

} // namespace settings::metrics
