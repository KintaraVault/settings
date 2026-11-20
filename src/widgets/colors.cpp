// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "widgets/colors.h"

#include "ui/theme.h"

using namespace cutefish::ui;

namespace settings {

static constexpr Color kWhite = Color::rgb(0xFFFFFF);
static constexpr Color kBlack = Color::rgb(0x000000);

void addSettingsColors(ColorMixer& mixer, const Theme& theme)
{
    const bool dark = theme.darkMode;

    mixer[SettingsColor::PageBackground] = Color::rgb(dark ? 0x212121 : 0xffffff);
    mixer[SettingsColor::CardBackground] = Color::rgb(dark ? 0x2a2a2a : 0xf7f7f7);
    mixer[SettingsColor::CardSeparator] = withAlpha(ColorId::Text, dark ? 0.09 : 0.07);
    mixer[SettingsColor::SidebarBackground] =
        withAlpha(wallpaperTinted(Color::rgb(dark ? 0x282829 : 0xececeb)), dark ? 0.83 : 0.87);
    mixer[SettingsColor::SidebarItemCurrent] = theme.highlight == accent::blue
        ? ColorRecipe(Color::rgb(dark ? 0x0879f9 : 0x0070f5)) : ColorRecipe(ColorId::Highlight);

    mixer[SettingsColor::RowHighlight] = withAlpha(ColorId::Text, dark ? 0.05 : 0.15);
    mixer[SettingsColor::RowComboBoxHovered] = withAlpha(ColorId::Text, dark ? 0.08 : 0.06);
    mixer[SettingsColor::RowComboBoxOpen] = withAlpha(ColorId::Text, dark ? 0.12 : 0.1);
    mixer[SettingsColor::StandardButtonBackground] = withAlpha(ColorId::Text, dark ? 0.12 : 0.085);
    mixer[SettingsColor::KeyCap] = dark ? lighter(ColorId::SecondBackground, 1.6)
                                        : darker(ColorId::SecondBackground, 1.08);
    mixer[SettingsColor::PreferredLanguageTag] = withAlpha(ColorId::Highlight, dark ? 0.2 : 0.12);
    mixer[SettingsColor::WallpaperPlaceholder] = dark ? kWhite : Color::rgb(0xC8CAD5);

    mixer[SettingsColor::TimeZoneMapPill] = dark ? kBlack.withAlpha(0.6) : kWhite.withAlpha(0.9);

    mixer[SettingsColor::BatteryChipBackground] = dark ? ColorId::AlternateBackground : ColorId::Background;
    mixer[SettingsColor::BatteryChipBorder] = withAlpha(ColorId::Text, dark ? 0.1 : 0.08);
    mixer[SettingsColor::GraphGrid] = dark ? kWhite.withAlpha(0.08) : kBlack.withAlpha(0.06);
    mixer[SettingsColor::GraphBaseline] = dark ? kWhite.withAlpha(0.18) : kBlack.withAlpha(0.14);
    mixer[SettingsColor::GraphProbe] = dark ? kWhite.withAlpha(0.28) : kBlack.withAlpha(0.2);
}

} // namespace settings
