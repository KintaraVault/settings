// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <memory>

#include "pagetest.h"
#include "ui/image.h"
#include "ui/layout.h"
#include "ui/window.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/picture.h"
#include "widgets/rowicon.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace cutefish::ui;
using namespace settings;
using settings::testing::isHidden;
using settings::testing::windowRect;

TEST(Foundation, SidebarLinesUpWithTheMetrics)
{
    SettingsWindow settingsWindow;
    Window& window = settingsWindow.window();
    window.setScale(1.5);
    window.resize({900, 610});
    settingsWindow.reveal();
    window.grab();
    const RectF search = windowRect(window, "sidebar:search");
    EXPECT_EQ(search.x, 10);
    EXPECT_NEAR(search.y, 61, 0.5);
    EXPECT_EQ(search.width, 212);
    EXPECT_EQ(search.height, 28);
    // Wi-Fi opens first, inset by the sidebar margin.
    const RectF wifi = windowRect(window, "sidebar:wlan");
    EXPECT_EQ(wifi.x, 10);
    EXPECT_EQ(wifi.width, 212);
    EXPECT_EQ(wifi.height, 32);
    const RectF page = windowRect(window, "page:wlan");
    EXPECT_EQ(page.x, 232);
    EXPECT_EQ(page.width, 668);
}

TEST(ListRow, KeepsTheRowHeights)
{
    Window window;
    window.setScale(1.5);
    window.resize({400, 300});
    // At its own height, as a page column holds it.
    auto page = std::make_unique<Column>();
    Card* card = page->addChild<Card>(Card::Style::List);
    ListRow* plain = card->addChild<ListRow>("Plain");
    plain->setName("plain");
    InfoRow* empty = card->addChild<InfoRow>("Empty", "");
    empty->setName("empty");
    InfoRow* info = card->addChild<InfoRow>("Info", "Value");
    info->setName("info");
    window.setContentView(std::move(page));
    window.show();
    window.grab();
    EXPECT_EQ(windowRect(window, "plain").height, 38);
    EXPECT_TRUE(isHidden(window, "empty"));
    EXPECT_EQ(windowRect(window, "info").y, 38);
}

TEST(Foundation, NavigationLeavesAnEvenGapToTheTitle)
{
    PageFixture page("appearance");
    const Window& window = page.window->window();
    const RectF back = windowRect(window, "window:back");
    const RectF forward = windowRect(window, "window:forward");
    const RectF title = windowRect(window, "window:title");
    EXPECT_EQ(back.x, 240);
    EXPECT_NEAR(back.width, forward.width, 1);
    EXPECT_NEAR(title.x - forward.right(), 12, 1);
}

TEST(RowIcons, RealArtworkIsCenteredInTheDevicePixelTile)
{
    const auto* wlan = findPage("wlan");
    const auto* network = findPage("network");
    const auto* vpn = findPage("vpn");
    ASSERT_TRUE(wlan && network && vpn);
    EXPECT_NE(vpn->icon, network->icon);
    for (const auto& glyph : {wlan->icon, network->icon, vpn->icon, std::string("general.svg")}) {
        SCOPED_TRACE(glyph);
        for (const double scale : {1.0, 1.25, 1.5, 2.0}) {
            SCOPED_TRACE(scale);
            Window window;
            auto root = std::make_unique<View>();
            auto* tile = root->addChild<RowIcon>(glyph, Color::rgb(0));
            tile->setGeometry({7, 3, 20, 20});
            window.setScale(scale);
            window.resize({48, 40});
            window.setContentView(std::move(root));
            window.show();
            const Image capture = window.grab();
            const int size = int(20 * scale);
            const int x = int(std::ceil(7 * scale));
            const int y = int(std::ceil(3 * scale));
            int left = size, right = -1, top = size, bottom = -1;
            for (int row = 2; row < size - 2; ++row) {
                for (int col = 2; col < size - 2; ++col) {
                    if (capture.pixel(x + col, y + row).b < 100)
                        continue;
                    left = std::min(left, col);
                    right = std::max(right, col);
                    top = std::min(top, row);
                    bottom = std::max(bottom, row);
                }
            }
            ASSERT_GE(right, left);
            ASSERT_GE(bottom, top);
            EXPECT_NEAR(left + right, size - 1, 1);
            EXPECT_NEAR(top + bottom, size - 1, 1);
        }
    }
}
