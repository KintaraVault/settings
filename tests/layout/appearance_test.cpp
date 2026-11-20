// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include "pagetest.h"
#include "ui/check_box.h"
#include "ui/header_button_group.h"
#include "ui/search_field.h"
#include "ui/slider.h"
#include "ui/test_input.h"
#include "ui/theme.h"
#include "widgets/desktopwallpaper.h"
#include "widgets/rowcombobox.h"

using namespace cutefish::ui;
using settings::testing::findView;

TEST(AppearancePage, MaterialAndIconControlsAcceptInput)
{
    PageFixture page("appearance");
    for (const char* name : {"appearance:glass", "appearance:wallpaperTint", "appearance:reduceTransparency",
                             "appearance:iconStyle:0", "appearance:iconStyle:1", "appearance:iconStyle:2",
                             "appearance:iconStyle:3", "appearance:folderColor",
                             "appearance:accent:-1", "appearance:textHighlight"}) {
        auto* control = findView(page.window->window().contentView(), name);
        ASSERT_NE(control, nullptr) << name;
        EXPECT_TRUE(control->isVisible()) << name;
        EXPECT_TRUE(control->isEnabled()) << name;
    }
}

TEST(AppearancePage, ChangesThePreviewWhileDraggingGlass)
{
    PageFixture page("appearance");
    auto& window = page.window->window();
    auto* slider = findView<Slider>(window.contentView(), "appearance:glass");
    auto* lenses = findView(window.contentView(), "appearance:previewGlass");
    ASSERT_TRUE(slider && lenses);
    auto* actions = findView<HeaderButtonGroup>(window.contentView(), "appearance:previewActions");
    ASSERT_NE(actions, nullptr);
    const auto sample = actions->mapToWindow({20, 8});
    const auto before = window.grab(1'000'000'000);
    const auto position = [&](double fraction) {
        return slider->mapToWindow({slider->localRect().width * fraction, slider->localRect().height / 2});
    };
    TestInput::press(window, position(.5));
    ASSERT_TRUE(slider->isPressed());
    TestInput::movePointer(window, position(.97));
    EXPECT_GT(slider->value(), .9);
    window.grab(1'100'000'000);
    const auto after = window.grab(1'500'000'000);
    const double scale = window.scale();
    EXPECT_NE(before.pixel(int(sample.x * scale), int(sample.y * scale)),
              after.pixel(int(sample.x * scale), int(sample.y * scale)));
    TestInput::release(window, position(.97));
    EXPECT_FALSE(slider->isPressed());
}

TEST(AppearancePage, ShowsToneAndSharedColorOnlyWhenAppropriate)
{
    struct Restore {
        Theme theme = Theme::current();
        AppearanceStyle style = Theme::appearanceStyle();
        ~Restore() { Theme::setCurrent(theme, style); }
    } restore;
    auto style = restore.style;
    PageFixture page("appearance");
    auto& window = page.window->window();
    auto* root = window.contentView();
    auto* tones = findView(root, "appearance:iconTones");
    auto* folder = findView<settings::RowComboBox>(root, "appearance:folderColor");
    auto* highlight = findView<settings::RowComboBox>(root, "appearance:textHighlight");
    auto* slider = findView<Slider>(root, "appearance:glass");
    ASSERT_TRUE(tones && folder && highlight && slider);
    EXPECT_FALSE(tones->isVisible());

    style.icons = IconStyle::Tinted;
    style.tone = IconTone::Dark;
    style.tintColor = 2;
    style.folderColor = 7;
    style.textHighlight = 4;
    style.reduceTransparency = true;
    Theme::setCurrent(restore.theme, style);
    window.grab();
    EXPECT_TRUE(tones->isVisible());
    auto* dark = findView<RadioButton>(root, "appearance:iconTone:1");
    ASSERT_NE(dark, nullptr);
    EXPECT_TRUE(dark->isChecked());
    EXPECT_EQ(folder->currentIndex(), 2);
    EXPECT_EQ(highlight->currentIndex(), 4);
    EXPECT_FALSE(slider->isEnabled());

    style.icons = IconStyle::Clear;
    Theme::setCurrent(restore.theme, style);
    window.grab();
    EXPECT_TRUE(tones->isVisible());
    EXPECT_EQ(folder->currentIndex(), 7);
}

TEST(AppearancePage, IconChoicesStayInPlaceWhenToneControlsAppear)
{
    struct Restore {
        Theme theme = Theme::current();
        AppearanceStyle style = Theme::appearanceStyle();
        ~Restore() { Theme::setCurrent(theme, style); }
    } restore;
    PageFixture page("appearance");
    auto& window = page.window->window();
    auto style = restore.style;
    style.icons = IconStyle::Default;
    Theme::setCurrent(restore.theme, style);
    window.grab();
    const auto before = settings::testing::windowRect(window, "appearance:iconStyle:0");
    ASSERT_GT(before.width, 0);

    for (auto choice : {IconStyle::Dark, IconStyle::Clear, IconStyle::Tinted}) {
        style.icons = choice;
        Theme::setCurrent(restore.theme, style);
        window.grab();
        const auto after = settings::testing::windowRect(window, "appearance:iconStyle:0");
        EXPECT_EQ(after.x, before.x);
        EXPECT_EQ(after.y, before.y);
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
