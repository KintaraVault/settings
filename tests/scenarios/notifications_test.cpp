// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include "scenario.h"

TEST(NotificationsPage, ShowsTheEmptyCardWithoutApplications)
{
    Scenario scenario("notifications");
    cutefish::ui::TestDriver& driver = scenario.driver;
    EXPECT_TRUE(driver.shows("notifications:empty"));
    EXPECT_EQ(driver.countShown("notifications:mode"), 0u);
    EXPECT_FALSE(driver.shows("notifications:footnote"));
}

TEST(FocusPage, ShowsDoNotDisturb)
{
    Scenario scenario("focus");
    EXPECT_TRUE(scenario.driver.shows("focus:doNotDisturb"));
}
