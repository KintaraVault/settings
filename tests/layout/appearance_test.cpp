// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include "pagetest.h"
#include "ui/header_button_group.h"
#include "ui/search_field.h"
#include "widgets/desktopwallpaper.h"

using namespace cutefish::ui;
using settings::testing::findView;

TEST(AppearancePage, ShowsReadOnlyMaterialAndIconControls)
{
    PageFixture page("appearance");
    for (const char* name : {"appearance:glass", "appearance:wallpaperTint", "appearance:reduceTransparency",
                             "appearance:iconStyle:0", "appearance:iconStyle:1", "appearance:iconStyle:2",
                             "appearance:iconStyle:3", "appearance:folderColor"}) {
        auto* control = findView(page.window->window().contentView(), name);
        ASSERT_NE(control, nullptr) << name;
        EXPECT_TRUE(control->isVisible()) << name;
        EXPECT_FALSE(control->isEnabled()) << name;
    }
}

TEST(AppearancePage, GlassPreviewUsesLiveWallpaperAndToolkitControls)
{
    PageFixture page("appearance");
    auto* root = page.window->window().contentView();
    auto* wallpaper = findView<settings::DesktopWallpaper>(root, "appearance:wallpaper");
    ASSERT_NE(wallpaper, nullptr);
    EXPECT_FALSE(wallpaper->wallpaperPath().empty());
    EXPECT_EQ(wallpaper->fillMode(), ImageView::FillMode::PreserveAspectCrop);
    auto* actions = findView<HeaderButtonGroup>(root, "appearance:previewActions");
    ASSERT_NE(actions, nullptr);
    EXPECT_EQ(actions->buttons().size(), 2u);
    auto* search = findView<SearchField>(root, "appearance:previewSearch");
    ASSERT_NE(search, nullptr);
    EXPECT_TRUE(search->isReadOnly());
    EXPECT_FALSE(search->acceptsFocus());
}
