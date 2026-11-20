// SPDX-License-Identifier: BSD-3-Clause
#include "base/i18n.h"
#include "widgets/paneintro.h"
#include "widgets/sections.h"
#include "window/page.h"
#include "window/pagelist.h"

using namespace cutefish::ui;
using cutefish::i18n::tr;

namespace settings {
namespace {

class AccessibilityPage : public Page
{
public:
    explicit AccessibilityPage(SettingsWindow& window) : Page(window, tr("Accessibility"))
    {
        content()->addChild<PaneIntro>(title(), tr("Adjust text, displays and input devices to suit the way you work."),
                                      "accessibility.svg", Color::rgb(0x0087ed));
        auto* vision = content()->addChild<Section>(tr("Vision"))->addChild<Card>(Card::Style::List);
        addPageLink(vision, window, "display", true);
        addPageLink(vision, window, "fonts", true);
        auto* input = content()->addChild<Section>(tr("Physical and Motor"))->addChild<Card>(Card::Style::List);
        addPageLink(input, window, "keyboard", true);
        addPageLink(input, window, "cursortheme", true);
        addPageLink(input, window, "mouse", true);
    }
};

const PageRegistration registration("accessibility", [](SettingsWindow& window) {
    return std::make_unique<AccessibilityPage>(window);
});

} // namespace
} // namespace settings
