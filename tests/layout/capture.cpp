// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <cstdio>
#include <cstdlib>
#include <string>

#include "base/timer.h"
#include "ui/application.h"
#include "ui/test_input.h"
#include "ui/theme.h"
#include "ui/window.h"
#include "widgets/picture.h"
#include "window/options.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace cutefish::ui;

// A fixed frame time, so captures show animations at one phase.
static constexpr uint64_t kFrameNs = 1000000000;

// settings-capture DIR [PAGE...]: every page, or the ones named, at the
// reference captures' window size, light and dark, at 100% and 150%, with
// the layout dump beside each picture.
int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s DIR [--style=0..3] [--tone=0..2] [--glass=0..1] [--tint=0..9] [--opaque] [PAGE...]\n", argv[0]);
        return 1;
    }
    Application application(argc, argv, Application::Platform::Headless);
    settings::setDataDirectory(SETTINGS_SOURCE_DIR);
    settings::setOptions({.mockNetwork = true, .mockBattery = true});
    const std::string directory = argv[1];
    AppearanceStyle style;
    std::vector<std::string> ids;
    for (int i = 2; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg.starts_with("--style="))
            style.icons = appearance_style::iconStyle(std::atoi(arg.c_str() + 8));
        else if (arg.starts_with("--tone="))
            style.tone = appearance_style::iconTone(std::atoi(arg.c_str() + 7));
        else if (arg.starts_with("--glass="))
            style.glass = appearance_style::glassValue(std::atof(arg.c_str() + 8));
        else if (arg.starts_with("--tint="))
            style.tintColor = appearance_style::colorIndex(std::atoi(arg.c_str() + 7));
        else if (arg == "--opaque")
            style.reduceTransparency = true;
        else
            ids.push_back(arg);
    }
    if (ids.empty()) {
        for (const settings::PageInfo& page : settings::pages()) {
            if (page.create)
                ids.push_back(page.id);
        }
    }
    const Theme light = Theme::current();
    int failures = 0;
    for (const auto& [mode, theme] : {std::pair{"light", light}, std::pair{"dark", Theme::dark()}}) {
        Theme::setCurrent(theme, style);
        for (const double scale : {1.0, 1.5}) {
            for (const std::string& id : ids) {
                const settings::PageInfo* page = settings::findPage(id);
                if (!page) {
                    fprintf(stderr, "settings-capture: no page %s\n", id.c_str());
                    ++failures;
                    continue;
                }
                settings::SettingsWindow window;
                window.window().setScale(scale);
                window.window().resize({740, 690});
                TestInput::configure(window.window(), 740, 690, {.activated = true});
                if (!page->name.empty())
                    window.switchToPage(page->name);
                else {
                    window.switchToPage(page->parentId);
                    window.pushPage(page->id);
                }
                window.reveal();
                // Wait for the preview's asynchronous wallpaper decode.
                if (id == "appearance" || id == "display" || id == "background") {
                    window.window().grab(kFrameNs);
                    cutefish::base::Timer settle;
                    settle.setSingleShot(true);
                    settle.onTimeout = [&application] { application.quit(); };
                    settle.start(std::chrono::milliseconds(500));
                    application.run();
                }
                char name[256];
                snprintf(name, sizeof(name), "%s/%s_%s@%g", directory.c_str(), mode, id.c_str(), scale);
                if (!window.window().grab(kFrameNs).savePng(std::string(name) + ".png")) {
                    fprintf(stderr, "settings-capture: cannot write %s.png\n", name);
                    ++failures;
                }
                if (FILE* file = fopen((std::string(name) + ".txt").c_str(), "w")) {
                    fputs(window.window().layoutDump().c_str(), file);
                    fclose(file);
                }
            }
        }
    }
    Theme::setCurrent(light);
    return failures ? 1 : 0;
}
