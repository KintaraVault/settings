// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include "pagetest.h"
#include "ui/segmented_control.h"

using namespace cutefish::ui;
using settings::testing::findView;

TEST(DockPage, DisplayModeGoesUnderItsLabelInANarrowWindow)
{
    PageFixture page("dock");
    Window& window = page.window->window();
    window.resize({640, 820});
    window.grab();
    auto* control = findView<SegmentedControl>(window.contentView(), "dock:displayMode");
    ASSERT_NE(control, nullptr);
    // Across the card inside its padding, below the label.
    EXPECT_EQ(control->geometry().x, 10);
    EXPECT_EQ(control->geometry().y, 38);
    EXPECT_EQ(control->geometry().width, 348);
    const double button = (control->geometry().width - 2 * control->padding()) / double(control->count());
    for (size_t i = 0; i < control->count(); ++i)
        EXPECT_GE(button, control->implicitItemWidth(i)) << i;
    EXPECT_EQ(control->parent()->geometry().height, 38 + control->geometry().height + 10);
}

TEST(DockPage, DisplayModeGoesUnderItsLabelWhileHalfTheRowIsTight)
{
    PageFixture page("dock");
    Window& window = page.window->window();
    auto* control = findView<SegmentedControl>(window.contentView(), "dock:displayMode");
    ASSERT_NE(control, nullptr);
    window.resize({690, 820});
    window.grab();
    EXPECT_EQ(control->geometry().x, 10);
    EXPECT_EQ(control->geometry().width, 398);
    // Half the row would leave the labels 2 px a side.
    window.resize({780, 820});
    window.grab();
    EXPECT_EQ(control->geometry().width, 488);
    window.resize({870, 820});
    window.grab();
    EXPECT_EQ(control->geometry().y, 9);
    EXPECT_EQ(control->geometry().width, 289);
}

TEST(DockPage, DisplayModeTakesHalfTheRowWhenItsLabelsFit)
{
    PageFixture page("dock");
    Window& window = page.window->window();
    window.resize({882, 820});
    window.grab();
    auto* control = findView<SegmentedControl>(window.contentView(), "dock:displayMode");
    ASSERT_NE(control, nullptr);
    // The page, the card's padding on both sides, half of the rest.
    EXPECT_EQ(control->geometry().width, 295);
}
