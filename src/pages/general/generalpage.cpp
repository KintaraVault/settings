// SPDX-License-Identifier: BSD-3-Clause
#include "base/i18n.h"
#include "widgets/listrow.h"
#include "widgets/paneintro.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace cutefish::ui;
using cutefish::i18n::tr;

namespace settings {
namespace {

class GeneralPage : public Page
{
public:
    explicit GeneralPage(SettingsWindow& window) : Page(window, tr("General"))
    {
        content()->addChild<PaneIntro>(title(),
            tr("Manage the overall setup of your computer, including software updates, storage, and language."),
            "general.svg", Color::rgb(0x96979c), true);
        auto* system = content()->addChild<Card>(Card::Style::List);
        addPageLink(system, window, "about");
        if (auto* update = addPageLink(system, window, "softwareupdate")) {
            update->setName("general:softwareupdate");
            auto refresh = [&window, update] { update->setVisible(window.isPageAvailable("softwareupdate")); };
            refresh();
            m_availability = window.pageAvailabilityChanged.connect(refresh);
        }
        addPageLink(system, window, "storage");
        auto* preferences = content()->addChild<Card>(Card::Style::List);
        for (const char* id : {"datetime", "language", "defaultapps", "power"})
            addPageLink(preferences, window, id);
    }

private:
    cutefish::base::Subscription m_availability;
};

const PageRegistration registration("general", [](SettingsWindow& window) {
    return std::make_unique<GeneralPage>(window);
});

} // namespace
} // namespace settings
