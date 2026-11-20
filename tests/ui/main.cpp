// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <cstdlib>
#include <string>

#include <unicode/uloc.h>

#include "ui/application.h"
#include "widgets/picture.h"
#include "window/settingswindow.h"

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    // Assertions and fake service replies use English regardless of the host locale.
    setenv("LC_ALL", "C.UTF-8", 1);
    setenv("LANGUAGE", "C", 1);
    UErrorCode status = U_ZERO_ERROR;
    uloc_setDefault("en_US", &status);
    cutefish::ui::Application application(argc, argv, cutefish::ui::Application::Platform::Headless);
    settings::setDataDirectory(SETTINGS_SOURCE_DIR);
    // The user's saved sidebar width would move every page.
    char scratch[] = "/tmp/settings-ui-tests-XXXXXX";
    if (mkdtemp(scratch))
        settings::SettingsWindow::setConfigPath(std::string(scratch) + "/settings.conf");
    return RUN_ALL_TESTS();
}
