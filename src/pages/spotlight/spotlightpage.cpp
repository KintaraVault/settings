// SPDX-License-Identifier: BSD-3-Clause
#include "base/i18n.h"
#include "base/paths.h"
#include "base/settings.h"
#include "ui/label.h"
#include "widgets/formdialog.h"
#include "widgets/listrow.h"
#include "widgets/paneintro.h"
#include "widgets/rowcombobox.h"
#include "widgets/sections.h"
#include "widgets/standardbutton.h"
#include "window/page.h"
#include "window/pagelist.h"

using namespace cutefish;
using namespace cutefish::ui;
using cutefish::i18n::tr;

namespace settings {
namespace {

class SpotlightPage : public Page
{
public:
    explicit SpotlightPage(SettingsWindow& window)
        : Page(window, tr("Spotlight")), m_path(base::paths::cutefishConfig("spotlight"))
    {
        content()->addChild<PaneIntro>(title(), tr("Find applications, files, actions, and clipboard items on this computer."),
                                      "search", Color::rgb(0x96979c));
        auto* settings = content()->addChild<Card>(Card::Style::List);
        auto* display = settings->addChild<ListRow>(tr("Applications view"));
        display->setInteractive(false);
        auto* layout = display->addContent<RowComboBox>(std::vector<std::string>{tr("Grid"), tr("List")});
        const base::Settings config(m_path);
        layout->setCurrentIndex(config.boolValue("Applications/List") ? 1 : 0);
        layout->onActivated = [this](int index) { base::Settings(m_path).setValue("Applications/List", index == 1); };

        auto* keys = settings->addChild<ListRow>(tr("Quick Keys"));
        keys->setInteractive(false);
        auto* reset = keys->addContent<StandardButton>(tr("Reset Quick Keys…"));
        reset->onClicked = [this] { confirmReset(); };

        auto* shortcut = content()->addChild<Card>(Card::Style::List);
        addPageLink(shortcut, window, "shortcuts", true);
    }

private:
    void confirmReset()
    {
        m_reset = std::make_unique<FormDialog>(settingsWindow(), tr("Reset Quick Keys?"), 350);
        m_reset->setCloseVisible(false);
        auto* message = m_reset->content()->addChild<Label>(tr("This removes your custom Spotlight Quick Keys."));
        message->setWrap(true);
        message->setLayoutHints({.fillWidth = true});
        auto* buttons = m_reset->addButtonRow();
        buttons->addChild<StandardButton>(tr("Cancel"))->onClicked = [this] { m_reset->close(); };
        buttons->addChild<StandardButton>(tr("Reset"))->onClicked = [this] {
            base::Settings(m_path).remove("QuickKeys");
            m_reset->close();
        };
        m_reset->show();
    }

    std::string m_path;
    std::unique_ptr<FormDialog> m_reset;
};

const PageRegistration registration("spotlight", [](SettingsWindow& window) {
    return std::make_unique<SpotlightPage>(window);
});

} // namespace
} // namespace settings
