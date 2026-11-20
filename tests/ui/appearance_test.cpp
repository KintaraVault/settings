// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include "cutefish/appearance/appearance.h"
#include "pagetest.h"
#include "input_test_support.h"
#include "ui/header_button_group.h"
#include "ui/search_field.h"
#include "widgets/desktopwallpaper.h"

// The tests run on a private bus, where no appearance service answers: a
// change goes out, and the value stays until the service reports one, as
// the Qt version's did.
TEST(AppearanceClient, WaitsForTheServiceToReportAChange)
{
    cutefish::appearance::Appearance appearance;
    int changes = 0;
    cutefish::base::Subscription subscription = appearance.changed.connect([&changes] { ++changes; });
    const int accent = appearance.accentColor();
    appearance.setAccentColor(accent + 1);
    appearance.setDarkModeSchedule(cutefish::appearance::Appearance::SunsetToSunrise);
    EXPECT_EQ(appearance.accentColor(), accent);
    EXPECT_EQ(appearance.darkModeSchedule(), int(cutefish::appearance::Appearance::Never));
    EXPECT_EQ(changes, 0);
}

TEST(AppearancePage, ShowsReadOnlyMaterialAndIconControls)
{
    PageFixture page("appearance");
    for (const char* name : {"appearance:glass", "appearance:wallpaperTint", "appearance:reduceTransparency",
                             "appearance:iconStyle:0", "appearance:iconStyle:1", "appearance:iconStyle:2",
                             "appearance:iconStyle:3", "appearance:folderColor"}) {
        auto* control = settings::testing::findView(page.window->window().contentView(), name);
        ASSERT_NE(control, nullptr) << name;
        EXPECT_TRUE(control->isVisible()) << name;
        EXPECT_FALSE(control->isEnabled()) << name;
    }
}

TEST(AppearancePage, GlassPreviewUsesLiveWallpaperAndToolkitControls)
{
    PageFixture page("appearance");
    const auto find = [&](const char* name) {
        return settings::testing::findView(page.window->window().contentView(), name);
    };
    auto* wallpaper = dynamic_cast<settings::DesktopWallpaper*>(find("appearance:wallpaper"));
    ASSERT_NE(wallpaper, nullptr);
    EXPECT_FALSE(wallpaper->wallpaperPath().empty());
    EXPECT_EQ(wallpaper->fillMode(), cutefish::ui::ImageView::FillMode::PreserveAspectCrop);
    auto* actions = dynamic_cast<cutefish::ui::HeaderButtonGroup*>(find("appearance:previewActions"));
    ASSERT_NE(actions, nullptr);
    EXPECT_EQ(actions->buttons().size(), 2u);
    auto* search = dynamic_cast<cutefish::ui::SearchField*>(find("appearance:previewSearch"));
    ASSERT_NE(search, nullptr);
    EXPECT_TRUE(search->isReadOnly());
    EXPECT_FALSE(search->acceptsFocus());
}
